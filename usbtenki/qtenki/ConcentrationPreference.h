#ifndef _concentration_preference_h__
#define _concentration_preference_h__

#include <QComboBox>

class ConcentrationPreference : public QComboBox
{
	Q_OBJECT

	public:
		ConcentrationPreference();
		int tenkiUnit();

	private slots:
		void idx_changed(int idx);

	private:
		QString config_key;
};



#endif // _concentration_preference_h__

