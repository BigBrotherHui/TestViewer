#ifndef DISTANCEMEASUREMENT_H
#define DISTANCEMEASUREMENT_H

#include <QWidget>
#include "ui_DistanceMeasurement.h"

namespace Ui
{
	class DistanceMeasurement;
}

class DistanceMeasurement : public QWidget
{
	Q_OBJECT

public:
	DistanceMeasurement(QWidget *parent = 0);
	~DistanceMeasurement();

	Ui::DistanceMeasurement *ui;
private:
//	Ui::DistanceMeasurement *ui;
};

#endif // DISTANCEMEASUREMENT_H
