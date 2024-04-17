#include "about.h"
#include "ui_about.h"

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent),ui(new Ui::Widget)
{
    ui->setupUi(this);
}

AboutDialog::~AboutDialog()
{
}
