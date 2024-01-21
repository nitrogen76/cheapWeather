#include "MathEditDialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include "globals.h"

MathEditDialog::MathEditDialog(QWidget *parent, int channel_id) : QDialog(parent)
{
	chnid = channel_id;

	lbl_equation = new QLabel(tr("Equation:"));
	le_equation = new QLineEdit(g_mathDevice->getChannelEquation(chnid));
	lbl_equation->setBuddy(le_equation);

	lbl_units = new QLabel(tr("Units:"));


	btn_ok = new QPushButton(tr("Ok"));
	btn_ok->setDefault(true);

	btn_cancel = new QPushButton(tr("Cancel"));

	connect(btn_ok, SIGNAL(clicked()), this, SLOT(applyClose()));
	connect(btn_cancel, SIGNAL(clicked()), this, SLOT(close()));

	ddUnits = new UnitsDropDown();
	ddUnits->selectUnits(g_mathDevice->getChannelUnits(chnid));

	//
	// +-----------------------------------+
	// | Equation:                         |
	// | [_____________________________]   |
	// | Units:                            |
	// | [ drop down ]                     |
	// |                         OK CANCEL |
	// +-----------------------------------|
	//


	QHBoxLayout *lay_btns = new QHBoxLayout();
	lay_btns->addStretch();
	lay_btns->addWidget(btn_ok);
	lay_btns->addWidget(btn_cancel);

	QVBoxLayout *lay_vert = new QVBoxLayout();
	lay_vert->addWidget(lbl_equation);
	lay_vert->addWidget(le_equation);
	lay_vert->addWidget(lbl_units);
	lay_vert->addWidget(ddUnits);
	lay_vert->addStretch();
	lay_vert->addLayout(lay_btns);

	setLayout(lay_vert);

	setWindowTitle(tr("Edit math source"));

	//qDebug() << "Created dialog for Channel id " << chnid;
}

MathEditDialog::~MathEditDialog()
{
	//qDebug() << "Destroying dialog for Channel id " << chnid;
}

void MathEditDialog::applyClose()
{
	QString str;

	str = le_equation->text();

	g_mathDevice->setChannelEquation(chnid, str);
	g_mathDevice->setChannelUnits(chnid, ddUnits->getSelectedUnits());

	// TODO : Update the math device
	close();
}

void MathEditDialog::close()
{
	QDialog::close();
}
