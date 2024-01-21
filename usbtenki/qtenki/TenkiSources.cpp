#include "TenkiSources.h"
#include "usbtenki.h"
#include <QTimer>
#include <QDebug>
#include <QSettings>
#include <string.h>
#include "usbtenki_units.h"
#include "globals.h"

TenkiSources::TenkiSources()
{
	QSettings settings;
	struct USBTenki_unitPreferences default_units = {
		.temperature = TENKI_UNIT_CELCIUS,
		.pressure = TENKI_UNIT_KPA,
		.frequency = TENKI_UNIT_HZ,
		.voltage = TENKI_UNIT_VOLTS,
		.current = TENKI_UNIT_AMPS,
		.power = TENKI_UNIT_WATTS,
		.length = TENKI_UNIT_METERS,
		.concentration = TENKI_UNIT_PPM,
	};

	units = default_units;

	recent_pressure_P = 101325;
	display_digits = 4;

	timer_interval = 1000;

	thread = new QThread();
	moveToThread(thread);
	connect(thread, SIGNAL(started()), this, SLOT(run()));

	// Initialize from settings here. ThermocoupleColorPreference is instanciated
	// after we first generate names in DashSensor(). It shows the wrong color for
	// about on update cycle.
	use_iec_thermocouple_colors = settings.value("data/thermocouple_color_system").toInt();

	g_mathDevice = new TenkiMathDevice();
}

TenkiSources::~TenkiSources()
{
	// todo : delete all sourceDescriptions
	delete timer;
}

void TenkiSources::start()
{
	thread->start();
}

void TenkiSources::formatValue(QString *str, float value)
{
	char fmtbuf[16];
	int n = display_digits;

	if (n<0)
		n = 0;
	if (n>9)
		n = 9;

	sprintf(fmtbuf, "%%.%df", n);
	str->sprintf(fmtbuf, value);
}

double TenkiSources::getRecentPressure(void)
{
	return recent_pressure_P;
}

void TenkiSources::setInterval_ms(int interval)
{
	timer_interval = interval;
}

void TenkiSources::setUseIECthermocoupleColors(int use_iec)
{
	use_iec_thermocouple_colors = use_iec;
}

int TenkiSources::getUseIECthermocoupleColors(void)
{
	return use_iec_thermocouple_colors;
}

void TenkiSources::setTemperatureUnit(int tenki_temp_unit)
{
	this->units.temperature = tenki_temp_unit;
}

void TenkiSources::setPressureUnit(int pressure_unit)
{
	this->units.pressure = pressure_unit;
}

void TenkiSources::setFrequencyUnit(int frequency_unit)
{
	this->units.frequency = frequency_unit;
}

void TenkiSources::setDisplayDigits(int digits)
{
	this->display_digits = digits;
}

int TenkiSources::displayDigits()
{
	return display_digits;
}

void TenkiSources::setVoltageUnit(int volt_unit)
{
	this->units.voltage = volt_unit;
}

void TenkiSources::setCurrentUnit(int current_unit)
{
	this->units.current = current_unit;
}

void TenkiSources::setPowerUnit(int power_unit)
{
	this->units.power = power_unit;
}

void TenkiSources::setLengthUnit(int length_unit)
{
	this->units.length = length_unit;
}

void TenkiSources::setConcentrationUnit(int concentration_unit)
{
	this->units.concentration = concentration_unit;
}


void TenkiSources::setReferenceSeaLevelPressure(double value)
{
	usbtenki_set_seaLevelStandardPressure(value);
}

int TenkiSources::init()
{
	usbtenki_init();
	scanForDevices();

	if (g_mathDevice) {
		//printf("Adding math device\n");
		device_list.append(g_mathDevice);
		addDeviceSources(g_mathDevice);
		emit newDeviceFound(g_mathDevice);
	}

	return 0;
}

// TODO : Compare with already existing devices to support Hot plug/unplug.
// For now, please call only once!
int TenkiSources::scanForDevices()
{
	struct USBTenki_list_ctx *ctx;
	struct USBTenki_info info;

	ctx = usbtenki_allocListCtx();
	if (!ctx)
		return -1;

	while (usbtenki_listDevices(&info, ctx)) {
		TenkiDevice *td;

		td = new TenkiDevice(info.str_serial);
		device_list.append(td);

		emit newDeviceFound(td);
		addDeviceSources(td);
	}

	usbtenki_freeListCtx(ctx);

	return 0;
}

