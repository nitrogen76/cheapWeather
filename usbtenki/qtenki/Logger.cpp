#include "Logger.h"
#include "TextViewer.h"

#include <QSettings>
#include <QDir> 
#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>

#define EXISTS_OVERWRITE	0
#define EXISTS_APPEND		1
#define EXISTS_CONFIRM		2

Logger::Logger(TenkiSources *s)
{
	int y=0;
	QSettings settings;
	QLocale locale;

	tenkisources = s;

	current_logger = NULL;
	textview = NULL;
	
	main_layout = new QVBoxLayout();
	setLayout(main_layout);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	/* Source channels selection */
	sourcebox = new QGroupBox(tr("Data sources"));
	svb = new QVBoxLayout();
	sourcebox->setLayout(svb);
	
	// have tenkisource call our addTenkiSource in loop for
	// each source present.
	tenkisources->addSourcesTo(this);

//	svb->addWidget(new QPushButton("Test"));

	svb->addStretch();

	/* Output selection */
	destbox = new QGroupBox(tr("Output configuration"));
	dbl = new QGridLayout();
	destbox->setLayout(dbl);

	comb_fmt = new QComboBox();
	/* Note: Added in this order to match SimpleLogger::FileFormat */
	comb_fmt->addItem(tr("Comma"));
	comb_fmt->addItem(tr("Tab"));
	comb_fmt->addItem(tr("Space"));
	comb_fmt->addItem(tr("Semicolon"));
	comb_fmt->setCurrentIndex(settings.value("logger/format").toInt());
	connect(comb_fmt, SIGNAL(currentIndexChanged(int)), this, SLOT(logFormatChanged(int)));

	dbl->addWidget(new QLabel(tr("Field separator:")), y, 0 );
	dbl->addWidget(comb_fmt, y, 1);
//	y++;
	
	comb_decimal = new QComboBox();
	/* Note: Added in order matching SimpleLogger::DecimalType */
	comb_decimal->addItem(tr("System default"));
	comb_decimal->addItem(tr("Period: ")+" .");
	comb_decimal->addItem(tr("Comma: ")+" ,");
	comb_decimal->setCurrentIndex(settings.value("logger/decimal_point").toInt());
	connect(comb_decimal, SIGNAL(currentIndexChanged(int)), this, SLOT(decimalPointChanged(int)));
	dbl->addWidget(new QLabel(tr("Decimal point:")), y, 2 );
	dbl->addWidget(comb_decimal, y, 3);
	y++;

	comb_timestamp = new QComboBox();
	/* Note: Added in order matching SimpleLogger::TimeStampFormat */
	comb_timestamp->addItem(tr("None"));
	comb_timestamp->addItem(tr("Short system-specific format"));
	comb_timestamp->addItem(tr("Long system-specific format"));
	comb_timestamp->addItem(tr("ISO8601 (single field YYYY-MM-DDTHH:MM:SS)"));
	comb_timestamp->addItem(tr("ISO8601 (date YYYY-MM-DD and hour HH:MM:SS across two fields)"));
	comb_timestamp->addItem(tr("ISO8601 Time only (HH:MM:SS)"));


	comb_timestamp->setCurrentIndex(settings.value("logger/timestamp").toInt());
	connect(comb_timestamp, SIGNAL(currentIndexChanged(int)), this, SLOT(timestampChanged(int)));
	dbl->addWidget(new QLabel(tr("Timestamps:")), y, 0 );
	dbl->addWidget(comb_timestamp, y, 1, 1, 4);
	y++;


	comb_on_error = new QComboBox();
	/* Note: Added in order matching SimpleLogger::OnError */
	comb_on_error->addItem(tr("Write empty field(s)"));
	comb_on_error->addItem(tr("Repeat previous value(s)"));
	comb_on_error->addItem(tr("Write 0 in field(s)"));
	comb_on_error->addItem(tr("Write -1 in field(s)"));
	comb_on_error->addItem(tr("Write 'error' in field(s)"));

	comb_on_error->setCurrentIndex(settings.value("logger/on_error").toInt());
	connect(comb_on_error, SIGNAL(currentIndexChanged(int)), this, SLOT(errorStrategyChanged(int)));
	dbl->addWidget(new QLabel(tr("On error:")), y, 0);
	dbl->addWidget(comb_on_error, y, 1, 1, 4);
	y++;

	/* If file already exists, what to do? */
	comb_file_exists = new QComboBox();
	comb_file_exists->addItem(tr("Overwrite file without confirmation"));
	comb_file_exists->addItem(tr("Append to file without confirmation"));
	comb_file_exists->addItem(tr("Overwrite file with confirmation"));
	comb_file_exists->addItem(tr("Append to file with confirmation"));

	comb_file_exists->setCurrentIndex(settings.value("logger/if_file_exists").toInt());
	connect(comb_file_exists, SIGNAL(currentIndexChanged(int)), this, SLOT(fileExistsStrategyChanged(int)));
	dbl->addWidget(new QLabel(tr("If file exists:")));
	dbl->addWidget(comb_file_exists, y, 1, 1, 4);
	y++;

	dbl->addWidget(new QLabel(tr("Output file:")), y, 0 );
	path = new QLineEdit(settings.value("logger/filename").toString());
	browseButton = new QPushButton(QIcon(":fileopen.png"), tr("Browse..."));
	dbl->addWidget(path, y, 1, 1, 4);
	y++;

	dbl->addWidget(browseButton, y, 4, 1, 1);
	y++;

	connect(path, SIGNAL(editingFinished()), this, SLOT(filenameEdited()));

	QObject::connect(browseButton, SIGNAL(clicked()), this, SLOT(browse_clicked()));

	dbl->addWidget(new QLabel(tr("Logging interval:")), y, 0 );
	log_interval = new QDoubleSpinBox();
	log_interval->setDecimals(1);
	log_interval->setSingleStep(0.1);
	log_interval->setMinimum(0.1);
	log_interval->setMaximum(31536000); // one year
	dbl->addWidget(log_interval, y, 1, 1, 1);
	dbl->addWidget(new QLabel(tr("(seconds)")), y, 2 );

	int compat_interval_s = settings.value("logger/interval", 1).toInt();
	log_interval->setValue(settings.value("logger/interval_ms", compat_interval_s*1000).toDouble() / 1000);
	connect(log_interval, SIGNAL(valueChanged(double)), this, SLOT(intervalChanged(double)));
	y++;


	// comments
	dbl->addWidget(new QLabel(tr("Log comment:")), y, 0);
	file_comments = new QLineEdit(settings.value("logger/file_comments").toString());
	dbl->addWidget(file_comments, y, 1, 1, 4);
	connect(file_comments, SIGNAL(editingFinished()), this, SLOT(commentsEdited()));
	y++;

	control = new QGroupBox(tr("Control"));
	control_layout = new QHBoxLayout();
	control->setLayout(control_layout);
	start_button = new QPushButton(QIcon(":record.png"), tr("Start"));
	stop_button = new QPushButton(QIcon(":stop.png"), tr("Stop"));
	stop_button->setEnabled(false);
	status_label = new QLabel(tr("Not running."));
	counter_label = new QLabel("0");

	viewButton = new QPushButton(QIcon(":view.png"), tr("View file"));
	QObject::connect(viewButton, SIGNAL(clicked()), this, SLOT(openViewer()));

	control_layout->addWidget(start_button);
	control_layout->addWidget(stop_button);
	control_layout->addWidget(viewButton);

	control_layout->addWidget(status_label);
	control_layout->addWidget(new QLabel(tr("Lines written: ")));
	control_layout->addWidget(counter_label);
	control_layout->addStretch();

	messages = new QGroupBox("Messages");
	QVBoxLayout *msg_layout;
	msg_layout = new QVBoxLayout();
	messages->setLayout(msg_layout);
	msgtxt = new QTextEdit();
	msgtxt->setReadOnly(true);

	QHBoxLayout *msg_control_layout = new QHBoxLayout();
	QFrame *msg_control_frm = new QFrame();
	msg_control_frm->setLayout(msg_control_layout);

	clr_messages_button = new QPushButton(QIcon(":document-close.png"), tr("Clear messages"));
	save_messages_button = new QPushButton(QIcon(":fileopen.png"), tr("Save messages..."));
	connect(clr_messages_button, SIGNAL(clicked()), this, SLOT(clearMessages()));
	connect(save_messages_button, SIGNAL(clicked()), this, SLOT(saveMessages()));
	msg_control_layout->addWidget(clr_messages_button);
	msg_control_layout->addWidget(save_messages_button);
	msg_control_layout->addStretch();

	msg_layout->addWidget(msgtxt);
	msg_layout->addWidget(msg_control_frm);

	connect(start_button, SIGNAL(clicked()), this, SLOT(startLogging()));
	connect(stop_button, SIGNAL(clicked()), this, SLOT(stopLogging()));

	/* Layout */
	mid_layer = new QWidget();
	mid_layout = new QHBoxLayout();
	mid_layer->setLayout(mid_layout);
	mid_layout->addWidget(sourcebox);
	mid_layout->addWidget(destbox);

	main_layout->addWidget(mid_layer);	

	main_layout->addWidget(control);
	main_layout->addWidget(messages);
	main_layout->setStretchFactor(messages, 100);
}

