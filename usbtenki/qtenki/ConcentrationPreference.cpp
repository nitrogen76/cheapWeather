#include <QSettings>
#include "ConcentrationPreference.h"
#include "usbtenki_units.h"

#include "globals.h"

ConcentrationPreference::ConcentrationPreference()
{
	QSettings settings;

	config_key = "data/concentration_unit";

	addItem(tr("Sensor default"));
	addItem(tr("ppb"));
	addItem(tr("ppm"));
	addItem(tr("%"));

	setCurrentIndex(settings.value(config_key).toInt());

	g_tenkisources->setConcentrationUnit(tenkiUnit());

	connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(idx_changed(int)));
}

int ConcentrationPreference::tenkiUnit(void)
{
	switch(currentIndex()) {
		default:
		case 0:
			return TENKI_UNIT_SENSOR_DEFAULT;
		case 1:
			return TENKI_UNIT_PPB;
		case 2:
			return TENKI_UNIT_PPM;
		case 3:
			return TENKI_UNIT_PERCENT;
	}
}

void ConcentrationPreference::idx_changed(int idx)
{
	QSettings settings;

	settings.setValue(config_key, idx);

	g_tenkisources->setConcentrationUnit(tenkiUnit());
}
