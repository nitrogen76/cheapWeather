#include <iostream>
#include <QSpinBox>
#include <QGroupBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QSettings>
#include <QColorDialog>
#include "GraphView.h"
#include "ConfigCheckbox.h"
#include "GraphLineStyleDialog.h"
#include "qcustomplot.h"
#include "globals.h"

#define DEFAULT_GRAPH_LINE_WIDTH	2

GraphView::GraphView()
{
	QSettings settings;

	lay = new QVBoxLayout();
	this->setLayout(lay);
	
	is_paused = 0;
	x_count = 0;
	x_max = settings.value("graph/xmax", 8000).toInt();

	plt = new QCustomPlot(this);
	
	plt->xAxis->setLabel("Samples");
	plt->yAxis->setLabel("Value");
	plt->setRangeDrag(Qt::Horizontal);
	plt->setRangeZoom(Qt::Horizontal);
	plt->setInteractions(QCustomPlot::iRangeZoom | QCustomPlot::iRangeDrag);
	//plt->setInteractions(QCustomPlot::iRangeZoom | QCustomPlot::iRangeDrag | QCustomPlot::iSelectLegend);

	connect(plt, SIGNAL(titleClick(QMouseEvent*)), this, SLOT(editTitle()));
	connect(plt, SIGNAL(legendClick(QCPLegend*, QCPAbstractLegendItem*, QMouseEvent *)), this, SLOT(editLegend(QCPLegend*, QCPAbstractLegendItem*, QMouseEvent *)));
	
	QFont legendFont = font();
	legendFont.setPointSize(9);
	plt->legend->setVisible(true);
	plt->legend->setFont(legendFont);
	plt->legend->setPositionStyle(QCPLegend::psBottomRight);

	plt->setAutoAddPlottableToLegend(true);
	plt->setTitle(tr("Untitled graph"));
	plt->setNoAntialiasingOnDrag(true);
	plt->setNotAntialiasedElements(QCP::aeAll);

	g_tenkisources->addSourcesTo(this);
	

	//////////////////////////
	QGroupBox *graph_opts = new QGroupBox(tr("Operations"));
	graph_opts->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	QHBoxLayout *graph_opts_lay = new QHBoxLayout();
	graph_opts->setLayout(graph_opts_lay);

	btn_pause_continue = new QPushButton(tr("Pause"));
	connect(btn_pause_continue, SIGNAL(clicked()), this, SLOT(pause_unpause()));

	QPushButton *btn_reset = new QPushButton(tr("Reset graph"));
	connect(btn_reset, SIGNAL(clicked()), this, SLOT(resetGraph()));
	
	QPushButton *btn_save = new QPushButton(QIcon(":fileopen.png"), tr("Save graph to file..."));
	connect(btn_save, SIGNAL(clicked()), this, SLOT(saveGraph()));
	

	// Sample interval
	QSpinBox *sample_interval = new QSpinBox();
	sample_interval->setMinimum(settings.value("graph/minimum_sample_interval_ms", 100).toInt());
	sample_interval->setMaximum(settings.value("graph/minimum_sample_interval_ms", 60 * 60 * 1000).toInt()); // 1 per hour
	sample_interval->setValue(settings.value("graph/sample_interval_ms", 1000).toInt());
	connect(sample_interval, SIGNAL(valueChanged(int)), this, SLOT(intervalChanged(int)));

	graph_opts_lay->addWidget(btn_pause_continue);
	graph_opts_lay->addWidget(btn_reset);
	graph_opts_lay->addWidget(btn_save);
	graph_opts_lay->addStretch();
	
	////////////////////////// OPTIONS
	QGroupBox *graph_opts2 = new QGroupBox(tr("Options"));
	graph_opts2->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

	QVBoxLayout *graph_opts_vbox = new QVBoxLayout();
	graph_opts_vbox->setContentsMargins(0,0,0,0);
	graph_opts2->setLayout(graph_opts_vbox);

	QHBoxLayout *graph_opts_lay2 = new QHBoxLayout();
	QHBoxLayout *graph_opts_lay3 = new QHBoxLayout();
	graph_opts_lay2->setContentsMargins(0,0,0,0);
	graph_opts_lay3->setContentsMargins(0,0,0,0);

	QFrame *line1 = new QFrame();
	line1->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	line1->setLayout(graph_opts_lay2);
	QFrame *line2 = new QFrame();
	line2->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	line2->setLayout(graph_opts_lay3);

	graph_opts_vbox->addWidget(line1);
	graph_opts_vbox->addWidget(line2);


	graph_rescale_x = new ConfigCheckbox(tr("Auto-scale X axis"), "graph/autoscale_x");
	graph_rescale_y = new ConfigCheckbox(tr("Auto-scale Y axis"), "graph/autoscale_y");
	graph_log_y = new ConfigCheckbox(tr("Logarithmic Y axis"), "graph/log_y");

	connect(graph_rescale_x, SIGNAL(changed()), this, SLOT(replot()));
	connect(graph_rescale_y, SIGNAL(changed()), this, SLOT(replot()));
	connect(graph_log_y, SIGNAL(changed()), this, SLOT(replot()));

	// Graph legend
	graph_legend_pref = new GraphLegendPreference();
	connect(graph_legend_pref, SIGNAL(changed()), this, SLOT(replot()));


	lbl_window_time = new QLabel();

	graph_opts_lay2->addWidget(new QLabel(tr("Graph legend:")));
	graph_opts_lay2->addWidget(graph_legend_pref);

	graph_opts_lay2->addWidget(graph_rescale_x);
	graph_opts_lay2->addWidget(graph_rescale_y);
	graph_opts_lay2->addWidget(graph_log_y);

	graph_opts_lay3->addWidget(new QLabel(tr("Sample interval (ms):")));
	graph_opts_lay3->addWidget(sample_interval);
	graph_opts_lay3->addWidget(lbl_window_time);


	lay->addWidget(plt);
	lay->addWidget(graph_opts2);
	lay->addWidget(graph_opts);

	plt->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	graph_opts->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	
	replot();

	sample_timer = new QTimer();
	sample_timer->setInterval(sample_interval->value());
	connect(sample_timer, SIGNAL(timeout()), this, SLOT(refreshView()));
	sample_timer->start();
	
	refreshWindowtime();
}

