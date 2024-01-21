#include <QSettings>
#include "ThermocoupleColorPreference.h"
#include "usbtenki_units.h"

#include "globals.h"

ThermocoupleColorPreference::ThermocoupleColorPreference()
{
	QSettings settings;

	config_key = "data/thermocouple_color_system";

	addItem(tr("ANSI"));
	addItem(tr("IEC"));

	setCurrentIndex(settings.value(config_key).toInt());

	g_tenkisources->setUseIECthermocoupleColors(isIEC());

	connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(idx_changed(int)));
}

int ThermocoupleColorPreference::isIEC(void)
{
	if (currentIndex() != 0) {
		return 1;
	}

	return 0;
}

void ThermocoupleColorPreference::idx_changed(int idx)
{
	QSettings settings;

	settings.setValue(config_key, idx);
	g_tenkisources->setUseIECthermocoupleColors(isIEC());
}
