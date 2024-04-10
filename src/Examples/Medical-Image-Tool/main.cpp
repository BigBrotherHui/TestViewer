#include "MedicalImageTool.h"
#include <QApplication>

#include <QPixmap>
#include <QSplashScreen>
#include <QTimer>
#include <QEventLoop>
#include <vtkOutputWindow.h>
#include <vtkAutoInit.h>

VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	vtkOutputWindow::GlobalWarningDisplayOff();
	QPixmap pixmap("splashscreen.PNG");
	QSplashScreen splash(pixmap);
	splash.show();
// 	QEventLoop eventloop;
// 	QTimer::singleShot(100, &eventloop, SLOT(quit()));
// 	eventloop.exec();

	MedicalImageTool w;
	w.show();
	return a.exec();
}
