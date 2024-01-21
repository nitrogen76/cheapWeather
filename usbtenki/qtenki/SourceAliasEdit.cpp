#include <QSettings>
#include "SourceAliasEdit.h"
#include "globals.h"

SourceAliasEdit::SourceAliasEdit(QString ss)
{
	QSettings settings;

	sourceName = ss;

	setText(g_tenkisources->getSourceAliasByName(ss));

	connect(this, SIGNAL(editingFinished()), this, SLOT(aliasChanged()));
}

void SourceAliasEdit::aliasChanged()
{
	emit sourceAliasChanged(sourceName, text());
}


