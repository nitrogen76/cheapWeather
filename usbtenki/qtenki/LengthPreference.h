#ifndef _length_preference_h__
#define _length_preference_h__

#include <QComboBox>

class LengthPreference : public QComboBox
{
	Q_OBJECT

	public:
		LengthPreference();
		int tenkiUnit();

	private slots:
		void idx_changed(int idx);

	private:
		QString config_key;
};



#endif
