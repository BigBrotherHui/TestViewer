
#include <vtkAutoInit.h>
#include <QApplication>
#include <vtkObject.h>
#include <vtkOBBTree.h>
#include <vtkTransformPolyDataFilter.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
#include "mainwindow.h"
int main(int argc, char* argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication application(argc, argv);
    vtkObject::GlobalWarningDisplayOff();
    MainWindow w;
    w.show();
    return application.exec();
}