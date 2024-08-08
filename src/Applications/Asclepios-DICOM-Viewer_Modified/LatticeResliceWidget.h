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
    void setSlabSliceCount(double slabSliceCount);
    void setSpacing(double spacing);
signals:
    void signal_wallChanged(bool isUp);

private:
    vtkSmartPointer<vtkImageResliceToColors> m_reslicer{nullptr};
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderwindow;
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkImageSlice> m_actor;
    double m_slabSliceCount{1};
    double m_spacing{1};
};
#endif // LATTICERESLICEWIDGET_H
