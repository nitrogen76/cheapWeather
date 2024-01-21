#ifndef _current_preference_h__
#define _current_preference_h__

#include <QComboBox>

class CurrentPreference : public QComboBox
{
	Q_OBJECT

	public:
		CurrentPreference();
		int tenkiUnit();

	private slots:
		void idx_changed(int idx);

	private:
		QString config_key;
};



#endif

