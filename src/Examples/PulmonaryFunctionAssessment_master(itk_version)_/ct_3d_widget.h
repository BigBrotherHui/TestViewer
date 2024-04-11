#pragma once
#include "QVTKOpenGLNativeWidget.h"
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkImageData.h>
#include <vtkRenderWindowInteractor.h>
#include <QProgressDialog>
#include <vtkProperty.h>
#include <QVector>
class CT_Image;

namespace Ui {
    class CT_3d_Widget;
}

class CT_3d_Widget : public QVTKOpenGLNativeWidget
{
    Q_OBJECT

public:
    CT_3d_Widget(QWidget *parent);
    ~CT_3d_Widget();
    void setRenderWindowSize(int height, int width);
    vtkGenericOpenGLRenderWindow* getRenderWindow();
    void setCTImage(CT_Image *ctImage);
    void setCTImage(vtkSmartPointer<vtkImageData> image);
    void setMesh(vtkSmartPointer<vtkPolyData> p);
    vtkProperty* getLastPickedProperty();
    void setLastPickedProperty(vtkProperty* lastPickedProperty);
    void reset();
    void getCameraSettings(double* position, double* focalPoint);

public slots:

private:
    vtkGenericOpenGLRenderWindow* renWin;
    vtkRenderer* ren;
    vtkSmartPointer<vtkImageData> ctImage;
    QVTKInteractor* interactor;
    vtkProperty* lastPickedProperty;
    double lastAngle = 0;
};

