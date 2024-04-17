#pragma once
#ifndef UIHELPER_H
#define UIHELPER_H
#include <QString>
#include <QMap>
#include <QProgressDialog>

QProgressDialog* createProgressDialog(QString title, QString prompt, int range);

#endif