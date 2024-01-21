#include "SimpleLogger.h"
#include "../common/usbtenki_version.h"
#include <QtGui>
#include "globals.h"
#include "usbtenki_cmds.h"
//#include <QHostInfo>

SimpleLogger::SimpleLogger(TenkiSources *ts, QString output_file, int interval_ms, enum SimpleLogger::FileFormat fmt, enum SimpleLogger::DecimalType dt, enum SimpleLogger::TimeStampFormat tfmt, enum SimpleLogger::OnError onerr)
{
	this->output_file = output_file;
	this->interval_ms = interval_ms;
	this->fmt = fmt;

	tenkisources = ts;
	count = 0;

	switch(dt)
	{
		default:
		case SystemFormat:
			logLocale = new QLocale();
			break;

		case Comma:
			logLocale = new QLocale(QLocale::French, QLocale::Canada);
			break;

		case Period:
			logLocale = new QLocale(QLocale::C, QLocale::AnyCountry);
			break;
	}

	logLocale->setNumberOptions(QLocale::OmitGroupSeparator);

	timestamp_format = tfmt;
	on_error = onerr;	
	use_utc = 0;
	append_to_file = 0;
	comments = QString("");
}

SimpleLogger::~SimpleLogger()
{
	delete logLocale;
}

void SimpleLogger::addSource(QString src, QString alias)
{
	sources.append(src);
	aliases.append(alias);
}

void SimpleLogger::writeHeader()
{
	QDateTime creation_time = QDateTime::currentDateTime();
	//QString hostname = QHostInfo::localHostName();

	file->write("# USBTenki version ");
	file->write(USBTENKI_VERSION);
	file->write(" log file\n");

	file->write("# original filename: ");
	file->write(output_file.toLocal8Bit());
	file->write("\n");

	file->write("# creation date: ");
//	file->write("on machine ");
//	file->write(hostname.toLocal8Bit());
	file->write(creation_time.toString("yyyy-MM-dd hh:mm:ss").toLocal8Bit());
	file->write("\n");

	file->write("# logging interval: ");
	file->write(logLocale->toString(interval_ms).toLocal8Bit());
	file->write(" millisecond");
	if (interval_ms != 1) {
		file->write("s");
	}
	file->write("\n");

	file->write("# log comments: ");
	file->write(comments.toLocal8Bit());
	file->write("\n");

	appendMathEquations();

	file->write("#\n");
}

// Add math equations to the header for any math channel being logged.
void SimpleLogger::appendMathEquations()
{
	for (int i=0; i<sources.size(); i++)
	{
		struct sourceDescription *sd;

		sd = tenkisources->getSourceByName(sources.at(i));
		if (!sd)
			continue;

		if (sd->chn_data->chip_id == USBTENKI_CHIP_MATH) {

			file->write("# Equation for " + sources.at(i).toLocal8Bit() + " : ");

			file->write(g_mathDevice->getChannelEquation(sd->chn_id).toLocal8Bit());

			file->write("\n");

		}
	}
}

void SimpleLogger::run()
{
	timer = new QTimer();
	timer->setInterval(interval_ms);
	connect(timer, SIGNAL(timeout()), this, SLOT(doLog()), Qt::DirectConnection);

	if (!sources.size()) {
		emit logMessage("No sources found. Stopping.");
		return;
	}

	emit logMessage("opening file..");

	file = new QFile(output_file);
	if (append_to_file) {
		file->open(QIODevice::Text | QIODevice::WriteOnly | QIODevice::Append);
	}
	else {
		file->open(QIODevice::Text | QIODevice::WriteOnly);
	}

	writeHeader();

	colTitles();
	doLog();

	timer->start();
	exec();
	timer->stop();

	delete timer;

	file->flush();
	file->close();
	emit logMessage("file closed.");
}

void SimpleLogger::logItem(QString str, int last)
{
	file->write(str.toLocal8Bit());

	if (last) {
		logLineEnd();
		return;
	}

	switch (fmt)
	{
		case SimpleLogger::Csv:
			file->write(", ");
			break;
		case SimpleLogger::Tsv:
			file->write("\t ");
			break;
		case SimpleLogger::Ssv:
			file->write(" ");
			break;
		case SimpleLogger::Scsv:
			file->write("; ");
			break;
	}
}

void SimpleLogger::logError(float v, int last)
{
	switch (on_error)
	{
		case WriteEmpty:
			logItem("", last);
			break;
		case RepeatPrevious:
			logValue(v, last);
			break;
		case WriteZero:
			logValue(0, last);
			break;
		case WriteMinusOne:
			logValue(-1, last);
			break;
		case WriteError:
			logItem("error", last);
			break;
	}
}

