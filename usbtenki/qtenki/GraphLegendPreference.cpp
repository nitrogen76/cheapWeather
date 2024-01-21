#include <QSettings>
#include "GraphLegendPreference.h"
#include "usbtenki_units.h"
#include "globals.h"

GraphLegendPreference::GraphLegendPreference()
{
	QSettings settings;

	config_key = "graph/legend_position";

	addItem(tr("None"));
	addItem(tr("Top left"));
	addItem(tr("Top"));
	addItem(tr("Top right"));
	addItem(tr("Right"));
	addItem(tr("Bottom right"));
	addItem(tr("Bottom"));
	addItem(tr("Bottom left"));
	addItem(tr("Left"));

	setCurrentIndex(settings.value(config_key, 0).toInt());


	connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(idx_changed(int)));
}

QCPLegend::PositionStyle GraphLegendPreference::getStyle(void)
{
	switch(currentIndex()) {
		default:
		case 0: return QCPLegend::psManual;
		case 1: return QCPLegend::psTopLeft;
		case 2: return QCPLegend::psTop;
		case 3: return QCPLegend::psTopRight;
		case 4: return QCPLegend::psRight;
		case 5: return QCPLegend::psBottomRight;
		case 6: return QCPLegend::psBottom;
		case 7: return QCPLegend::psBottomLeft;
		case 8: return QCPLegend::psLeft;
	}
}

void GraphLegendPreference::idx_changed(int idx)
{
	QSettings settings;

	settings.setValue(config_key, idx);
	emit changed();
}
