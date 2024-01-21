#ifndef _math_edit_dialog_h__
#define _math_edit_dialog_h__

#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>

#include "UnitsDropDown.h"

class MathEditDialog : public QDialog
{
	Q_OBJECT

	public:
		MathEditDialog(QWidget *parent = 0, int channel_id = 0);
		~MathEditDialog();

	private slots:
		void applyClose(void);
		void close(void);

	private:
		QLabel *lbl_equation, *lbl_units;
		QLineEdit *le_equation;
		QPushButton *btn_ok, *btn_cancel;
		UnitsDropDown *ddUnits;
		int chnid;

};

#endif // _math_edit_dialog_h__

