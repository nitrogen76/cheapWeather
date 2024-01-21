#ifndef _biglabel_h__
#define _biglabel_h__

#include <QLabel>
#include <QRect>

class BigLabel : public QLabel
{
	Q_OBJECT

	public:
		BigLabel(const QString &text, QString source_name);

		void resizeEvent(QResizeEvent *event);
		void refresh();
		QString src_name;

	private:
		void fitFont(QRect rect);
};

#endif // _biglabel_h__


