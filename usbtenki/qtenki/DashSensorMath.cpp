#include <stdint.h>
#include <QDebug>
#include <QPushButton>
#include <QSettings>
#include <QLineEdit>
#include <QMessageBox>
#include "DashSensorMath.h"
#include "TenkiDevice.h"
#include "SourceAliasEdit.h"
#include "ConfigCheckbox.h"
#include "globals.h"
#include "usbtenki_cmds.h"
#include "EditButton.h"
#include "HTMLViewer.h"

// #define RESET_MIN_MAX_ON_EQUATION_EDIT

DashSensorMath::DashSensorMath(TenkiMathDevice *td)
{
	int col = 0, i;
	title = QString::fromLocal8Bit(td->getSerial());
	setTitle(title);
	setObjectName("source"); // selector for stylesheet	
	tenki_device = td;


	layout = new QGridLayout();

	layout->setVerticalSpacing(1);
	layout->setHorizontalSpacing(10);

//	layout->addWidget(new QLabel("<b>Channel</b>"), 0, col++);
	layout->addWidget(new QLabel("<b>Source ID</b>"), 0, col++);
	layout->setColumnMinimumWidth(col, 4);

	layout->setColumnStretch(col, 2);
	layout->addWidget(new QLabel("<b>Equation</b>"), 0, col++);

	// Leave a blank column for the edit button
	col++;

	layout->addWidget(new QLabel("<b>Result</b>"), 0, col++);
	layout->setColumnMinimumWidth(col, 4);

	layout->addWidget(new QLabel("<b>Min.</b>"), 0, col++);
	layout->setColumnMinimumWidth(col, 4);

	layout->addWidget(new QLabel("<b>Max.</b>"), 0, col++);
	layout->setColumnMinimumWidth(col, 4);

	layout->addWidget(new QLabel("<b>Unit</b>"), 0, col++);
	layout->setColumnMinimumWidth(col, 4);

	layout->setColumnStretch(col, 1);
	layout->addWidget(new QLabel("<b>Alias</b>"), 0, col++);

	layout->addWidget(new QLabel("<b>Big View</b>"), 0, col++);
	layout->setColumnStretch(col, 0);

	layout->addWidget(new QLabel("<b>Graph</b>"), 0, col++);
	layout->setColumnStretch(col, 0);

	layout->addWidget(new QLabel("<b></b>"), 0, col++);
	layout->setColumnMinimumWidth(col, 4);

	for (i=0; i<tenki_device->getNumChannels(); i++)
	{
		if (tenki_device->isChannelHidden(i)) {
			continue;
		}

		addChannel(i, i+1);
	}


	helpbtn = new QPushButton(QIcon(":help-about.png"), tr("Help"));
	layout->addWidget(helpbtn, i + 1, 0);
	connect(helpbtn, SIGNAL(clicked(bool)), this, SLOT(helpClicked(bool)));


	setLayout(layout);
}

