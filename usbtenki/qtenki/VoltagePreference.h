#ifndef _voltage_preference_h__
#define _voltage_preference_h__

#include <QComboBox>

class VoltagePreference : public QComboBox
{
	Q_OBJECT

	public:
		VoltagePreference();
		int tenkiUnit();

	private slots:
		void idx_changed(int idx);

	private:
		QString config_key;
};



#endif

