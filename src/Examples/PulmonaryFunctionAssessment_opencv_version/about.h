#pragma once

#include <QDialog>
#include "ui_about.h"

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();

private:
    Ui::Widget* ui;
};



