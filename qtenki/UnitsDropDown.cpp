#include <QDebug>
#include "UnitsDropDown.h"


UnitsDropDown::UnitsDropDown()
{
	addChoice(tr("Arbitrary unit"), TENKI_UNIT_ARBITRARY);

	addChoice(tr("Temperature: Celsius"), TENKI_UNIT_CELCIUS);
	addChoice(tr("Temperature: Fahrenheit"), TENKI_UNIT_FAHRENHEIT);
	addChoice(tr("Tenperature: Kelvin"), TENKI_UNIT_KELVIN);

	addChoice(tr("Relative Humidity: %"), TENKI_UNIT_RH);

	addChoice(tr("Pressure: kPa"), TENKI_UNIT_KPA);
	addChoice(tr("Pressure: hPa"), TENKI_UNIT_HPA);
	addChoice(tr("Pressure: Bar"), TENKI_UNIT_BAR);
	addChoice(tr("Pressure: At"), TENKI_UNIT_AT);
	addChoice(tr("Pressure: Atm"), TENKI_UNIT_ATM);
	addChoice(tr("Pressure: Torr"), TENKI_UNIT_TORR);
	addChoice(tr("Pressure: PSI"), TENKI_UNIT_PSI);
	addChoice(tr("Pressure: inHg"), TENKI_UNIT_INHG);

	addChoice(tr("Concentration: % "), TENKI_UNIT_PERCENT);
	addChoice(tr("Concentration: ppm"), TENKI_UNIT_PPM);
	addChoice(tr("Concentration: ppb"), TENKI_UNIT_PPB);

	addChoice(tr("Tension: V"), TENKI_UNIT_VOLTS);
	addChoice(tr("Tension: mV"), TENKI_UNIT_MILLIVOLT);

	addChoice(tr("Current: A"), TENKI_UNIT_AMPS);
	addChoice(tr("Current: mA"), TENKI_UNIT_MILLIAMPS);

	addChoice(tr("Power: kW"), TENKI_UNIT_KILOWATTS);
	addChoice(tr("Power: W"), TENKI_UNIT_WATTS);
	addChoice(tr("Power: mW"), TENKI_UNIT_MILLIWATTS);

	addChoice(tr("Frequency: mHz"), TENKI_UNIT_MILLIHZ);
	addChoice(tr("Frequency: Hz"), TENKI_UNIT_HZ);
	addChoice(tr("Frequency: kHz"), TENKI_UNIT_KHZ);
	addChoice(tr("Frequency: MHz"), TENKI_UNIT_MHZ);
	addChoice(tr("Frequency: RPM"), TENKI_UNIT_RPM);

	addChoice(tr("Distance: m"), TENKI_UNIT_METERS);
	addChoice(tr("Distance: dm"), TENKI_UNIT_DECIMETERS);
	addChoice(tr("Distance: cm"), TENKI_UNIT_CENTIMETERS);
	addChoice(tr("Distance: mm"), TENKI_UNIT_MILLIMETERS);
	addChoice(tr("Distance: um"), TENKI_UNIT_MICROMETERS);
	addChoice(tr("Distance: mil"), TENKI_UNIT_MILS);
	addChoice(tr("Distance: inch"), TENKI_UNIT_INCHES);
	addChoice(tr("Distance: ft"), TENKI_UNIT_FEET);
	addChoice(tr("Distance: yard"), TENKI_UNIT_YARDS);

	// addChoice(tr(""), );
}

UnitsDropDown::~UnitsDropDown()
{
	int i;
	for (i=0; i<choices.size(); i++) {
		delete choices.at(i);
	}
	choices.clear();
}

void UnitsDropDown::selectUnits(int units)
{
	int i;

	for (i=0; i<choices.size(); i++) {
		if (choices.at(i)->value == units) {
			setCurrentIndex(i);
			return;
		}
	}
}

int UnitsDropDown::getSelectedUnits()
{
	int i;

	i = currentIndex();

	if (i < choices.size()) {
		return choices.at(i)->value;
	}

	return TENKI_UNIT_RAW;
}

void UnitsDropDown::addChoice(QString caption, int value)
{
	struct unitChoice *choice = new unitChoice;

	choice->caption = caption;
	choice->value = value;
	choices.append(choice);

	addItem(caption);
}

