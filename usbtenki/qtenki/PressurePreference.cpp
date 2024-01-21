#include <QSettings>
#include "PressurePreference.h"
#include "usbtenki_units.h"
#include "globals.h"

PressurePreference::PressurePreference()
{
	QSettings settings;

	config_key = "data/pressure_unit";

	addItem(tr("kPa"));
	addItem(tr("hPa"));
	addItem(tr("bar"));
	addItem(tr("at (98.0665 kPa)"));
	addItem(tr("atm (101.325 kPa)"));
	addItem(tr("Torr"));
	addItem(tr("psi"));
	addItem(tr("inHg"));

	setCurrentIndex(settings.value(config_key).toInt());

	g_tenkisources->setPressureUnit(tenkiUnit());

	connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(idx_changed(int)));
}

int PressurePreference::tenkiUnit(void)
{
	switch(currentIndex()) {
		default:
		case 0:
			return TENKI_UNIT_KPA;
		case 1:
			return TENKI_UNIT_HPA;
		case 2:
			return TENKI_UNIT_BAR;
		case 3:
			return TENKI_UNIT_AT;
		case 4:
			return TENKI_UNIT_ATM;
		case 5:
			return TENKI_UNIT_TORR;
		case 6:
			return TENKI_UNIT_PSI;
		case 7:
			return TENKI_UNIT_INHG;
	}
}

void PressurePreference::idx_changed(int idx)
{
	QSettings settings;

	settings.setValue(config_key, idx);

	g_tenkisources->setPressureUnit(tenkiUnit());
}
