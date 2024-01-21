#include <QDebug>
#include "EditButton.h"

EditButton::EditButton(const QString &text, int id) : QPushButton(text)
{
	buttonid = id;
	connect(this, SIGNAL(clicked()), this, SLOT(was_clicked()));

}

EditButton::~EditButton()
{
}

void EditButton::was_clicked()
{
	emit buttonIdClicked(buttonid);
}

