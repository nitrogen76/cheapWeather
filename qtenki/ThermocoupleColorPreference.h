#ifndef _thermocoupleColor_preference_h__
#define _thermocoupleColor_preference_h__

#include <QComboBox>

class ThermocoupleColorPreference : public QComboBox
{
	Q_OBJECT

	public:
		ThermocoupleColorPreference();
		int isIEC();

	private slots:
		void idx_changed(int idx);

	private:
		QString config_key;
};



#endif // _thermocoupleColor_preference_h__

