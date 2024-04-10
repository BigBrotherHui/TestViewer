#include "STLSet.h"

STLSet::STLSet(QWidget *parent)
	: QWidget(parent), ui(new Ui::STLSet)
{
	ui->setupUi(this);
	this->setWindowTitle("Image To Ply Set Parameter");
	this->setMaximumSize(488,251);
	this->setMinimumSize(488,251);
}

STLSet::~STLSet()
{
	delete ui;
}

