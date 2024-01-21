#include <QSettings>
#include "ConfigCheckbox.h"


ConfigCheckbox::ConfigCheckbox(QString caption, QString config_key)
{
	QSettings settings;

	this->config_key = config_key;

	setText(caption);
	setChecked(settings.value(config_key).toBool());

	connect(this, SIGNAL(stateChanged(int)), this, SLOT(update(int)));
}

void ConfigCheckbox::update(int state)
{
	QSettings settings;

	(void)state;

	settings.setValue(config_key, isChecked());

	emit changed();
}

