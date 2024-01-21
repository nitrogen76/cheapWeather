#include <QTextEdit>
#include <QFile>
#include "About.h"
#include "../common/usbtenki_version.h"

#define HOMEPAGE	"https://www.dracal.com"

About::About()
{
	QTextEdit *te;
	lay = new QVBoxLayout();

	setLayout(lay);

	te = new QTextEdit();
	te->setReadOnly(true);

	lay->addWidget(te);
	
	te->setHtml(
"<h1>QTenki</h1>"
"QTenki is part of "
"USBTenki version " USBTENKI_VERSION "<br>"
USBTENKI_COPYRIGHTS_HTML "<br><br>"
"Visit <a href=\"" HOMEPAGE "\">" HOMEPAGE "</a> for the latest releases."
"<br><br>"
"<hr>"
"<h1>License</h1>"
"USBTenki is released under the terms of the GPL license as below.<hr>"
);

	QFile *file = new QFile(":gpl.txt");
	if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
		while (!file->atEnd()) {
			QByteArray line = file->readLine();
			te->append(line.trimmed());
		}
		file->close();
	}
}

About::~About()
{
}
