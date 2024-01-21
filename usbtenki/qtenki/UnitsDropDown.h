#ifndef _units_dropdown_h__
#define _units_dropdown_h__

#include <QList>
#include <QComboBox>
#include "usbtenki_units.h"

struct unitChoice {
	QString caption;
	int value;
};

class UnitsDropDown : public QComboBox
{
	Q_OBJECT

	public:
		UnitsDropDown();
		~UnitsDropDown();
		int getSelectedUnits();
		void selectUnits(int units);

	private:
		void addChoice(QString caption, int value);
		QList<struct unitChoice*> choices;

};


#endif // _units_dropdown_h__

