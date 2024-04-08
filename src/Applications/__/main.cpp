//#include <vld.h>
#include <vtkAutoInit.h>
#include "gui.h"
#include "guiframe.h"
#include <QFileDialog>
#include <qtemporarydir.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>
int main(int argc, char* argv[])
{
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication application(argc, argv);
	//vtkObject::GlobalWarningDisplayOff();
	application.setWindowIcon(QIcon(iconapp));
	/*asclepios::gui::GUIFrame guiFrame;
	asclepios::gui::GUI gui;
	guiFrame.setContent(&gui);
        guiFrame.show();*/
        vtkNew<vtkRenderWindow> rw;
        vtkNew<vtkRenderer> rend;
        rw->AddRenderer(rend);
        vtkNew<vtkRenderWindowInteractor> inter;
        rw->SetInteractor(inter);
        rw->Render();
        rw->SetCurrentCursor(VTK_CURSOR_CUSTOM);
        QTemporaryDir tempDir;
        if (tempDir.isValid()) {
            const QString tempFile = tempDir.path() + "/rotate1.cur";
            if (QFile::copy(":/res/rrotate.cur", tempFile)) {
                rw->SetCursorFileName(tempFile.toStdString().c_str());
            }
        }
        inter->Initialize();
        inter->Start();
	return application.exec();
}