Logger::~Logger()
{
}

void Logger::addTenkiSource(struct sourceDescription *sd)
{
	 DataSourceCheckBox *cb = new DataSourceCheckBox(sd->q_name + "  --  " + sd->chipShortString, sd->q_name);
	 svb->addWidget(cb);
	 sources.append(cb);

	 connect(tenkisources, SIGNAL(changed()), cb, SLOT(refreshAlias()));
}

void Logger::removeTenkiSource(struct sourceDescription *sd)
{
	(void)sd;
}

void Logger::logMessage(QString str)
{
	QDateTime now = QDateTime::currentDateTime();
	msgtxt->append(now.toString("(yyyy-MM-dd hh:mm:ss)") + " " + str);
}

void Logger::loggerMessage(QString str)
{
	logMessage("Logger: "+str);
}

void Logger::cannotStartPopup(QString reason, QString hint)
{
	QMessageBox msgBox;
	msgBox.setText(reason);
	msgBox.setInformativeText(hint);
	msgBox.setStandardButtons(QMessageBox::Ok);
	msgBox.setIcon(QMessageBox::Warning);
	msgBox.exec();
}

void Logger::startLogging()
{
	int ex = comb_file_exists->currentIndex();

	/* Make sure a file was specified.
	 * TODO: Test for write access? */

	if (0 == path->text().size()) {
		cannotStartPopup(tr("No file selected."), tr("You must select an output file."));
		return;
	}

	QFileInfo qf = QFileInfo(path->text());
	if (qf.isFile()) {
		logMessage("warning: File already exists");

		if (ex & EXISTS_CONFIRM) {
			QMessageBox msgBox;
			msgBox.setText("File already exists. Continue?");
			msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);	
			msgBox.setIcon(QMessageBox::Warning);
			int selection = msgBox.exec();
			if (selection != QMessageBox::Yes) {
				logMessage("Logging cancelled by user.");
				return;
			}
		}

		if (!qf.isWritable()) {
			cannotStartPopup(tr("File is not writeable."), tr("Select another file or check permissions."));
			return;
		}
	}
	
	if (comb_fmt->currentIndex() == -1) {
		QMessageBox msgBox;
		
		msgBox.setText(tr("No file format selected."));
		msgBox.setInformativeText(tr("Please select an output file format."));
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
		return;
	}
	
	SimpleLogger::FileFormat fmt = SimpleLogger::Csv;
	
	switch(comb_fmt->currentIndex()) {
		case 0: fmt = SimpleLogger::Csv; break;
		case 1: fmt = SimpleLogger::Tsv; break;
		case 2: fmt = SimpleLogger::Ssv; break;
		case 3: fmt = SimpleLogger::Scsv;break;
	}

	SimpleLogger::DecimalType dt = SimpleLogger::SystemFormat;

	switch(comb_decimal->currentIndex()) {
		case 0: dt = SimpleLogger::SystemFormat;break;
		case 1: dt = SimpleLogger::Period;		break;
		case 2: dt = SimpleLogger::Comma;		break;
	}

	SimpleLogger::TimeStampFormat tfmt = SimpleLogger::None;
	switch(comb_timestamp->currentIndex()) {
		case 0: tfmt = SimpleLogger::None;				break;
		case 1: tfmt = SimpleLogger::SystemShort;		break;
		case 2: tfmt = SimpleLogger::SystemLong;		break;
		case 3: tfmt = SimpleLogger::ISO8601;			break;
		case 4: tfmt = SimpleLogger::SplitISO8601;		break;
		case 5: tfmt = SimpleLogger::ISO8601TimeOnly;	break;
	}

	SimpleLogger::OnError onerr = SimpleLogger::WriteEmpty;
	switch(comb_on_error->currentIndex()) {
		case 0: onerr = SimpleLogger::WriteEmpty;		break;
		case 1: onerr = SimpleLogger::RepeatPrevious; 	break;
		case 2: onerr = SimpleLogger::WriteZero; 		break;
		case 3: onerr = SimpleLogger::WriteMinusOne; 	break;
		case 4: onerr = SimpleLogger::WriteError; 		break;
	}

	current_logger = new SimpleLogger(tenkisources, path->text(), log_interval->value()*1000, fmt, dt, tfmt, onerr);
	current_logger->setAppend(ex & EXISTS_APPEND ? true : false);
	current_logger->setComment(file_comments->text());

	for (int i=0; i<sources.size(); i++) {
		DataSourceCheckBox *cb = sources.at(i);
		if (cb->isChecked()) {
			current_logger->addSource(cb->src, cb->getAlias());
		}
	}

	start_button->setEnabled(false);
	stop_button->setEnabled(true);
	mid_layer->setEnabled(false);
	logMessage("Starting logger...");

	QObject::connect(current_logger, SIGNAL(started()), this, SLOT(loggerStarted()));	
	QObject::connect(current_logger, SIGNAL(finished()), this, SLOT(loggerStopped()));	
	QObject::connect(current_logger, SIGNAL(logMessage(QString)), this, SLOT(loggerMessage(QString)));
	QObject::connect(current_logger, SIGNAL(logged(int)), this, SLOT(loggerActivity(int)));
	current_logger->start();
	logging_just_started = 1;


}