void DashSensorMath::addChannel(int chn, int row)
{
	QSettings settings;
	USBTenki_channel ch;
	QString a, b, c, d, e, f, g;
	QLabel *value_label, *unit_label;
	//QLabel *tmp_label;
	QLineEdit *eqedit;
	QPushButton *rst;
	QPushButton *editbtn;
	MinMaxResettable *min, *max;
	int col=0;

	g_tenkisources->convertToUnits(tenki_device->getChannelData(chn), &ch);

/*	
	a.sprintf("%d", chn);
	layout->addWidget(new QLabel(a), row, col++);*/

	f.sprintf("%s:%02X", tenki_device->getSerial(), chn);
	layout->addWidget(new QLabel(f), row, col++);

	// Equation editor
	b = tenki_device->getChannelEquation(chn);
	eqedit = new QLineEdit(b);
	equations.append(eqedit);
	chip_ids.append(ch.chip_id);
	layout->addWidget(eqedit, row, col++);
	connect(eqedit, SIGNAL(editingFinished()), this, SLOT(equationEdited()));

	// Edit button
	editbtn = new EditButton("Edit", chn);
	editbtn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	layout->addWidget(editbtn, row, col++);
	connect(editbtn, SIGNAL(buttonIdClicked(int)), this, SLOT(editClicked(int)));

	// Current value
	g_tenkisources->formatValue(&d, ch.converted_data);
	value_label = new QLabel(d);
	values.append(value_label);
	layout->addWidget(value_label, row, col++);

	// Minimum value
	min = new MinMaxResettable(true); // Minimum tracking mode
	minimums.append(min);
	layout->addWidget(min, row, col++);

	// Maximum value
	max = new MinMaxResettable(false); // Maximum tracking mode
	maximums.append(max);
	layout->addWidget(max, row, col++);


	e = QString::fromUtf8(unitToString(ch.converted_unit, 0));
	unit_label = new QLabel(e);
	layout->addWidget(unit_label, row, col++);
	units.append(unit_label);

	// alias
	SourceAliasEdit *se = new SourceAliasEdit(f);
	layout->addWidget(se, row, col++);
	connect(se, SIGNAL(sourceAliasChanged(QString,QString)), g_tenkisources, SLOT(updateAlias(QString,QString)));

	// In bigview
	ConfigCheckbox *ccb = new ConfigCheckbox("", "bigviewChecked/" + f);
	layout->addWidget(ccb, row, col++);

	// In graph
	ConfigCheckbox *ccb_gr = new ConfigCheckbox("", "graphChecked/" + f);
	layout->addWidget(ccb_gr, row, col++);

	// Min/Max reset button
	rst = new QPushButton("Reset min./max.");
	rst->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	layout->addWidget(rst, row, col++);
	QObject::connect(rst, SIGNAL(clicked()), min, SLOT(reset()));
	QObject::connect(rst, SIGNAL(clicked()), max, SLOT(reset()));

	channel_id.append(chn);
}

DashSensorMath::~DashSensorMath()
{
	QLayoutItem *child;
	while ((child = layout->takeAt(0)) != 0) {
		delete child;
	}
	delete layout;
}

void DashSensorMath::refresh()
{
	USBTenki_channel *cdat;

	if (tenki_device->getStatus() == TENKI_DEVICE_STATUS_UNABLE_TO_OPEN) {
		title = QString::fromLocal8Bit(tenki_device->getSerial());
		title += " [ERROR]";
	} else {
		title = QString::fromLocal8Bit(tenki_device->getSerial());
	}
	setTitle(title);

	for (int i=0; i<values.size(); i++) {
		QString d,e;

		cdat = tenki_device->getChannelData(channel_id.at(i));
		if (!cdat) {
			values.at(i)->setText("Error");
			continue;
		}
		if (cdat->status != USBTENKI_CHN_STATUS_VALID) {
			values.at(i)->setText(usbtenki_getChannelStatusString(cdat));
			continue;
		}

		g_tenkisources->formatValue(&d, cdat->converted_data);

		values.at(i)->setText(d);
		minimums.at(i)->submitValue(cdat->converted_data);
		maximums.at(i)->submitValue(cdat->converted_data);

		// those two QList are populated in the same
		// order. So it will be the same index i.
		e = QString::fromUtf8(unitToString(cdat->converted_unit, 0));
		units.at(i)->setText(e);

	//	qDebug() << d;
	}

}

// Called when one of the equations in this DashSensorMath object has potentially been edited
void DashSensorMath::equationEdited()
{
	for (int i=0; i<equations.size(); i++) {
		QString cur_eq = equations.at(i)->text();

		if (0 == QString::compare(cur_eq, g_mathDevice->getChannelEquation(i))) {
			continue;
		}

//		qDebug() << "Changed: " << cur_eq;

		g_mathDevice->setChannelEquation(i, cur_eq);
#ifdef RESET_MIN_MAX_ON_EQUATION_EDIT
		maximums.at(i)->reset();
		minimums.at(i)->reset();
#endif
	}
}


void DashSensorMath::helpClicked(bool checked)
{
	HTMLViewer *viewer;
	(void)checked;

	viewer = new HTMLViewer(":mathhelp.html", "Help");
	viewer->exec();

	delete viewer;
}

void DashSensorMath::editClicked(int id)
{
	editDialog = new MathEditDialog(this, id);
	editDialog->exec();

	delete editDialog;
	editDialog = NULL;

#ifdef RESET_MIN_MAX_ON_EQUATION_EDIT
	maximums.at(id)->reset();
	minimums.at(id)->reset();
#endif

	equations.at(id)->setText(g_mathDevice->getChannelEquation(id));
}
