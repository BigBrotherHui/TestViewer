#include <QApplication>
#include "MitkWidgetBase.h"

#include <QmitkStdMultiWidget.h>
#include <QmitkRegisterClasses.h>
#include <QHBoxLayout>
#include <memory>
#include "THAReamWidget.h"
#include <vtkSTLReader.h>
#include <vtkSphereSource.h>
#include <QThread>
int main(int argc, char *argv[])
{
    QmitkRegisterClasses();
    QApplication a(argc, argv);
    //QWidget w;
    //QHBoxLayout *l=new QHBoxLayout(&w);
    //MitkWidgetBase base;
    //QmitkStdMultiWidget *widget=base.createMPRWidget(&w);
    ////this is required because not call it will result in slice view can't be interacted.
    //widget->ResetCrosshair();
    //l->addWidget(widget);
    //base.configMPRWidget(widget);
    //
    //mitk::DataStorage::SetOfObjects::Pointer so=base.load("D:/Images/THA_Pelvis.stl");
    //for (auto s : *so) {
    //    base.addDataNode(s);
    //}
    //base.setWidgetPlaneVisibility(4, true);
    //base.computeBounds(true);
    //base.requestUpdateRender();
    //w.show();
    THAReamWidget w;
    w.show();
    double center[3]{63.078, 175.420, 1228.245};
    w.setCupRadius(25,center);
    vtkNew<vtkSTLReader> reader;
    reader->SetFileName("D:/Images/THA_Pelvis.stl");
    reader->Update();
    w.setSourcePolyData(reader->GetOutput());
    vtkSmartPointer<vtkSphereSource> spGreen = vtkSmartPointer<vtkSphereSource>::New();
    spGreen->SetRadius(25);
    spGreen->SetThetaResolution(40);
    spGreen->SetPhiResolution(40);
    spGreen->Update();
    w.setTool(25, spGreen->GetOutput(), nullptr);
    w.renderResult();
    static double zz = 90.078;
    while (1) {
        vtkSmartPointer<vtkMatrix4x4> vmt = vtkSmartPointer<vtkMatrix4x4>::New();
        vmt->SetElement(0, 3, zz);
        vmt->SetElement(1, 3, 175.420);
        vmt->SetElement(2, 3, 1228.245);
        w.setToolPosture(vmt);
        w.runReam();
        qApp->processEvents(QEventLoop::AllEvents, 20);
        zz -= 0.1;
    }
    return a.exec();
}
