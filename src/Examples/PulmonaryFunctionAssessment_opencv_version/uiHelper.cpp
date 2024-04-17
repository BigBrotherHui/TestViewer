#include "uiHelper.h"

QProgressDialog* createProgressDialog(QString title, QString prompt, int range)
{
    QProgressDialog* progressDialog = new QProgressDialog;
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setMinimumDuration(100);
    progressDialog->setWindowTitle(title);
    progressDialog->setLabelText(prompt);
    progressDialog->setCancelButtonText("Cancel");
    progressDialog->setRange(0, range);
    progressDialog->setCancelButton(nullptr);
    return progressDialog;
}