bool Logger::confirmMayExit()
{
	if (current_logger != NULL) {
		QMessageBox msgBox;

		msgBox.setText(tr("Logger still running."));
		msgBox.setInformativeText(tr("Exit anyway?"));
		msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
		msgBox.setIcon(QMessageBox::Warning);
		if (msgBox.exec() == QMessageBox::Cancel) {
			return false;
		}
	}

	return true;
}

void Logger::stopLogging()
{
	logMessage("Stopping logger...");
	current_logger->quit();
	logMessage("Waiting for logger thread to stop...");
	status_label->setText(tr("Not running."));
	current_logger->wait();
}

void Logger::openViewer()
{
	if (textview) {
		delete textview;
	}
	textview = new TextViewer(path->text());
	//t->exec();
	textview->show();
}

void Logger::browse_clicked()
{
	QString filename;
	QString default_dir;

	if (0 == path->text().size()) {
		default_dir = QDir::homePath();
	} else {
		default_dir = path->text();
	}
	filename = QFileDialog::getSaveFileName(this, tr("Output file"), default_dir, "(*.txt *.csv *.tsv)" );

	if (filename.size()) {
		path->setText(filename);
	}

	filenameEdited(); // QLineEdit does not emit a signal in this case.
}

void Logger::loggerStarted()
{
	emit loggerStatusChanged(1);

	logMessage("logger started successfully");
	status_label->setText(tr("Running."));
}

