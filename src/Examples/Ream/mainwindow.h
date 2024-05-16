#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <QTimer>
#include <vtkSphereSource.h>
#include <vtkImageData.h>
#include <QtConcurrent/QtConcurrent>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class QVTKOpenGLNativeWidget;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
protected slots:
    void on_pushButton_move_clicked();
    void on_pushButton_hide_clicked();

private:
    Ui::MainWindow *ui;
    QVTKOpenGLNativeWidget* mwidget;
    vtkNew<vtkGenericOpenGLRenderWindow> rw;
    vtkNew<vtkActor> actor;
    vtkNew<vtkRenderer> mrenderer;

    vtkNew<vtkActor> actorsphere;
    vtkNew<vtkSphereSource> sp;
    double center[3];
    vtkSmartPointer<vtkImageData> image;
    QFutureWatcher<void> watcher;
    vtkSmartPointer<vtkPolyData> ret = vtkSmartPointer<vtkPolyData>::New();
    vtkPolyData *out;
};
#endif // MAINWINDOW_H
