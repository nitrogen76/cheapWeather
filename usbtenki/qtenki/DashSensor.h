#ifndef _dashsensor_h__
#define _dashsensor_h__

#include <QGroupBox>

class DashSensor : public QGroupBox
{
	Q_OBJECT

	public:
		virtual void refresh() = 0;
};

#endif