void Logger::loggerActivity(int count)
{
	QString str;
	str.setNum(count);
	counter_label->setText(str);

	// This is used to reload the file at a point where
	// we know the logger has written to the file. This
	// is necessary otherwier the text window is not
	// cleared when the file was truncated.
	if (logging_just_started) {
		if (textview) {
			textview->reloadFile();
		}
		logging_just_started = 0;
	}
}

void Logger::loggerStopped()
{
	logMessage("logger stopped successfully");

	start_button->setEnabled(true);
	stop_button->setEnabled(false);
	mid_layer->setEnabled(true);

	delete current_logger;
	current_logger = NULL;

	emit loggerStatusChanged(0);
}

void Logger::fileExistsStrategyChanged(int idx)
{
	QSettings settings;
	settings.setValue("logger/if_file_exists", idx);
}

void Logger::errorStrategyChanged(int idx)
{
	QSettings settings;
	settings.setValue("logger/on_error", idx);
}

void Logger::timestampChanged(int idx)
{
	QSettings settings;
	settings.setValue("logger/timestamp", idx);
}

void Logger::decimalPointChanged(int idx)
{
	QSettings settings;
	settings.setValue("logger/decimal_point", idx);
}

void Logger::logFormatChanged(int idx)
{
	QSettings settings;
	settings.setValue("logger/format", idx);
}

void Logger::intervalChanged(double i)
{
	QSettings settings;
	settings.setValue("logger/interval_ms", i*1000);
}

void Logger::filenameEdited()
{
	QSettings settings;
	settings.setValue("logger/filename", path->text());
}

void Logger::commentsEdited()
{
	QSettings settings;
	settings.setValue("logger/file_comments", file_comments->text());
}

void Logger::clearMessages()
{
	msgtxt->clear();
}

void Logger::saveMessages()
{
	QString filename;
	QString default_dir;

	if (0 == path->text().size()) {
		default_dir = QDir::homePath();
	} else {
		default_dir = path->text();
	}
	filename = QFileDialog::getSaveFileName(this, tr("Output file"), default_dir, "(*.txt)" );

	if (filename.size()) {
		QFile outfile;
		outfile.setFileName(filename);
		outfile.open(QIODevice::WriteOnly | QIODevice::Text);
		outfile.write(msgtxt->toPlainText().toUtf8());
		outfile.close();
	}
}

