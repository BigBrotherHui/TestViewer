#include <QApplication>
#include "MitkWidgetBase.h"

#include <QmitkStdMultiWidget.h>
#include <QmitkRegisterClasses.h>
#include <QHBoxLayout>
#include <memory>

int main(int argc, char *argv[])
{
    QmitkRegisterClasses();
    QApplication a(argc, argv);
    QWidget w;
    QHBoxLayout *l=new QHBoxLayout(&w);
    MitkWidgetBase base;
    QmitkStdMultiWidget *widget=base.createMPRWidget(&w);
    //this is required because not call it will result in slice view can't be interacted.
    widget->ResetCrosshair();
    l->addWidget(widget);
    base.configMPRWidget(widget);
    
    mitk::DataStorage::SetOfObjects::Pointer so=base.load("D:/Images/THA_Pelvis.stl");
    for (auto s : *so) {
        base.addDataNode(s);
    }
    base.setWidgetPlaneVisibility(4, true);
    base.computeBounds(true);
    base.requestUpdateRender();
    w.show();

    return a.exec();
}
