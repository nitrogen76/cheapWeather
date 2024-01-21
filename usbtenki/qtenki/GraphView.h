#ifndef _graphview_h__
#define _graphview_h__

#include <QWidget>
#include <QTimer>
#include <QPushButton>
#include <QTimer>
#include <QList>
#include <QVBoxLayout>
#include <QLabel>

#include "ConfigCheckbox.h"
#include "TenkiSources.h"
#include "BigLabel.h"
#include "qcustomplot.h"
#include "GraphLegendPreference.h"

class GraphView : public QWidget, public TenkiSourceAddRemove
{
	Q_OBJECT

	public:
		GraphView();
		~GraphView(void);

		virtual void addTenkiSource(struct sourceDescription *sd);
		virtual void removeTenkiSource(struct sourceDescription *sd);

	public slots:
		void refreshView(void);
		void addSourceByName(QString src_name);
		void resetGraph(void);
		void saveGraph(void);
		void replot(void);
		void editTitle(void);
		void editLegend(QCPLegend *legend, QCPAbstractLegendItem *item, QMouseEvent *event);
	
	private slots:
		void intervalChanged(int i);
		void pause_unpause(void);

	private:
		int findGraphIndex(QCPGraph *graph);
		void saveGraphPen(QString sourcename, QPen pen);
		QPen determineGraphPen(int index, QString sourcename);
		void fancyTime(QString *s, unsigned long long ms);
		void refreshWindowtime(void);
		QTimer *sample_timer;
		QVBoxLayout *lay;
		QPushButton *btn_pause_continue;
		QList<QString> sources;
		QList<QCPGraph*> src_graphs;
		ConfigCheckbox *graph_rescale_x;
		ConfigCheckbox *graph_rescale_y;
		ConfigCheckbox *graph_log_y;
		QCustomPlot *plt;
		GraphLegendPreference *graph_legend_pref;
		QLabel *lbl_window_time;
		int x_count;
		int x_max;
		int is_paused;
};

#endif 

