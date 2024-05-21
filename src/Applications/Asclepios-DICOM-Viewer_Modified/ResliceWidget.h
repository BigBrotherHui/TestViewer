#ifndef RESLICEWIDGET_H
#define RESLICEWIDGET_H

#include <QVTKOpenGLNativeWidget.h>
#include <vtkImageResliceToColors.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkImageActor.h>
class ResliceWidget : public QVTKOpenGLNativeWidget
{
    Q_OBJECT

public:
    ResliceWidget(QWidget *parent = nullptr);
    ~ResliceWidget();
    void setImageReslicer(const vtkSmartPointer<vtkImageResliceToColors>& reslicer);
    void setResliceMatrix(vtkMatrix4x4 *vmt);
private:
    vtkSmartPointer<vtkImageResliceToColors> m_reslicer{nullptr};
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderwindow;
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkImageActor> m_actor;
};
#endif // WIDGET_H
