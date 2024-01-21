#include "GraphLineStyleDialog.h"
#include <QColorDialog>

GraphLineStyleDialog::GraphLineStyleDialog(QWidget *parent) : QDialog(parent)
{
	apply = 0;

	//
	// +--------------------------------+
	// | Color: XXXX    Change...       |
	// |                                |
	// | Line width:                    |
	// | [ spinbox ]                    |
	// | Line style:                    |
	// | [ drop down ]                  |
	// |                                |
	// |                      OK CANCEL |
	// +--------------------------------+
	//
	//
	//

	lbl_color = new QLabel(tr("Color:"));
	btn_change_color = new QPushButton(tr("Change..."));
	lbl_cur_color = new QLabel();
	connect(btn_change_color, SIGNAL(clicked()), this, SLOT(changeColor()));
	lay_coloroptions = new QHBoxLayout();
	lay_coloroptions->addWidget(lbl_color);
	lay_coloroptions->addWidget(lbl_cur_color);
	lay_coloroptions->addWidget(btn_change_color);


	lbl_ls = new QLabel(tr("Line style:"));
	ddStyle = new LineStyleDropDown();
	connect(ddStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(lineStyleChanged(int)));

	lbl_lw = new QLabel(tr("Line width:"));
	qs_lw = new QSpinBox();
	qs_lw->setRange(1,5);
	qs_lw->setSingleStep(1);
	connect(qs_lw, SIGNAL(valueChanged(int)), this, SLOT(lineWidthChanged(int)));

	btn_ok = new QPushButton(tr("Ok"));
	btn_ok->setDefault(true);
	btn_cancel = new QPushButton(tr("Cancel"));
	connect(btn_ok, SIGNAL(clicked()), this, SLOT(applyClose()));
	connect(btn_cancel, SIGNAL(clicked()), this, SLOT(close()));
	lay_btns = new QHBoxLayout();
	lay_btns->addStretch();
	lay_btns->addWidget(btn_ok);
	lay_btns->addWidget(btn_cancel);


	lay_vert = new QVBoxLayout();
	lay_vert->addLayout(lay_coloroptions);
	lay_vert->addWidget(lbl_lw);
	lay_vert->addWidget(qs_lw);
	lay_vert->addWidget(lbl_ls);
	lay_vert->addWidget(ddStyle);
	lay_vert->addStretch();
	lay_vert->addLayout(lay_btns);

	setLayout(lay_vert);
	setWindowTitle(tr("Edit graph line style"));
}

GraphLineStyleDialog::~GraphLineStyleDialog()
{
	delete lay_btns;
	delete lay_coloroptions;
	delete lay_vert;
	delete lbl_color;
	delete lbl_ls;
	delete lbl_cur_color;
	delete lbl_lw;
	delete btn_change_color;
	delete btn_ok;
	delete btn_cancel;
	delete qs_lw;
	delete ddStyle;
}

void GraphLineStyleDialog::setCurrentSettings(QPen pen)
{
	cur_pen = pen;
	QPixmap pm(16,16);
	pm.fill(cur_pen.color());
	lbl_cur_color->setPixmap(pm);
	ddStyle->selectStyle(cur_pen.style());
	qs_lw->setValue(cur_pen.width());
}

QPen GraphLineStyleDialog::getCurrentSettings()
{
	return cur_pen;
}

void GraphLineStyleDialog::lineWidthChanged(int w)
{
	cur_pen.setWidth(w);
}

void GraphLineStyleDialog::lineStyleChanged(int idx)
{
	(void)idx;
	cur_pen.setStyle(ddStyle->getSelectedStyle());
}

void GraphLineStyleDialog::changeColor()
{
	QColor color;
	color = QColorDialog::getColor(cur_pen.color());
	if (color.isValid()) {
		cur_pen.setColor(color);
		setCurrentSettings(cur_pen);
	}
}

void GraphLineStyleDialog::applyClose()
{
	apply = 1;
	close();
}

void GraphLineStyleDialog::close()
{
	QDialog::close();
}
