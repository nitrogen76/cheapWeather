#include <QHBoxLayout>
#include <QSettings>
#include <QPushButton>
#include <QColorDialog>
#include <QCheckBox>

#include "SelectableColor.h"


SelectableColor::SelectableColor(QString name, QString caption, QColor default_color)
{
	QSettings settings;
	QHBoxLayout	*l = new QHBoxLayout();
	setLayout(l);

	cb = new QCheckBox(caption);
	
	cb->setChecked(settings.value(name).toBool());
	connect(cb, SIGNAL(stateChanged(int)), this, SLOT(checkChanged(int)));	

	QPushButton *selbtn = new QPushButton(tr("Select color..."));
	connect(selbtn, SIGNAL(clicked()), this, SLOT(doDialog()));

	current_color = default_color;

	if (settings.value(name + "_color").isValid()) {
		current_color = settings.value(name + "_color").value<QColor>();
	}

	l->addWidget(cb);
	l->addWidget(selbtn);

	this->name = name;
}

bool SelectableColor::getSelected()
{
	return cb->isChecked();
}

QString SelectableColor::getName()
{
	return name;
}

QColor SelectableColor::getColor()
{
	return current_color;
}

void SelectableColor::checkChanged(int state)
{
	QSettings settings;
	settings.setValue(name, state);

	emit selectedChanged(name, state, current_color);
}

void SelectableColor::doDialog()
{
	QColor color;
	QSettings settings;

	color = QColorDialog::getColor();

	current_color = color;

	QVariant variant = current_color;

	settings.setValue(name + "_color", variant);

	emit colorChanged(name, current_color);
}

SelectableColor::~SelectableColor()
{
}