GraphView::~GraphView(void)
{
}

int GraphView::findGraphIndex(QCPGraph *graph)
{
	int i;

	for (i=0; i<src_graphs.size(); i++) {
		if (src_graphs.at(i) == graph)
			return i;
	}
	return -1;
}

void GraphView::editLegend(QCPLegend *legend, QCPAbstractLegendItem *item, QMouseEvent *event)
{
	int i;
	int index=-1;

	if (!item)
		return;

	// plt->legend (QCPLegend*)
	for (int i=0; i<plt->legend->itemCount(); i++) {
		if (item == plt->legend->item(i)) {
			index = i;
			break;
		}
	}

	if (index >= 0) {
		QCPGraph *gr;
		gr = plt->graph(index);
		if (gr) {
			GraphLineStyleDialog *ls_dialog = new GraphLineStyleDialog();
			ls_dialog->setCurrentSettings(gr->pen());
			ls_dialog->exec();

			if (ls_dialog->apply) {
				qDebug("Applying change");
				gr->setPen(ls_dialog->getCurrentSettings());

				int graphidx = findGraphIndex(gr);
				if (graphidx >= 0) {
					saveGraphPen(sources.at(graphidx), ls_dialog->getCurrentSettings());
				}

			} else {
				qDebug("Change cancelled");
			}

			delete ls_dialog;

			/*
			QColor color;
			color = QColorDialog::getColor(gr->pen().color());
			if (color.isValid()) {
				QPen p(color);
				p.setWidth(2);
				p.setStyle(Qt::DashLine);
				gr->setPen(p);
				replot();
			}
			*/
		}
	}
}

void GraphView::editTitle()
{
	QString new_title;
	bool ok = false;

	new_title = QInputDialog::getText(this, tr("Enter graph title"), tr("Title:"), 
									QLineEdit::Normal, plt->title() ,  &ok);

	new_title = new_title.trimmed();
	if (ok && !new_title.isEmpty()) {
		plt->setTitle(new_title);
		replot();
	}
}

