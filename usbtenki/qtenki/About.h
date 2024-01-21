#ifndef _about_h__
#define _about_h__

#include <QWidget>
#include <QVBoxLayout>

class About : public QWidget
{
	Q_OBJECT

	public:
		About();
		~About();

	private:
		QVBoxLayout *lay;
};


#endif // _about_h__

