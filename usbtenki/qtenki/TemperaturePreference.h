#ifndef _temperature_preference_h__
#define _temperature_preference_h__

#include <QComboBox>

class TemperaturePreference : public QComboBox
{
	Q_OBJECT

	public:
		TemperaturePreference();
		int tenkiUnit();

	private slots:
		void idx_changed(int idx);

	private:
		QString config_key;
};



#endif // _temperature_preference_h__

