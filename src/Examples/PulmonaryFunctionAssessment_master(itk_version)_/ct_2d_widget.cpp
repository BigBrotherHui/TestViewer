
#include <vtkCoordinate.h>
#include "CT_2d_Widget.h"
#include <vtkNew.h>
#include <vtkDICOMImageReader.h>
#include <vtkImageCast.h>
#include "vtkHelper.h"
#include <vtkPiecewiseFunction.h>
#include <vtkVolume.h>
#include <vtkColorTransferFunction.h>
#include <vtkVolumeProperty.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkTransform.h>
#include <vtkNamedColors.h>
#include <vtkCamera.h>
#include <QDebug>
#include <vtkActor.h>
#include <vtkProp3D.h>
#include <vtkPolyDataMapper.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkSmartVolumeMapper.h>
#include "ct_image.h"
#include <vtkImageActor.h>
#include <vtkImageProperty.h>
#include <vtkInteractorStyleImage.h>
#include "myInteractorStyleImage.h"
#include <vtkTextProperty.h>
#include <vtkTextMapper.h>

CT_2d_Widget::CT_2d_Widget(QWidget *parent = Q_NULLPTR)
{
    mImageViewer = vtkSmartPointer<vtkImageViewer2>::New();
    mImageViewer->GetRenderer()->GetActiveCamera()->SetParallelProjection(1);
    renderWindow()->AddRenderer(mImageViewer->GetRenderer());
    mImageViewer->GetRenderWindow()->SetOffScreenRendering(1);
    vtkNew<vtkTextProperty> sliceTextProp;
    sliceTextProp->SetFontFamilyToCourier();
    sliceTextProp->SetFontSize(18);
    sliceTextProp->SetVerticalJustificationToBottom();
    sliceTextProp->SetJustificationToLeft();
    sliceTextMapper = vtkSmartPointer<vtkTextMapper>::New();
    std::string msg = StatusMessage::Format(mImageViewer->GetSliceMin(),
        mImageViewer->GetSliceMax());
    sliceTextMapper->SetInput(msg.c_str());
    sliceTextMapper->SetTextProperty(sliceTextProp);
    vtkNew<vtkActor2D> sliceTextActor;
    sliceTextActor->SetMapper(sliceTextMapper);
    sliceTextActor->SetPosition(15, 0);

    // usage hint message
    vtkNew<vtkTextProperty> usageTextProp;
    usageTextProp->SetFontFamilyToCourier();
    usageTextProp->SetFontSize(18);
    usageTextProp->SetVerticalJustificationToTop();
    usageTextProp->SetJustificationToLeft();

    vtkNew<vtkTextMapper> usageTextMapper;
    usageTextMapper->SetInput(
        "- Slice with mouse wheel\n  or Up/Down-Key\n- Zoom with pressed right\n "
        " mouse button while dragging");
    usageTextMapper->SetTextProperty(usageTextProp);

    vtkNew<vtkActor2D> usageTextActor;
    usageTextActor->SetMapper(usageTextMapper);
    usageTextActor->GetPositionCoordinate()
        ->SetCoordinateSystemToNormalizedDisplay();
    usageTextActor->GetPositionCoordinate()->SetValue(0.05, 0.95);

    mImageViewer->GetRenderer()->AddActor2D(sliceTextActor);
    mImageViewer->GetRenderer()->AddActor2D(usageTextActor);

    {
        vtkNew<vtkTextProperty> usageTextProp;
        usageTextProp->SetFontFamilyToCourier();
        usageTextProp->SetFontSize(18);
        usageTextProp->SetVerticalJustificationToBottom();
        usageTextProp->SetJustificationToLeft();

        AnnotationTextMapper->SetInput("Axial");
        AnnotationTextMapper->SetTextProperty(usageTextProp);

        vtkNew<vtkActor2D> usageTextActor;
        usageTextActor->SetMapper(AnnotationTextMapper);
        usageTextActor->SetPosition(260, 0);

        mImageViewer->GetRenderer()->AddActor2D(usageTextActor);
    }
}


CT_2d_Widget::~CT_2d_Widget()
{
}

void CT_2d_Widget::setCTImage(CT_Image *img)
{
    setCTImage(img->getCTImageDataVtk());
}

void CT_2d_Widget::setCTImage(vtkSmartPointer<vtkImageData> image)
{
    mImageViewer->SetInputData(image);
    int sliceMax = image->GetDimensions()[2];
    //autoAdjustImagePos();
    setViewDirection(m_vd);
    vtkNew<myInteractorStyleImage> myInteractorStyle;
    myInteractorStyle->SetImageViewer(this);
    myInteractorStyle->SetStatusMapper(sliceTextMapper);
    GetInteractor()->SetInteractorStyle(myInteractorStyle);
    myInteractorStyle->initSlice();
    myInteractorStyle->MoveSliceBackward();
}

void CT_2d_Widget::setViewDirection(ViewDirection d)
{
    if (m_vd == d)
        return;
    if (d == ViewDirection_Axial)
    {
        mImageViewer->SetSliceOrientationToXY();
        AnnotationTextMapper->SetInput("Axial");
    }
    else if (d == ViewDirection_Coronal)
    {
        mImageViewer->SetSliceOrientationToXZ();
        AnnotationTextMapper->SetInput("Coronal");
    } 
    else
    {
        mImageViewer->SetSliceOrientationToYZ();
        AnnotationTextMapper->SetInput("Sagittal");
    }
    m_vd = d;
    renderWindow()->Render();
}

vtkImageViewer2* CT_2d_Widget::getImageViewer2()
{
    return mImageViewer;
}

void CT_2d_Widget::Render()
{
    renderWindow()->Render();
}

inline void CT_2d_Widget::autoAdjustImagePos()
{
    double windowWidth = renderWindow()->GetSize()[0];
    double windowHeight = renderWindow()->GetSize()[1];
    vtkImageData* img = mImageViewer->GetInput();
    if (!img)
        return;
    // Assuming you have a reference to your scene's bounding box or extent
    double sceneWidth = img->GetBounds()[1] - img->GetBounds()[0];
    double sceneHeight = img->GetBounds()[3] - img->GetBounds()[2];
    if(ViewDirection_Sagittal== m_vd)
    {
        sceneWidth = img->GetBounds()[3] - img->GetBounds()[2];
        sceneHeight = img->GetBounds()[5] - img->GetBounds()[4];
    }else if(m_vd==ViewDirection_Coronal)
    {
        sceneWidth = img->GetBounds()[1] - img->GetBounds()[0];
        sceneHeight = img->GetBounds()[5] - img->GetBounds()[4];
    }
    // Calculate the aspect ratios
    double windowAspectRatio = windowWidth / windowHeight;
    double sceneAspectRatio = sceneWidth / sceneHeight;

    double parallelScale;
    if (windowAspectRatio > sceneAspectRatio)
    {
        parallelScale = sceneHeight / 2.0;
    }
    else
    {
        parallelScale = (sceneWidth / windowAspectRatio) / 2.0;
    }
    mImageViewer->GetRenderer()->GetActiveCamera()->SetParallelScale(parallelScale);
}

void CT_2d_Widget::resizeEvent(QResizeEvent* event)
{
    //autoAdjustImagePos();
    return QVTKOpenGLNativeWidget::resizeEvent(event);
}
