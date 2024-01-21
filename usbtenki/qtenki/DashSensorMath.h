#ifndef _dashsensor_math_h__
#define _dashsensor_math_h__

#include <QGroupBox>
#include <QLabel>
#include <QGridLayout>
#include <QString>
#include <QList>
#include <QMap>
#include <QString>
#include <QLineEdit>

#include "TenkiDevice.h"
#include "MinMaxResettable.h"
#include "DashSensor.h"
#include "globals.h"
#include "MathEditDialog.h"

class DashSensorMath : public DashSensor
{
	Q_OBJECT

	public:
		DashSensorMath(TenkiMathDevice *td);
		~DashSensorMath();

		TenkiMathDevice *tenki_device;
		void refresh();

	private slots:
		void editClicked(int id);
		void equationEdited();
		void helpClicked(bool);

	private:
		void addChannel(int channel, int row);
		QString title;
		QGridLayout *layout;
		QList<QLineEdit*> equations;
		QList<QLabel*> values;
		QList<QLabel*> units;
		QList<MinMaxResettable*> minimums;
		QList<MinMaxResettable*> maximums;
		QList<int> channel_id;
		QList<int> chip_ids;
//		MinMaxResettable *min, *max;

		MathEditDialog *editDialog;
		QPushButton *helpbtn;

		void recolorizeThermocouple(void);
		int prev_iec;
};

#endif

