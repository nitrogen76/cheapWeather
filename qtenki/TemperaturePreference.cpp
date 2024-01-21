#include <QSettings>
#include "TemperaturePreference.h"
#include "usbtenki_units.h"

#include "globals.h"

TemperaturePreference::TemperaturePreference()
{
	QSettings settings;

	config_key = "data/temperature_unit";

	addItem(tr("Celcius"));
	addItem(tr("Fahrenheit"));
	addItem(tr("Kelvin"));

	setCurrentIndex(settings.value(config_key).toInt());

	g_tenkisources->setTemperatureUnit(tenkiUnit());

	connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(idx_changed(int)));
}

int TemperaturePreference::tenkiUnit(void)
{
	switch(currentIndex()) {
		default:
		case 0:
			return TENKI_UNIT_CELCIUS;
		case 1:
			return TENKI_UNIT_FAHRENHEIT;
		case 2:
			return TENKI_UNIT_KELVIN;
	}
}

void TemperaturePreference::idx_changed(int idx)
{
	QSettings settings;

	settings.setValue(config_key, idx);
	
	g_tenkisources->setTemperatureUnit(tenkiUnit());
}
