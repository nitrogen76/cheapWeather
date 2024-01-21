#include <QSettings>
#include "LengthPreference.h"
#include "usbtenki_units.h"
#include "globals.h"

LengthPreference::LengthPreference()
{
	QSettings settings;

	config_key = "data/length_unit";

	addItem(tr("millimeters"));
	addItem(tr("centimeters"));
	addItem(tr("decimeters"));
	addItem(tr("meters"));
	addItem(tr("mils (1/1000in)"));
	addItem(tr("inches"));
	addItem(tr("feet"));
	addItem(tr("yards"));

	setCurrentIndex(settings.value(config_key).toInt());

	g_tenkisources->setLengthUnit(tenkiUnit());

	connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(idx_changed(int)));
}

int LengthPreference::tenkiUnit(void)
{
	switch(currentIndex()) {
		default:
		case 0:
			return TENKI_UNIT_MILLIMETERS;
		case 1:
			return TENKI_UNIT_CENTIMETERS;
		case 2:
			return TENKI_UNIT_DECIMETERS;
		case 3:
			return TENKI_UNIT_METERS;
		case 4:
			return TENKI_UNIT_MILS;
		case 5:
			return TENKI_UNIT_INCHES;
		case 6:
			return TENKI_UNIT_FEET;
		case 7:
			return TENKI_UNIT_YARDS;
	}
}

void LengthPreference::idx_changed(int idx)
{
	QSettings settings;

	settings.setValue(config_key, idx);

	g_tenkisources->setLengthUnit(tenkiUnit());
}
