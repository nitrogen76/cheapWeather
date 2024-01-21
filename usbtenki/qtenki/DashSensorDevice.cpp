#include <stdint.h>
#include <QDebug>
#include <QSettings>
#include <QMessageBox>
#include <QLineEdit>
#include "DashSensorDevice.h"
#include "TenkiDevice.h"
#include "SourceAliasEdit.h"
#include "ConfigCheckbox.h"
#include "globals.h"
#include "usbtenki_cmds.h"

DashSensorDevice::DashSensorDevice(TenkiDevice *td)
{
	int col=0, i;

	title = QString::fromLocal8Bit(td->getSerial());
	setTitle(title);
	setObjectName("source"); // selector for stylesheet
	tenki_device = td;

	layout = new QGridLayout();
	setLayout(layout);

	layout->setVerticalSpacing(1);
	layout->setHorizontalSpacing(10);

//	layout->addWidget(new QLabel("<b>Channel</b>"), 0, col++);
	layout->addWidget(new QLabel("<b>Source ID</b>"), 0, col++);
	layout->setColumnMinimumWidth(col, 4);
	layout->addWidget(new QLabel("<b>Description</b>"), 0, col++);
	layout->setColumnMinimumWidth(col, 4);
	layout->addWidget(new QLabel("<b>Type</b>"), 0, col++);
	layout->setColumnMinimumWidth(col, 4);
	layout->addWidget(new QLabel("<b>Current</b>"), 0, col++);
	layout->setColumnMinimumWidth(col, 4);
	layout->addWidget(new QLabel("<b>Min.</b>"), 0, col++);
	layout->setColumnMinimumWidth(col, 4);
	layout->addWidget(new QLabel("<b>Max.</b>"), 0, col++);
	layout->setColumnMinimumWidth(col, 4);
	layout->addWidget(new QLabel("<b>Unit</b>"), 0, col++);
	layout->setColumnMinimumWidth(col, 4);
	layout->addWidget(new QLabel("<b>Alias</b>"), 0, col++);
	layout->setColumnStretch(col, 0);
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

	infobtn = new QPushButton(QIcon(":help-about.png"), tr("About"));
	layout->addWidget(infobtn, i + 1, 0);
	connect(infobtn, SIGNAL(clicked(bool)), this, SLOT(infoClicked(bool)));


	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
}

void DashSensorDevice::addChannel(int chn, int row)
{
	QSettings settings;
	USBTenki_channel ch;
	QString a, b, c, d, e, f, g;
	QLabel *value_label, *unit_label;
	QLabel *tmp_label;
	MinMaxResettable *min, *max;
	QPushButton *rst;
	int col=0;

	g_tenkisources->convertToUnits(tenki_device->getChannelData(chn), &ch);

/*	
	a.sprintf("%d", chn);
	layout->addWidget(new QLabel(a), row, col++);*/

	f.sprintf("%s:%02X", tenki_device->getSerial(), chn);
	layout->addWidget(new QLabel(f), row, col++);

	b = QString::fromLocal8Bit(chipToString(ch.chip_id));

	tmp_label = new QLabel(b);
	descriptions.append(tmp_label);

	chip_ids.append(ch.chip_id);

//	tmp_label->setWordWrap(true);
//	tmp_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	layout->addWidget(tmp_label, row, col++);

	c = QString::fromLocal8Bit(chipToShortString(ch.chip_id));
	layout->addWidget(new QLabel(c), row, col++);

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

	prev_iec = -1;
	recolorizeThermocouple();
}

DashSensorDevice::~DashSensorDevice()
{
	QLayoutItem *child;
	while ((child = layout->takeAt(0)) != 0) {
		delete child;
	}
	delete layout;
}

