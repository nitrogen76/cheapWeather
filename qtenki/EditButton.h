#ifndef _editbutton_h__
#define _editbutton_h__

#include <QPushButton>

class EditButton : public QPushButton
{
	Q_OBJECT

	public:
		EditButton(const QString &text, int id);
		~EditButton();

	signals:
		void buttonIdClicked(int id);

	public slots:
		void was_clicked();

	private:
		int buttonid;
};

#endif // _editbutton_h__

