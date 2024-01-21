#ifndef _logger_h__
#define _logger_h__

#include <QWidget>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QComboBox>
#include <QGridLayout>
#include "TextViewer.h"
#include "TenkiSources.h"
#include "DataSourceCheckBox.h"
#include "SimpleLogger.h"

class Logger : public QWidget, public TenkiSourceAddRemove
{
	Q_OBJECT

	public:
		Logger(TenkiSources *s);
		~Logger();

		void addTenkiSource(struct sourceDescription *sd);
		void removeTenkiSource(struct sourceDescription *sd);

	public slots:
		void openViewer();
		void browse_clicked();
		void startLogging();
		void stopLogging();
		bool confirmMayExit();
		void clearMessages();
		void saveMessages();

	signals:
		void loggerStatusChanged(int running);

	protected:
		void logMessage(QString str);
		void cannotStartPopup(QString reason, QString hint);

	protected slots:
		void loggerStarted();
		void loggerStopped();
		void loggerMessage(QString str);
		void loggerActivity(int counter);
		void logFormatChanged(int idx);
		void decimalPointChanged(int idx);
		void timestampChanged(int idx);
		void intervalChanged(double i);
		void filenameEdited();
		void commentsEdited();
		void errorStrategyChanged(int);
		void fileExistsStrategyChanged(int);

	private:
		TenkiSources *tenkisources;
		TextViewer *textview;

		QGroupBox *sourcebox;
		QVBoxLayout *svb;
		QList<DataSourceCheckBox*> sources;

		QGroupBox *destbox;
		QGridLayout *dbl;
		QComboBox *comb_fmt;
		QComboBox *comb_decimal;
		QComboBox *comb_timestamp;
		QComboBox *comb_file_exists;
		QComboBox *comb_on_error;
		QLineEdit *path, *file_comments;
		QPushButton *browseButton, *viewButton;

		QDoubleSpinBox *log_interval;

		QWidget *mid_layer; // source and dest
		QHBoxLayout *mid_layout;

		QGroupBox *control;
		QHBoxLayout *control_layout;
		QPushButton *start_button, *stop_button, *clr_messages_button, *save_messages_button;
		QLabel *status_label, *counter_label;
	
		QGroupBox *messages;
		QTextEdit *msgtxt;
			
		QVBoxLayout *main_layout;

		SimpleLogger *current_logger;
		char logging_just_started;
};

#endif // _logger_h__