void SimpleLogger::logValue(float v, int last)
{
	//char tmpbuf[32];

//	sprintf(tmpbuf, "%0.02f", v);

	// TODO : Potential options:
	//   - scientific notation
	//   - precision (now hardcoded to 2)
	logItem(logLocale->toString(v, 'f', g_tenkisources->displayDigits()), last);
//	logItem(QString::fromLocal8Bit(tmpbuf), last);

}

void SimpleLogger::logLineEnd()
{
	file->write("\n");
	file->flush();
	count++;
	emit logged(count);
}

void SimpleLogger::tsTitlesPre(int step)
{
	(void)step;

	// 0: Source names
	// 1: Measurement type - Unit
	// 2: Aliases

	switch(timestamp_format)
	{
		case None:
			break;

		case SystemShort:
			logItem("Datetime");
			break;

		case SystemLong:
			logItem("Datetime");
			break;

		case ISO8601:
			logItem("Datetime");
			break;

		case SplitISO8601:
			logItem("Date");
			logItem("Time");
			break;

		case ISO8601TimeOnly:
			logItem("Time");
			break;
	}


}

void SimpleLogger::colTitles()
{
	// Source names: AAAAAA:00
	tsTitlesPre(0);
	for (int i=0; i<sources.size(); i++)
	{
		struct sourceDescription *sd;
		sd = tenkisources->getSourceByName(sources.at(i));
		logItem(sd->q_name, i==(sources.size()-1));
	}

	// measurement type - unit
	tsTitlesPre(1);
	for (int i=0; i<sources.size(); i++)
	{
		struct sourceDescription *sd;
		struct USBTenki_channel tmp;

		sd = tenkisources->getSourceByName(sources.at(i));
		if (sd->chn_data->chip_id == USBTENKI_CHIP_MATH) {
			// math data is logged as is - the user selected units, we do not
			// convert them.
			memcpy(&tmp, sd->chn_data, sizeof(tmp));
		} else {
			tenkisources->convertToUnits(sd->chn_data, &tmp);
		}

		logItem(sd->chipShortString + " - " +
		QString::fromLocal8Bit(unitToString(tmp.converted_unit,1))
		, i==(sources.size()-1));
	}

	// aliases
	// this works with the assumption that sources and aliases QLists share
	// the same indices.
	tsTitlesPre(2);
	for (int i=0; i<aliases.size(); i++)
	{
		logItem(aliases.at(i), i==(aliases.size()-1));
	}


}

void SimpleLogger::setUseUTC(bool use)
{
	use_utc = use;
}

void SimpleLogger::setAppend(bool append)
{
	append_to_file = append;
}

void SimpleLogger::setComment(QString comment)
{
	this->comments = comment;
}

void SimpleLogger::doLog()
{
	// Requires QT 4.7
//	if (use_utc) {
//		QDateTime now = QDateTime::currentDateTimeUtc();
//	} else {
		QDateTime now = QDateTime::currentDateTime();
//	}

	switch(timestamp_format)
	{
		case None:
			break;

		case SystemShort:
			logItem(now.toString(Qt::SystemLocaleShortDate));
			break;

		case SystemLong:
			logItem(now.toString(Qt::SystemLocaleLongDate));
			break;

		case ISO8601:
			logItem(now.toString(Qt::ISODate));
			break;

		case SplitISO8601:
			logItem(now.toString("yyyy-MM-dd"));
			logItem(now.toString("hh:mm:ss"));
			break;

		case ISO8601TimeOnly:
			logItem(now.toString("hh:mm:ss"));
			break;
	}


	for (int i=0; i<sources.size(); i++)
	{
		struct sourceDescription *sd;

		sd = tenkisources->getSourceByName(sources.at(i));

		if (sd == NULL) {
			// this would be an internal error.
			emit logMessage("Source '" + sources.at(i)+ "' not found");
		} else {
			// device level error
			if (sd->td->getStatus() != TENKI_DEVICE_STATUS_OK) {
				// call log error with a value. Knowing the converted data
				// will stay at it's last value, this makes it possible for
				// the OnError/RepeatPrevious function to work.
				logError(sd->chn_data->converted_data, i==(sources.size()-1));
				emit logMessage("ERROR: An error occured on '" +sources.at(i) + "'.");
			}
			else {
				// source level error
				if (sd->chn_data->status != USBTENKI_CHN_STATUS_VALID) {
					logError(sd->chn_data->converted_data, i==(sources.size()-1));
					emit logMessage("ERROR: An error occured on '" +sources.at(i) + "': " + usbtenki_getChannelStatusString(sd->chn_data) + ".");
				} else {
					struct USBTenki_channel tmp;
					if (sd->chn_data->chip_id == USBTENKI_CHIP_MATH) {
						// math data is logged as is - the user selected units, we do not
						// convert them.
						memcpy(&tmp, sd->chn_data, sizeof(tmp));
					} else {
						tenkisources->convertToUnits(sd->chn_data, &tmp);
					}
					logValue(tmp.converted_data, i==(sources.size()-1));
				}
			}
		}
	}
}

