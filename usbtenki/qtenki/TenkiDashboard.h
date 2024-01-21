#ifndef _tenki_dashboard_h__
#define _tenki_dashboard_h__

#include <QWidget>
#include <QList>
#include <QLabel>
#include <QVBoxLayout>
#include "DashSensor.h"
#include "TenkiSources.h"
#include "TenkiDevice.h"

class TenkiDashboard : public QWidget, public TenkiDeviceAddRemove
{
	Q_OBJECT

	public:
		TenkiDashboard();
		~TenkiDashboard();
		void addDashSensor(DashSensor *ds);

		void addTenkiDevice(TenkiDevice *td);
		void removeTenkiDevice(TenkiDevice *td);

	public slots:
		void refreshView(void);

	private:
		QLabel *mainLabel;
		QVBoxLayout *vbox;
		QList<DashSensor*> sensors;
		QLabel *nosensorsLabel;
};

#endif

