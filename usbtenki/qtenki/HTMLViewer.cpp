#include "HTMLViewer.h"
#include <QFile>
#include <QString>
#include <QLabel>
#include <QTimer>

HTMLViewer::HTMLViewer(QString filename, QString title)
{
	setWindowTitle(title);

	editor = new QTextEdit();
	editor->setReadOnly(true);

	btnbox = new QDialogButtonBox();
	closeBtn = new QPushButton(tr("Close"));
	btnbox->addButton(closeBtn, QDialogButtonBox::AcceptRole);

	QObject::connect(btnbox, SIGNAL(accepted()), this, SLOT(accept()));

	file = new QFile(filename);
	if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
		editor->append("Error: Could not open file");
	}
	else {
		QByteArray data = file->readAll();
		editor->append(data);
	}

	editor->moveCursor(QTextCursor::Start);
	editor->setMinimumWidth(600);
	editor->setMinimumHeight(500);

	layout = new QVBoxLayout();
	setLayout(layout);

	//layout->addWidget(new QLabel(tr("File: ") + filename));
	layout->addWidget(editor);
	layout->addWidget(btnbox);
}

HTMLViewer::~HTMLViewer()
{
	file->close();
	delete file;
}

