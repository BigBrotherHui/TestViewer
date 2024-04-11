#ifndef VTKPLOTPIEWIDGET_H
#define VTKPLOTPIEWIDGET_H
#include <QVTKOpenGLNativeWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include "VtkPieChartActor.h"
class VtkPlotPieWidgetPrivate;
class VtkPlotPieWidget : public QVTKOpenGLNativeWidget
{
    Q_OBJECT
public:
    explicit VtkPlotPieWidget(QWidget* parent = nullptr);
    ~VtkPlotPieWidget();
    void setData(const std::map<std::string, int> &data);
private:
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;
    vtkSmartPointer<vtkRenderer> renderer;
    vtkSmartPointer<VtkPieChartActor> actor;
};
#endif
;
