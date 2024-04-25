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
class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    vtkSmartPointer<vtkPolyData> ExpandSpline(vtkPolyData *line, int divisionNum, double stepSize);
    vtkSmartPointer<vtkPolyData> SweepLine_2Sides(vtkPolyData *line, double direction[3], double distance,
                                                  unsigned int cols);
protected slots:
    void SlotContourEndInteractionEvent(vtkObject*, unsigned long, void*, void*);
private slots:
    void on_pushButton_loaddata_clicked();
    void on_pushButton_addpoint_clicked();
    void on_pushButton_cpr_clicked();
    void on_pushButton_sharedTo3_clicked();
    mitk::DataStorage::Pointer GetDataStorage();

private:
    Ui::MainWindow *ui;
    QmitkStdMultiWidget *w1;
    QmitkStdMultiWidget *w2;
    mitk::StandaloneDataStorage::Pointer ds1;
    mitk::StandaloneDataStorage::Pointer ds2;
    mitk::StandaloneDataStorage::Pointer ds3;
    QmitkRenderWindow *rw;
    mitk::PointSet::Pointer m_ps{nullptr};
    vtkSmartPointer<vtkEventQtSlotConnect> m_scrollConnection{nullptr};
    vtkContourWidget* m_contour_widget_{nullptr};
};
#endif // MAINWINDOW_H
