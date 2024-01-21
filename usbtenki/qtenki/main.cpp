#include "single_application.h"
#include "MainWindow.h"

int main(int argc, char **argv)
{
	SingleApplication app(argc, argv, "QTENKI-INSTANCE-06e23293-16e0-465d-842e-973269331243");

	if (app.isRunning()) {
		app.sendMessage("RAISE");
		return 0;
	}

	MainWindow *window = new MainWindow();

	QObject::connect(&app, SIGNAL(messageAvailable(QString)), window, SLOT(receiveMessage(QString)));

	//app.setStyle(new QCleanlooksStyle());
	
	QFile file(":qtenki.qss");
	file.open(QFile::ReadOnly);
	QString stylesheet = QLatin1String(file.readAll());
	app.setStyleSheet(stylesheet);

//	app.setStyleSheet("QGroupBox { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E0E0E0, stop: 1 #FFFFFF); border-radius: 5px; border: 2px solid grey;}");

	window->show();

	return app.exec();
}
