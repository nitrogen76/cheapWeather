#include <QSettings>
#include "VoltagePreference.h"
#include "usbtenki_units.h"
#include "globals.h"

VoltagePreference::VoltagePreference()
{
	QSettings settings;

	config_key = "data/voltage_unit";

	addItem(tr("Volt (V)"));
	addItem(tr("Millivolt (mV)"));

	setCurrentIndex(settings.value(config_key).toInt());

	g_tenkisources->setVoltageUnit(tenkiUnit());

	connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(idx_changed(int)));
}

int VoltagePreference::tenkiUnit(void)
{
	switch(currentIndex()) {
		default:
		case 0:
			return TENKI_UNIT_VOLTS;
		case 1:
			return TENKI_UNIT_MILLIVOLT;
	}
}

void VoltagePreference::idx_changed(int idx)
{
	QSettings settings;

	settings.setValue(config_key, idx);
	
	g_tenkisources->setVoltageUnit(tenkiUnit());
}
