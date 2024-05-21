#ifndef LATTICERESLICEWIDGET_H
#define LATTICERESLICEWIDGET_H

#include <QVTKOpenGLNativeWidget.h>
#include <vtkImageResliceToColors.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkImageActor.h>
class LatticeResliceWidget : public QVTKOpenGLNativeWidget
{
    Q_OBJECT

public:
    LatticeResliceWidget(QWidget *parent = nullptr);
    ~LatticeResliceWidget();
    void setImageReslicer(const vtkSmartPointer<vtkImageResliceToColors>& reslicer);
    void setResliceMatrix(vtkMatrix4x4 *vmt);
    void centerImageActor(std::array<double,3> position);
    const vtkSmartPointer<vtkImageResliceToColors> &getImageResicer();

private:
    vtkSmartPointer<vtkImageResliceToColors> m_reslicer{nullptr};
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderwindow;
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkImageActor> m_actor;
};
#endif // LATTICERESLICEWIDGET_H
