#ifndef _min_max_resettable_h__
#define _min_max_resettable_h__

#include <QLabel>
#include <QString>

class MinMaxResettable : public QLabel
{
	Q_OBJECT

	public:
		MinMaxResettable(bool minimum_mode);
		~MinMaxResettable();

		void submitValue(float value);

	public slots:
		void reset(void);

	private:
		bool empty, minimum_mode;
		float cur_min, cur_max;
};

#endif // _min_max_resettable_h__