void DashSensorDevice::recolorizeThermocouple(void)
{
	QString b;
	int use_iec;

	use_iec = g_tenkisources->getUseIECthermocoupleColors();

	if (use_iec == prev_iec) {
		return; // nothing to do
	}
	prev_iec = use_iec;

	for (int i=0; i<chip_ids.size(); i++)
	{
		b = QString::fromLocal8Bit(chipToString(chip_ids.at(i)));
		if (b.startsWith("Type-")) {

			if (use_iec) {
				b.replace("Type-K", "<font style='background-color: #40ab45; color: white;'><b>&nbsp;Type-K&nbsp;</b></font>");
				b.replace("Type-J", "<font style='background-color: #000000; color: white;'><b>&nbsp;Type-J&nbsp;</b></font>");
				b.replace("Type-N", "<font style='background-color: #fac9e1; color: black;'><b>&nbsp;Type-N&nbsp;</b></font>");
				b.replace("Type-E", "<font style='background-color: #6a3f92; color: white;'><b>&nbsp;Type-E&nbsp;</b></font>");
				b.replace("Type-T", "<font style='background-color: #794205; color: white;'><b>&nbsp;Type-T&nbsp;</b></font>");
				b.replace("Type-B", "<font style='background-color: #808080; color: white;'><b>&nbsp;Type-B&nbsp;</b></font>");
				b.replace("Type-S", "<font style='background-color: #e38538; color: white;'><b>&nbsp;Type-S&nbsp;</b></font>");
				b.replace("Type-R", "<font style='background-color: #e38538; color: white;'><b>&nbsp;Type-R&nbsp;</b></font>");
			} else {
				b.replace("Type-K", "<font style='background-color: #ffff00; color: black;'><b>&nbsp;Type-K&nbsp;</b></font>");
				b.replace("Type-J", "<font style='background-color: #000000; color: white;'><b>&nbsp;Type-J&nbsp;</b></font>");
				b.replace("Type-N", "<font style='background-color: #e2811f; color: white;'><b>&nbsp;Type-N&nbsp;</b></font>");
				b.replace("Type-E", "<font style='background-color: #d9018b; color: white;'><b>&nbsp;Type-E&nbsp;</b></font>");
				b.replace("Type-T", "<font style='background-color: #2350a0; color: white;'><b>&nbsp;Type-T&nbsp;</b></font>");
				b.replace("Type-B", "<font style='background-color: #808080; color: white;'><b>&nbsp;Type-B&nbsp;</b></font>");
				b.replace("Type-S", "<font style='background-color: #3f8d3e; color: white;'><b>&nbsp;Type-S&nbsp;</b></font>");
				b.replace("Type-R", "<font style='background-color: #3f8d3e; color: white;'><b>&nbsp;Type-R&nbsp;</b></font>");
			}
		}

		descriptions.at(i)->setText(b);
	}
}

void DashSensorDevice::refresh()
{
	USBTenki_channel ch, *cdat;
//	qDebug() << "DashSensorDevice::refresh()";

	if (tenki_device->getStatus() == TENKI_DEVICE_STATUS_UNABLE_TO_OPEN) {
		title = QString::fromLocal8Bit(tenki_device->getSerial());
		title += " [ERROR]";
	} else {
		title = QString::fromLocal8Bit(tenki_device->getSerial());
	}
	setTitle(title);

	for (int i=0; i<values.size(); i++) {
		QString d,e;

		// react to setting changes
		recolorizeThermocouple();

		cdat = tenki_device->getChannelData(channel_id.at(i));
		if (!cdat) {
			values.at(i)->setText("Error");
			continue;
		}
		if (cdat->status != USBTENKI_CHN_STATUS_VALID) {
			values.at(i)->setText(usbtenki_getChannelStatusString(cdat));
			continue;
		}

		g_tenkisources->convertToUnits(cdat, &ch);

		g_tenkisources->formatValue(&d, ch.converted_data);
		if (chip_ids.at(i) == USBTENKI_CHIP_HEXCOLOR) {
			char tmpbuf[128];
			snprintf(tmpbuf, 128, "<font style='background-color: #%06x'; color: #%06x'><b>#%06x</b></font>",
				(uint32_t)ch.converted_data,
				0x000000,
				(uint32_t)ch.converted_data
				);
			values.at(i)->setText(tmpbuf);
		} else {
			values.at(i)->setText(d);
		}
		minimums.at(i)->submitValue(ch.converted_data);
		maximums.at(i)->submitValue(ch.converted_data);

		// those two QList are populated in the same
		// order. So it will be the same index i.
		e = QString::fromUtf8(unitToString(ch.converted_unit, 0));
		units.at(i)->setText(e);

	//	qDebug() << d;
	}

}

void DashSensorDevice::infoClicked(bool checked)
{
	QMessageBox msgBox;
	int major, minor;

	(void)checked;

	tenki_device->getFirmwareVersion(&major, &minor);

	QString text =	"<html><body><h1>" + QString(tenki_device->getProductName()) + "</h1>" +
					"Firmware version: " + QString::number(major) + "." + QString::number(minor) + "<br>" +
					"Serial number: " + QString(tenki_device->getSerial()) + "<br>" +
					"Channels: " + QString::number(tenki_device->getVisibleChannels()) +

					"</body></html>";

	msgBox.setWindowTitle("About sensor " + title );
	msgBox.setText(text);
	msgBox.setStandardButtons(QMessageBox::Ok);
	msgBox.setIcon(QMessageBox::Information);

	msgBox.exec();
}
