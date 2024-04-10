#include "Crosshair.h"

Crosshair::Crosshair(QWidget *parent)
	: QWidget(parent), ui(new Ui::Crosshair)
{
	ui->setupUi(this);

	this->setWindowTitle("Crosshair");

	this->setMaximumSize(371, 291);
	this->setMinimumSize(371, 291);
}

Crosshair::~Crosshair()
{
	delete ui;
}

void Crosshair::on_pushButtonClose_clicked()
{
	this->close();
}