void GraphView::saveGraph(void)
{
	QString filename;
	QString default_dir = QDir::homePath();
	QString sel_filt;

	filename = QFileDialog::getSaveFileName(this, tr("Save graph to file"), default_dir,
		"PNG (*.png);; JPEG (*.jpg *.jpeg);; BMP (*.bmp);; PDF (*.pdf)", &sel_filt);


//	qDebug() << "Selected filter: " + sel_filt;

	if (filename.size()) {
//		qDebug() << "filename: " << filename;

		if (sel_filt.startsWith("JPEG")) {
			QFileInfo inf(filename);
			if (!inf.suffix().toLower().startsWith("jp")) {
				filename = filename + ".jpeg";
			}
			plt->saveJpg(filename);
		}
		if (sel_filt.startsWith("PNG")) {
			QFileInfo inf(filename);
			if (inf.suffix().toLower().compare("png") != 0) {
				filename = filename + ".png";
			}

			plt->savePng(filename);
		}
		if (sel_filt.startsWith("BMP")) {
			QFileInfo inf(filename);
			if (inf.suffix().toLower().compare("bmp") != 0) {
				filename = filename + ".bmp";
			}

			plt->saveBmp(filename);
		}
		if (sel_filt.startsWith("PDF")) {
			QFileInfo inf(filename);
			if (inf.suffix().toLower().compare("pdf") != 0) {
				filename = filename + ".pdf";
			}

			plt->savePdf(filename, true);
		}
	}
}

void GraphView::resetGraph(void)
{
	plt->clearGraphs();
	src_graphs.clear();
	for (int i=0; i<sources.size(); i++) {
		src_graphs.append(NULL);
	}
	x_count = 0;
}

void GraphView::addTenkiSource(struct sourceDescription *sd)
{
	addSourceByName(sd->q_name);
}

void GraphView::removeTenkiSource(struct sourceDescription *sd)
{
	// TODO
	(void)sd;
}

void GraphView::addSourceByName(QString sname)
{
	sources.append(sname);
	src_graphs.append(NULL);
}

void GraphView::refreshView()
{
	struct sourceDescription *sd;
	struct USBTenki_channel chndata;
	QSettings settings;
	QCPGraph *gr;

	for (int i=0; i<sources.size(); i++)
	{
		if (!settings.value("graphChecked/"+sources.at(i)).toBool())
			continue;

		sd = g_tenkisources->getSourceByName(sources.at(i));
		QSettings settings;

		if (!sd) {
			return;
		}

		g_tenkisources->convertToUnits(sd->chn_data, &chndata);
		QString alias = sd->q_alias;
		QString units = QString::fromUtf8(unitToString(chndata.converted_unit, 0));
		QString name = QString::fromUtf8(sd->name);
		QString d;

		// Decide what shall be the name of the graph in the
		// legend. Use alias by default, fallback to SOURCE:ID
		QString displayname = alias;
		if (alias.length()==0) {
			displayname = name;
		}

		// Ok, now we have our value.
		// Find if there is a pre-existing graph
		if (src_graphs.at(i)) {
			gr = src_graphs.at(i);
			if (displayname.compare(gr->name()) != 0) {
				// Name changed
				gr->setName(displayname);
			}
		
		} else {
			// Create it on the fly
			gr = plt->addGraph();
			gr->setName(displayname);

			QPen p = determineGraphPen(i, name);

			gr->setPen(p);
			src_graphs.replace(i, gr);
		}

		if (chndata.status == USBTENKI_CHN_STATUS_VALID &&
			(chndata.converted_data == chndata.converted_data )	) // ! Nan
		{
				gr->addData(x_count, chndata.converted_data);
		}
		else {
//			gr->setDataKeyError(x_count);
		}
		if (x_count >= x_max) {
			gr->removeData(x_count - x_max);
			//printf("Sliding window\n");
		}

	}

	replot();
	x_count++;
}

void GraphView::saveGraphPen(QString sourcename, QPen pen)
{
	QSettings settings;
	QString key = "graphStyle/" + sourcename + "pen";

	settings.setValue(key, pen);
}

