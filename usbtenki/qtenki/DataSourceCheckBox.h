#ifndef _DataSourceCheckBox
#define _DataSourceCheckBox

#include <QString>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include "TenkiSources.h"

class DataSourceCheckBox : public QWidget
{
	Q_OBJECT

	public:
		DataSourceCheckBox(QString caption, QString src);
		~DataSourceCheckBox();
		QString src;	

		QString getAlias();
		bool isChecked();

	public slots:
		void refreshAlias(void);
		
	private slots:
		void checkChanged(int state);

	private:
		QCheckBox *checkbox;
		QLabel *lbl_alias;

};

#endif

