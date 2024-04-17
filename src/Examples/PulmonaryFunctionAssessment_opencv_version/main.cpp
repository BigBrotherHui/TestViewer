#include "mainwindow.h"
#include <QApplication>
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);
#include <vtkOutputWindow.h>
int main(int argc, char *argv[])
{
    vtkOutputWindow::GlobalWarningDisplayOff();
    QApplication a(argc, argv);
    MainWindow w;
    w.setAttribute(Qt::WA_DeleteOnClose);
    w.showMaximized();
    return a.exec();
}