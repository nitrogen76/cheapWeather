#ifndef _bigview_h__
#define _bigview_h__

#include <QWidget>
#include <QPushButton>
#include <QList>
#include <QVBoxLayout>

#include "TenkiSources.h"
#include "BigLabel.h"

class BigView : public QWidget, public TenkiSourceAddRemove
{
	Q_OBJECT

	public:
		BigView();
		~BigView(void);

		virtual void addTenkiSource(struct sourceDescription *sd);
		virtual void removeTenkiSource(struct sourceDescription *sd);

	public slots:
		void refreshView(void);
		void addSourceByName(QString src_name);
	
	private:
		QPushButton *addBtn;
		QVBoxLayout *lay;
		QList<BigLabel*> labels;
};

#endif 

