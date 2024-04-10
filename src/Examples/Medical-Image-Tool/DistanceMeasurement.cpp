#include "DistanceMeasurement.h"

DistanceMeasurement::DistanceMeasurement(QWidget *parent)
	: QWidget(parent),ui(new Ui::DistanceMeasurement)
{
	ui->setupUi(this);
	this->setWindowTitle("DistanceMeasurement");
	this->setMaximumSize(311, 241);
	this->setMinimumSize(311, 241);
}

DistanceMeasurement::~DistanceMeasurement()
{
	delete ui;
}
