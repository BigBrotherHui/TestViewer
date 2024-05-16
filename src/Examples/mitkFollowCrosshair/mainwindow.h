#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <mitkStandaloneDataStorage.h>
#include <mitkPointSet.h>
#include <vtkEventQtSlotConnect.h>
#include <vtkSmartPointer.h>
#include <vtkContourWidget.h>
#include <vtkActor.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class QmitkStdMultiWidget;
class QmitkRenderWindow;
class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void ModifyObserved(const itk::EventObject &e);
private slots:
    void on_pushButton_loaddata_clicked();

private:
    Ui::MainWindow *ui;
    QmitkStdMultiWidget *w1;
    mitk::StandaloneDataStorage::Pointer ds1;

};
#endif // MAINWINDOW_H
