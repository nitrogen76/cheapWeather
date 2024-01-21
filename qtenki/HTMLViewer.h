#ifndef _HTMLViewer_h__
#define _HTMLViewer_h__

#include <QDialog>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QTimer>
#include <QFile>

class HTMLViewer : public QDialog
{
	Q_OBJECT

	public:
		HTMLViewer(QString filename, QString title);
		~HTMLViewer();

	private:
		QTextEdit *editor;
		QVBoxLayout *layout;
		QDialogButtonBox *btnbox;
		QPushButton *closeBtn;
		QFile *file;
};

#endif // _HTMLViewer_h__


