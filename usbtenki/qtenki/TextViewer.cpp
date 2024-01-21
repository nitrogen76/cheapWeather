#include "TextViewer.h"
#include <QFile>
#include <QString>
#include <QLabel>
#include <QTimer>

TextViewer::TextViewer(QString filename)
{
	editor = new QPlainTextEdit();
	editor->setReadOnly(true);
	editor->setLineWrapMode(QPlainTextEdit::NoWrap);

	btnbox = new QDialogButtonBox();
	closeBtn = new QPushButton(tr("Close"));
	btnbox->addButton(closeBtn, QDialogButtonBox::AcceptRole);

	QObject::connect(btnbox, SIGNAL(accepted()), this, SLOT(accept()));

	timer = new QTimer(this);
	timer->setInterval(1000); // 1 second
	connect(timer, SIGNAL(timeout()), this, SLOT(followFile()));

	file = new QFile(filename);
	if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
		editor->appendPlainText("Error: Could not open file");
	}
	else {
		while (!file->atEnd()) {
			QByteArray line = file->readLine();
			editor->appendPlainText(line.trimmed());
		}

		timer->start();
	}

	editor->moveCursor(QTextCursor::Start);
	editor->setMinimumWidth(600);
	editor->setMinimumHeight(500);

	layout = new QVBoxLayout();
	setLayout(layout);

	layout->addWidget(new QLabel(tr("File: ") + filename));
	layout->addWidget(editor);
	layout->addWidget(btnbox);
}

void TextViewer::reloadFile()
{
	timer->stop();
	file->seek(0);
	editor->clear();

	while (!file->atEnd()) {
		QByteArray line = file->readLine();
		editor->appendPlainText(line.trimmed());
	}

	timer->start();
}

void TextViewer::followFile()
{
	while (!file->atEnd()) {
		QByteArray line = file->readLine();
		editor->appendPlainText(line.trimmed());
	}
}

TextViewer::~TextViewer()
{
	file->close();
	delete file;
}

