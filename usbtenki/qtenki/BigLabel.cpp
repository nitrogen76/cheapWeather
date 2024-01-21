#include <iostream>
#include "BigLabel.h"
#include "globals.h"
#include "usbtenki_cmds.h"

#include <QSettings>
#include <QFont>
#include <QLabel>
#include <QWidget>
#include <QResizeEvent>

BigLabel::BigLabel(const QString &text, QString source_name)
{
	setText(text);
	setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	src_name = source_name;
	refresh();
}

void BigLabel::refresh()
{
	QSettings settings;
	QString final_text;
	struct USBTenki_channel chndata;
	struct sourceDescription *sd = g_tenkisources->getSourceByName(src_name);

	if (!sd) {
		setText("err");
		return;
	}

	QString alias = sd->q_alias;

	if (sd->chn_data->status != USBTENKI_CHN_STATUS_VALID) {

		if (settings.value("bigview/show_aliases").toBool()) {
			final_text += alias;
			final_text += ": ";
		}

		final_text += usbtenki_getChannelStatusString(sd->chn_data);
	}
	else
	{
		if (sd->chn_data->chip_id == USBTENKI_CHIP_MATH) {
			memcpy(&chndata, sd->chn_data, sizeof(USBTenki_channel));
		} else {
			g_tenkisources->convertToUnits(sd->chn_data, &chndata);
		}

		QString units = QString::fromUtf8(unitToString(chndata.converted_unit, 0));

		QString d;

		g_tenkisources->formatValue(&d, chndata.converted_data);


		if (settings.value("bigview/show_aliases").toBool()) {
			final_text += alias;
			final_text += ": ";
		}

		final_text += d;

		if (settings.value("bigview/show_units").toBool()) {
			final_text += " ";
			final_text += units;
		}

		if (sd->td->getStatus() != TENKI_DEVICE_STATUS_OK) {
			final_text += " (error)";
		}
	}

	setText(final_text);
	fitFont(rect()); // fix the font size
}

void BigLabel::fitFont(QRect rect)
{
	int flags = Qt::TextDontClip;
	QFont f = font();

	// Step 1: Try to set the requested height. If everything fits horizontally,
	// all labels will have equal height.
	f = font();
	float orig = rect.size().height() * 0.8;
	f.setPixelSize(orig);

	// Step 2: Long strings may be cut. If this happens,
	// reduce the font until it fits in the target width.
	QRect fontBoundRect = QFontMetrics(f).boundingRect(rect,flags, text());

//	qDebug() << "font width: " << fontBoundRect.width() << " space:" << resize.width() << " Text: " << text();

	while (fontBoundRect.width() > rect.size().width() && orig > 0 ) {

		f.setPixelSize(orig);
		orig -= (orig*0.1); // 10% decrements. Every resize then requires approx. 10 iterations

//		qDebug() << "Testing size: " << orig;

		fontBoundRect = QFontMetrics(f).boundingRect(rect,flags, text());
	}

	setFont(f);
}

void BigLabel::resizeEvent(QResizeEvent *event)
{
	QRect resize(0,0,event->size().width(), event->size().height());
	fitFont(resize);
#if 0

	int flags = Qt::TextDontClip;
	QRect resize(0,0,event->size().width(), event->size().height());
	QFont f = font();

	// Step 1: Try to set the requested height. If everything fits horizontally,
	// all labels will have equal height.
	f = font();
	float orig = event->size().height() * 0.8;
	f.setPixelSize(orig);

	// Step 2: Long strings may be cut. If this happens,
	// reduce the font until it fits in the target width.
	QRect fontBoundRect = QFontMetrics(f).boundingRect(resize,flags, text());

//	qDebug() << "font width: " << fontBoundRect.width() << " space:" << resize.width() << " Text: " << text();

	while (fontBoundRect.width() > resize.width() && orig > 0 ) {

		f.setPixelSize(orig);
		orig -= (orig*0.1); // 10% decrements. Every resize then requires approx. 10 iterations

//		qDebug() << "Testing size: " << orig;

		fontBoundRect = QFontMetrics(f).boundingRect(resize,flags, text());
	}

	setFont(f);
#endif
	QFrame::resizeEvent(event);
}
