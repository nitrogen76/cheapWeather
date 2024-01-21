#include "DataSourceCheckBox.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QSettings>
#include "globals.h"

DataSourceCheckBox::DataSourceCheckBox(QString caption, QString src)
{
	QSettings settings;

	checkbox = new QCheckBox();
	checkbox->setText(caption);

	checkbox->setChecked(settings.value("sourcesChecked/"+src).toBool());

	if (g_tenkisources->getSourceAliasByName(src).length() > 0) {
		lbl_alias = new QLabel("(" + g_tenkisources->getSourceAliasByName(src) + ")");
	} else {
		lbl_alias = new QLabel("");
	}

//	alias_edit = new QLineEdit();
//	alias_edit->setText(settings.value("sourcesAliases/"+src).toString());
	
	this->src = src;

	QHBoxLayout *lay = new QHBoxLayout();
	lay->setContentsMargins(0,0,0,0);
	setLayout(lay);

	lay->addWidget(checkbox);
//	lay->addWidget(new QLabel(tr("Alias:")));
	lay->addWidget(lbl_alias);
	lay->addStretch();

//	alias_edit->setMinimumWidth(150);
//	alias_edit->setMaximumWidth(200);

//	connect(alias_edit, SIGNAL(editingFinished()), this, SLOT(aliasChanged()));
	connect(checkbox, SIGNAL(stateChanged(int)), this, SLOT(checkChanged(int)));
}

QString DataSourceCheckBox::getAlias()
{
	return g_tenkisources->getSourceAliasByName(src);
}

bool DataSourceCheckBox::isChecked()
{
	return checkbox->isChecked();
}

void DataSourceCheckBox::refreshAlias(void)
{
	lbl_alias->setText(g_tenkisources->getSourceAliasByName(src));
}

void DataSourceCheckBox::checkChanged(int st)
{
	QSettings settings;
	settings.setValue("sourcesChecked/"+src, checkbox->isChecked());
	(void)st;
}

DataSourceCheckBox::~DataSourceCheckBox()
{
}
