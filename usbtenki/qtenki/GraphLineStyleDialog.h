#ifndef _graph_line_style_dialog_h__
#define _graph_line_style_dialog_h__

#include <QDialog>
#include <QPen>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpinBox>

#include "LineStyleDropDown.h"

class GraphLineStyleDialog : public QDialog
{
	Q_OBJECT

	public:
		GraphLineStyleDialog(QWidget *parent = 0);
		~GraphLineStyleDialog();

		void setCurrentSettings(QPen pen);
		QPen getCurrentSettings();

		int apply;

	private slots:
		void lineStyleChanged(int idx);
		void lineWidthChanged(int w);
		void applyClose(void);
		void close(void);
		void changeColor(void);

	private:
		QHBoxLayout *lay_btns, *lay_coloroptions;
		QVBoxLayout *lay_vert;

		QPen cur_pen;
		QLabel *lbl_color, *lbl_ls, *lbl_cur_color, *lbl_lw;
		QPushButton *btn_change_color, *btn_ok, *btn_cancel;
		QSpinBox *qs_lw;
		LineStyleDropDown *ddStyle;

};

#endif

