#ifndef _MainWindow_h__
#define _MainWindow_h__

#include <QWidget>
#include <QCloseEvent>
#include <QSystemTrayIcon>
#include <QTabWidget>
#include <QIcon>

#include "Logger.h"
#include "ConfigPanel.h"
#include "BigView.h"
#include "GraphView.h"

class MainWindow : public QWidget
{
	Q_OBJECT

	public:
		MainWindow();
		~MainWindow();

	protected:
		virtual void closeEvent(QCloseEvent *ev);
		virtual void changeEvent(QEvent *e);
		virtual void readSettings();

	private slots:
		void on_show_hide(QSystemTrayIcon::ActivationReason reason);
		void on_show_hide();
		void loggerStatusChanged(int running);
		void receiveMessage(QString msg);

	private:
		Logger *logger;
		ConfigPanel *cfgPanel;
		BigView *bigView;
		GraphView *graphView;
		QSystemTrayIcon *trayicon;
		QTabWidget *tw;
		QIcon *windowIcon;
		QIcon *trayQIcon;
};

#endif // _MainWindow_h__