int TenkiSources::addDeviceSources(TenkiDevice *td)
{
	int chn = td->getNumChannels();

//qDebug() << "addDeviceSources for " << td->getSerial();

	for (int i=0; i<chn; i++) {
		if (!td->isChannelHidden(i)) {
			addDeviceSource(td, i, td->getChannelData(i));
		}
	}

	return 0;
}

int TenkiSources::addDeviceSource(TenkiDevice *td, int chn_id, struct USBTenki_channel *chndat)
{
	QSettings settings;

	struct sourceDescription *sd;
	const char *serial = td->getSerial();

	if (strlen(serial) > 8) {
		return -1;
	}

	sd = new struct sourceDescription;
	if (!sd)
		return -1;

	sprintf(sd->name, "%s:%02X", serial, chn_id);
	sd->q_name = QString::fromLocal8Bit(sd->name);
//	printf("TenkiSources: Registering source '%s'\n", sd->name);

	sd->td = td;
	sd->chn_id = chn_id;

	sd->chipShortString = QString::fromLocal8Bit(chipToShortString(chndat->chip_id));
	sd->chipString = QString::fromLocal8Bit(chipToString(chndat->chip_id));
	sd->chn_data = chndat;

	sd->q_alias = settings.value("sourcesAliases/"+sd->q_name).toString();

	sourceList.append(sd);

	emit changed();

	return 0;
}

// TODO : Add / remove
void TenkiSources::syncDevicesTo(TenkiDeviceAddRemove *tdr)
{
	for (int i=0; i<device_list.size(); i++)
	{
		tdr->addTenkiDevice(device_list.at(i));
	}
}

void TenkiSources::addSourcesTo(TenkiSourceAddRemove *tsar)
{
	for (int i=0; i<sourceList.size(); i++) {
		tsar->addTenkiSource(sourceList.at(i));
	}
}

void TenkiSources::run()
{
	timer = new QTimer();
	timer->setInterval(timer_interval);
	connect(timer, SIGNAL(timeout()), this, SLOT(doCaptures()));
	doCaptures(); // Initial captures
	timer->start();
}

void TenkiSources::convertToUnits(const struct USBTenki_channel *chn, struct USBTenki_channel *dst)
{
	struct USBTenki_channel tmp;

	if (TENKI_UNIT_IS_PRESSURE(chn->converted_unit)) {
		recent_pressure_P = usbtenki_convertPressure(chn->converted_data, chn->converted_unit, TENKI_UNIT_KPA) * 1000;
	}

	memcpy(&tmp, chn, sizeof(tmp));
	usbtenki_convertUnits(&tmp, &units); //temperature_unit, pressure_unit, frequency_unit, volt_unit, current_unit, power_unit, length_unit);
	memcpy(dst, &tmp, sizeof(tmp));
}

void TenkiSources::doCaptures()
{
	//qDebug() << "Capture time!";

	if (timer_interval != timer->interval()) {
		timer->setInterval(timer_interval);
	}

	for (int i=0; i<device_list.size(); i++) {
	//	printf("Updating %s\n", device_list.at(i)->getSerial());
		device_list.at(i)->updateChannelData();
	}

	emit captureCycleCompleted();
}

struct sourceDescription *TenkiSources::getSourceByName(QString source_name)
{
//	qDebug() << "Looking for " + source_name;

	for (int i=0; i<sourceList.size(); i++) {
		struct sourceDescription *sd = sourceList.at(i);
	
//		qDebug() << "Considering " + sd->q_name;
		
		if (sd->q_name == source_name) {
			return sd;
		}
	}

	return NULL;
}

QString TenkiSources::getSourceAliasByName(QString source_name)
{
//	qDebug() << "Looking for " + source_name;

	for (int i=0; i<sourceList.size(); i++) {
		struct sourceDescription *sd = sourceList.at(i);
	
//		qDebug() << "Considering " + sd->q_name;
		
		if (sd->q_name == source_name) {
			return sd->q_alias;
		}
	}

	return "";
}

void TenkiSources::updateAlias(QString source_name, QString alias)
{
	QSettings settings;
	struct sourceDescription *sd;

	sd = getSourceByName(source_name);
	if (sd)
	{
		sd->q_alias = alias;
		settings.setValue("sourcesAliases/"+source_name, alias);
	
		qDebug() << "Updated alias for source '"+source_name+"' for '" + alias + "'";

		emit changed();
	} else {
		qDebug() << "Source not found: "+source_name;
	}
}
