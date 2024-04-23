

#include <QApplication>
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);
#include "widget.h"
int main( int argc, char *argv[] )
{
    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}

