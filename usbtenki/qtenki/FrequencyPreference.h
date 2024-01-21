#ifndef _frequency_preference_h__
#define _frequency_preference_h__

#include <QComboBox>

class FrequencyPreference : public QComboBox
{
	Q_OBJECT

	public:
		FrequencyPreference();
		int tenkiUnit();

	private slots:
		void idx_changed(int idx);

	private:
		QString config_key;
};



#endif // _frequency_preference_h__

