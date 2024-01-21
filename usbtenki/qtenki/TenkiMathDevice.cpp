#include <QDebug>
#include <QSettings>
#include <QRegExp>
#include <QStringList>
#include "TenkiMathDevice.h"
#include "../common/usbtenki_cmds.h"
#include "globals.h"

TenkiMathDevice::TenkiMathDevice() // : TenkiDevice("MATH")
{
	QSettings settings;
	int i;

	setSerial("MATH");
	num_channels = 8;

	for (i=0; i<num_channels; i++) {
		channel_data[i].channel_id = i;
		channel_data[i].chip_id = USBTENKI_CHIP_MATH;
		channel_data[i].status = USBTENKI_CHN_STATUS_UNDEFINED;
		channel_data[i].raw_value = 0;
		channel_data[i].converted_data = 0;
		channel_data[i].converted_unit = TENKI_UNIT_RAW;
	}

	status = TENKI_DEVICE_STATUS_OK;

	qDebug() << "Loading math equations and units...";
	for (i=0; i<num_channels; i++) {
		QString eq = settings.value(getChannelSettingsKey(i, false)).toString();
		int un = settings.value(getChannelSettingsKey(i, true), TENKI_UNIT_RAW).toInt();


		equationsStrings[i] = eq;
		channel_data[i].converted_unit = un;
	}
}

QString TenkiMathDevice::getChannelEquation(int chn)
{
	if (!equationsStrings[chn].isEmpty()) {
		return equationsStrings[chn];
	}
	return QString("");
}

QString TenkiMathDevice::getChannelSettingsKey(int chn, bool units)
{
	if (units) {
		return QString("data/math_equations_units/") + QString::number(chn);
	}

	return QString("data/math_equations/") + QString::number(chn);
}

int TenkiMathDevice::getChannelUnits(int id)
{
	if (id < 0 || id > num_channels) {
		return TENKI_UNIT_RAW;
	}

	return channel_data[id].converted_unit;
}

void TenkiMathDevice::setChannelUnits(int id, int units)
{
	QSettings settings;

	channel_data[id].converted_unit = units;

	settings.setValue(getChannelSettingsKey(id, true), channel_data[id].converted_unit);
}

void TenkiMathDevice::setChannelEquation(int id, QString &str)
{
	QSettings settings;

	equationsStrings[id] = str;

	settings.setValue(getChannelSettingsKey(id, false), equationsStrings[id]);
}

// TODO : Compute!
int TenkiMathDevice::updateChannelData()
{
	int i;

	for (i=0; i<num_channels; i++)
	{
		QString equation = equationsStrings[i];

		// Substitude variables
		equation = substvars(equation);

		engine.clearExceptions();

		channel_data[i].converted_data =
			engine.evaluate(equation).toNumber();

		if (engine.hasUncaughtException()) {
			channel_data[i].status = USBTENKI_CHN_STATUS_UNDEFINED;
		} else {
			channel_data[i].status = USBTENKI_CHN_STATUS_VALID;
		}


	}

	return 0;
}


QString TenkiMathDevice::substvars(QString in)
{
	QRegExp rx("\\[[0-9a-zA-Z]{6}:\\d{2}\\]");
	QStringList matches;
	int pos = 0, i;

	//qDebug() << "Pre substitution: " << in;

	while ((pos = rx.indexIn(in, pos)) != -1) {
		QString var = rx.cap(0);
		// Strip the [ and ] characters
		matches << var.mid(1, var.size() - 2);

		pos += rx.matchedLength();
	}

	//qDebug() << "Variables found: " << matches;

	for (i=0; i<matches.size(); i++) {
		QString value;
		struct sourceDescription *sd = g_tenkisources->getSourceByName(matches.at(i));
		struct USBTenki_channel tmp;
		if (!sd) {
			return "Error";
		}
		if (sd->td->getStatus() != TENKI_DEVICE_STATUS_OK) {
			return "Error";
		}
		if (sd->chn_data->status != USBTENKI_CHN_STATUS_VALID) {
			return "Error";
		}

		g_tenkisources->convertToUnits(sd->chn_data, &tmp);
		in.replace("[" + matches.at(i) + "]" , QString::number(tmp.converted_data));
	}

	//qDebug() << "Post substitution: " << in;

	return in;
}
