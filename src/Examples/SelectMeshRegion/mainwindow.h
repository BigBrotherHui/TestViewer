#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkActor.h>
#include <vtkCallbackCommand.h>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class QVTKOpenGLNativeWidget;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    static void lassoselect(vtkObject* caller, long unsigned int vtkNotUsed(eventId), void* vtkNotUsed(clientData),
                     void* vtkNotUsed(callData));
    static void rectselect(vtkObject* caller, long unsigned int vtkNotUsed(eventId), void* vtkNotUsed(clientData),
                            void* vtkNotUsed(callData));
    static void randomselect(vtkObject* caller, long unsigned int vtkNotUsed(eventId), void* vtkNotUsed(clientData),
                           void* vtkNotUsed(callData));
protected slots:
    void on_pushButton_rect_clicked();
    void on_pushButton_lasso_clicked();
    void on_pushButton_random_clicked();

private:
    Ui::MainWindow *ui;
    QVTKOpenGLNativeWidget* mwidget;
    vtkNew<vtkGenericOpenGLRenderWindow> rw;
    vtkNew<vtkActor> actor;
    vtkNew<vtkRenderer> mrenderer;
    vtkSmartPointer<vtkCallbackCommand> lassoselectionCallback = vtkSmartPointer<vtkCallbackCommand>::New();
    vtkSmartPointer<vtkCallbackCommand> rectselectionCallback = vtkSmartPointer<vtkCallbackCommand>::New();
    vtkSmartPointer<vtkCallbackCommand> randomCallback = vtkSmartPointer<vtkCallbackCommand>::New();
};
#endif // MAINWINDOW_H