QPen GraphView::determineGraphPen(int index, QString sourcename)
{
	#define NUM_DEFAULT_COLORS	12
	QColor colors[NUM_DEFAULT_COLORS] = {
		Qt::green,
		Qt::blue,
		Qt::red,
		QColor(255,133,26), // orange
		Qt::cyan,
		Qt::magenta,
		QColor(255,200,255), // pink
		Qt::darkMagenta,
		Qt::darkRed,
		Qt::darkYellow,
		Qt::gray,
		Qt::black,
	};
	QSettings settings;
	QString key = "graphStyle/" + sourcename + "pen";

	// If a setting does not already exist, choose a color
	// from the palete above, and create the setting.
	QVariant v = settings.value(key);
	if (v.isNull()) {
		QPen pen(colors[index%NUM_DEFAULT_COLORS]);
		pen.setWidth(DEFAULT_GRAPH_LINE_WIDTH);
		settings.setValue(key, pen);
	}

	// Now load the setting created above, or a pre-existing setting.
	QPen pen = settings.value(key).value<QPen>();

	return pen;
}

void GraphView::replot(void)
{
	if (graph_legend_pref->getStyle() == QCPLegend::psManual) {
		plt->legend->setVisible(false);
	} else {
		plt->legend->setVisible(true);
		plt->legend->setPositionStyle(graph_legend_pref->getStyle());
	}

	if (graph_log_y->isChecked()) {
		plt->yAxis->setScaleType(QCPAxis::stLogarithmic);
		plt->yAxis->setScaleLogBase(10.0); // TODO configurabl
	}
	else {
		plt->yAxis->setScaleType(QCPAxis::stLinear);
	}

	Qt::Orientations orient = 0;

	if (graph_rescale_x->isChecked()) {
		for (int i=0; i<plt->graphCount(); i++) {
			plt->graph(i)->rescaleKeyAxis(i==0 ? false : true);
		}
	} else {
		orient |= Qt::Horizontal;
	}


	if (graph_rescale_y->isChecked()) {
		for (int i=0; i<plt->graphCount(); i++) {
			plt->graph(i)->rescaleValueAxis(i==0? false : true);
		}
	} else {
		orient |= Qt::Vertical;
	}

	plt->setRangeZoom(orient);
	plt->setRangeDrag(orient);


	plt->replot();
}

void GraphView::intervalChanged(int i)
{
	QSettings settings;
	settings.setValue("graph/sample_interval_ms", i);
	sample_timer->setInterval(i);
	refreshWindowtime();

	QString lbl;
	lbl.sprintf("Samples at ");
	
	if (sample_timer->interval()>=1000) {
		QString duration;
		fancyTime(&duration, sample_timer->interval());
		lbl += duration + " intervals (";
		QString ms_interval;
		ms_interval.sprintf("%d", sample_timer->interval());

		lbl += ms_interval + "ms)";
	}
	else {
		QString ms_interval;
		ms_interval.sprintf("%d", sample_timer->interval());
		lbl += ms_interval + "ms intervals";
	}

	plt->xAxis->setLabel(lbl);

	replot();
}

void GraphView::pause_unpause(void)
{
	is_paused = !is_paused;

	if (is_paused) {
		sample_timer->stop();
		btn_pause_continue->setText(tr("Unpause"));
	} else {
		sample_timer->start();
		btn_pause_continue->setText(tr("Pause"));
	}
}

void GraphView::fancyTime(QString *s, unsigned long long ms)
{
	if (ms < 1000) {
		s->sprintf("%lld ms", ms);	
	}
	else if (ms < 60 * 1000) { // less than 1 minute
		s->sprintf("%.1f s", ms / 1000.0);	
	}
	else if (ms < 60*60*1000) { // les than 1 hour
		s->sprintf("%.1f m", ms / 1000.0 / 60.0);	
	}
	else if (ms < 60*60*1000*24) { // les than 1 day
		s->sprintf("%.1f h", ms / 1000.0 / 60.0 / 60.0);
	}
	else {
		s->sprintf("%.1f d", ms / 1000.0 / 60.0 / 60.0 / 30.4363);
	}
}

void GraphView::refreshWindowtime(void)
{
	QString res;
	QString duration;
	unsigned long long cap_ms;

	cap_ms = sample_timer->interval() * (unsigned long long)x_max;
	
	fancyTime(&duration, cap_ms);

	res = "Buffer capacity: ";
	res += duration;

	lbl_window_time->setText(res);
}

