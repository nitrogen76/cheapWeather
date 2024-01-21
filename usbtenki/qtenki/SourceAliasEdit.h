#ifndef _SourceAliasEdit_h__
#define _SourceAliasEdit_h__

#include <QLineEdit>

class SourceAliasEdit : public QLineEdit
{
	Q_OBJECT

	public:
		SourceAliasEdit(QString src_name);

	signals:
		void sourceAliasChanged(QString src_name, QString new_alias);

	private slots:
		void aliasChanged();

	private:
		QString sourceName;
};

#endif // _SourceAliasEdit_h__

