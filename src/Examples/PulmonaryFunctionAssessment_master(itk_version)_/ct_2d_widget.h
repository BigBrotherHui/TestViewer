#pragma once
#include "QVTKOpenGLNativeWidget.h"
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkImageData.h>
#include <vtkRenderWindowInteractor.h>
#include <QProgressDialog>
#include <vtkImageViewer2.h>
#include <vtkTextMapper.h>
class vtkEventQtSlotConnect;

class CT_Image;
namespace Ui {
    class CT_3d_Widget;
}
enum ViewDirection
{
    ViewDirection_Axial=0,
    ViewDirection_Coronal,
    ViewDirection_Sagittal
};
class CT_2d_Widget : public QVTKOpenGLNativeWidget
{
    Q_OBJECT

public:
    CT_2d_Widget(QWidget *parent);
    ~CT_2d_Widget();
    void setCTImage(CT_Image* img);
    void setCTImage(vtkSmartPointer<vtkImageData> image);
    void setViewDirection(ViewDirection d);
    vtkImageViewer2* getImageViewer2();
    void Render();
    
protected:
    void autoAdjustImagePos();
    void resizeEvent(QResizeEvent* event) override;
protected slots:
public slots:

private:
    vtkSmartPointer<vtkImageViewer2> mImageViewer{ nullptr };
    ViewDirection m_vd{ ViewDirection_Axial };
    vtkSmartPointer<vtkTextMapper> sliceTextMapper;
    vtkNew<vtkTextMapper> AnnotationTextMapper;
};

