#include <QDebug>
#include "LineStyleDropDown.h"


LineStyleDropDown::LineStyleDropDown()
{
	addChoice(tr("Solid"), Qt::SolidLine);
	addChoice(tr("DashLine - - - -"), Qt::DashLine);
	addChoice(tr("DotLine . . . ."), Qt::DotLine);
	addChoice(tr("DashDotLine -.-."), Qt::DashDotLine);
	addChoice(tr("DasdhDotDotLine -..-..-.."), Qt::DashDotDotLine);

	// addChoice(tr(""), );
}

LineStyleDropDown::~LineStyleDropDown()
{
	int i;
	for (i=0; i<choices.size(); i++) {
		delete choices.at(i);
	}
	choices.clear();
}

void LineStyleDropDown::selectStyle(Qt::PenStyle style)
{
	int i;

	for (i=0; i<choices.size(); i++) {
		if (choices.at(i)->style == style) {
			setCurrentIndex(i);
			return;
		}
	}
}

Qt::PenStyle LineStyleDropDown::getSelectedStyle()
{
	int i;

	i = currentIndex();

	if (i < choices.size()) {
		return choices.at(i)->style;
	}

	return Qt::SolidLine;
}

void LineStyleDropDown::addChoice(QString caption, Qt::PenStyle style)
{
	struct lineStyleChoice *choice = new lineStyleChoice;

	choice->caption = caption;
	choice->style = style;
	choices.append(choice);

	addItem(caption);
}

