#include <QSettings>
#include "CurrentPreference.h"
#include "usbtenki_units.h"
#include "globals.h"

CurrentPreference::CurrentPreference()
{
	QSettings settings;

	config_key = "data/current_unit";

	addItem(tr("Ampere (A)"));
	addItem(tr("Milliampere (mA)"));

	setCurrentIndex(settings.value(config_key).toInt());

	g_tenkisources->setCurrentUnit(tenkiUnit());

	connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(idx_changed(int)));
}

int CurrentPreference::tenkiUnit(void)
{
	switch(currentIndex()) {
		default:
		case 0:
			return TENKI_UNIT_AMPS;
		case 1:
			return TENKI_UNIT_MILLIAMPS;
	}
}

void CurrentPreference::idx_changed(int idx)
{
	QSettings settings;

	settings.setValue(config_key, idx);
	
	g_tenkisources->setCurrentUnit(tenkiUnit());
}
