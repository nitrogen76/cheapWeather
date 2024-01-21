#include <QSettings>
#include "FrequencyPreference.h"
#include "usbtenki_units.h"
#include "globals.h"

FrequencyPreference::FrequencyPreference()
{
	QSettings settings;

	config_key = "data/frequency_unit";

	addItem(tr("mHz"));
	addItem(tr("Hz"));
	addItem(tr("kHz"));
	addItem(tr("MHz"));
	addItem(tr("rpm"));

	setCurrentIndex(settings.value(config_key).toInt());

	g_tenkisources->setFrequencyUnit(tenkiUnit());

	connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(idx_changed(int)));
}

int FrequencyPreference::tenkiUnit(void)
{
	switch(currentIndex()) {
		default:
		case 0:
			return TENKI_UNIT_MILLIHZ;
		case 1:
			return TENKI_UNIT_HZ;
		case 2:
			return TENKI_UNIT_KHZ;
		case 3:
			return TENKI_UNIT_MHZ;
		case 4:
			return TENKI_UNIT_RPM;
	}
}

void FrequencyPreference::idx_changed(int idx)
{
	QSettings settings;

	settings.setValue(config_key, idx);
	
	g_tenkisources->setFrequencyUnit(tenkiUnit());
}
