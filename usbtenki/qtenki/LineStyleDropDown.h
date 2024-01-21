#ifndef _line_style_dropdown_h__
#define _line_style_dropdown_h__

#include <QList>
#include <QComboBox>

struct lineStyleChoice {
	QString caption;
	Qt::PenStyle style;
};

class LineStyleDropDown : public QComboBox
{
	Q_OBJECT

	public:
		LineStyleDropDown();
		~LineStyleDropDown();
		Qt::PenStyle getSelectedStyle();
		void selectStyle(Qt::PenStyle style);

	private:
		void addChoice(QString caption, Qt::PenStyle style);
		QList<struct lineStyleChoice*> choices;

};


#endif

