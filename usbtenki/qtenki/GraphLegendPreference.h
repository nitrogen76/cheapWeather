#ifndef _graph_legend_preference_h__
#define _graph_legend_preference_h__

#include <QComboBox>
#include "qcustomplot.h"

class GraphLegendPreference : public QComboBox
{
	Q_OBJECT

	public:
		GraphLegendPreference();

		QCPLegend::PositionStyle getStyle(void);

	signals:
		void changed(void);

	private slots:
		void idx_changed(int idx);

	private:
		QString config_key;
};



#endif

