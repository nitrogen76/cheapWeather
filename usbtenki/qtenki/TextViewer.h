#ifndef _TextViewer_h__
#define _TextViewer_h__

#include <QDialog>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QTimer>
#include <QFile>

class TextViewer : public QDialog
{
	Q_OBJECT

	public:
		TextViewer(QString filename);
		~TextViewer();

	public slots:
		void followFile();
		void reloadFile();
	
	private:
		QPlainTextEdit *editor;
		QVBoxLayout *layout;
		QDialogButtonBox *btnbox;
		QPushButton *closeBtn;
		QTimer *timer;	
		QFile *file;
};

#endif // _TextViewer_h__


