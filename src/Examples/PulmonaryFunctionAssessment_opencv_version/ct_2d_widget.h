#pragma once
#include "QVTKOpenGLNativeWidget.h"
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkImageData.h>
#include <vtkRenderWindowInteractor.h>
#include <QProgressDialog>
#include <vtkTextMapper.h>
#include "InteractorStyleImage.h"

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
    CT_2d_Widget(QWidget *parent=nullptr);
    ~CT_2d_Widget();
    void setCTImage(CT_Image* img);
    void Render();
    void Reset();
    void setSlice(int index);
    vtkSmartPointer<vtkImageActor> getImageActor();
    void setDrawModeToDrawRectangle();
    void setDrawModeToDrawEllipse();
    void endDraw();
    void setToImageProcessWidget();
    void setToSegmentWidget();
    int getSlice();
    void applyROI();
protected:
    void setCTImage(vtkSmartPointer<vtkImageData> image);
protected slots:
public slots:

private:

    ViewDirection m_vd{ ViewDirection_Axial };
    vtkSmartPointer<vtkTextMapper> sliceTextMapper;
    vtkNew<vtkTextMapper> AnnotationTextMapper;

    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;
    vtkSmartPointer<vtkRenderer> renderer;
    vtkSmartPointer<vtkImageActor> imageActor;

    vtkSmartPointer<InteractorStyleImage> style;
    CT_Image* mImage;
    int WindowType{0};//0默认 1图像处理 2分割
    int slice{1};
};

