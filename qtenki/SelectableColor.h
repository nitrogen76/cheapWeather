#ifndef Selectable_color_h__
#define Selectable_color_h__

#include <QWidget>
#include <QString>
#include <QColor>
#include <QCheckBox>

class SelectableColor : public QWidget
{
	Q_OBJECT

	public:
		SelectableColor(QString name, QString caption, QColor default_color);
		~SelectableColor();
		QColor getColor();
		bool getSelected();
		QString getName();

	private slots:
		void checkChanged(int state);
		void doDialog();

	signals:
		void colorChanged(QString name, QColor value);
		void selectedChanged(QString name, int state, QColor col);	
		
	private:
		QCheckBox *cb;
		QString name;
		QColor current_color;
};
#endif // SelectableColor
