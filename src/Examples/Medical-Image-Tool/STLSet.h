#ifndef STLSET_H
#define STLSET_H

#include <QWidget>
#include "ui_STLSet.h"

namespace Ui{
	class STLSet;
};

class STLSet : public QWidget
{
	Q_OBJECT

public:
	STLSet(QWidget *parent = 0);
	~STLSet();

	Ui::STLSet *ui;
private:
//	Ui::STLSet *ui;
};

#endif // STLSET_H
