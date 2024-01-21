#include <QSettings>
#include "PowerPreference.h"
#include "usbtenki_units.h"
#include "globals.h"

PowerPreference::PowerPreference()
{
	QSettings settings;

	config_key = "data/power_unit";

	addItem(tr("Watt (W)"));
	addItem(tr("Milliwatt (mW)"));

	setCurrentIndex(settings.value(config_key).toInt());

	g_tenkisources->setPowerUnit(tenkiUnit());

	connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(idx_changed(int)));
}

int PowerPreference::tenkiUnit(void)
{
	switch(currentIndex()) {
		default:
		case 0:
			return TENKI_UNIT_WATTS;
		case 1:
			return TENKI_UNIT_MILLIWATTS;
	}
}

void PowerPreference::idx_changed(int idx)
{
	QSettings settings;

	settings.setValue(config_key, idx);
	
	g_tenkisources->setPowerUnit(tenkiUnit());
}
