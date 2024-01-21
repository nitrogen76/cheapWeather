#ifndef _power_preference_h__
#define _power_preference_h__

#include <QComboBox>

class PowerPreference : public QComboBox
{
	Q_OBJECT

	public:
		PowerPreference();
		int tenkiUnit();

	private slots:
		void idx_changed(int idx);

	private:
		QString config_key;
};



#endif

