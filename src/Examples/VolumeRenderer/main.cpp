#include "mainwindow.h"
#include <QApplication>

#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)
VTK_MODULE_INIT(vtkRenderingFreeType)
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2)
int main(int argc, char *argv[])
{
	//ignore global warning window
    vtkOutputWindow::GlobalWarningDisplayOff();
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    
    return a.exec();
}
