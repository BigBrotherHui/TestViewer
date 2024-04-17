
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
#include <vtkTextProperty.h>
#include <vtkTextMapper.h>
#include <vtkPointPicker.h>
#include <vtkInformation.h>

CT_2d_Widget::CT_2d_Widget(QWidget *parent)
{
    renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow->AddRenderer(renderer);
    setRenderWindow(renderWindow);
    imageActor = vtkSmartPointer<vtkImageActor>::New();
    renderer->AddActor(imageActor);

    vtkNew<vtkTextProperty> sliceTextProp;
    sliceTextProp->SetFontFamilyToCourier();
    sliceTextProp->SetFontSize(18);
    sliceTextProp->SetVerticalJustificationToBottom();
    sliceTextProp->SetJustificationToLeft();
    sliceTextMapper = vtkSmartPointer<vtkTextMapper>::New();
    sliceTextMapper->SetTextProperty(sliceTextProp);

    vtkNew<vtkActor2D> sliceTextActor;
    sliceTextActor->SetMapper(sliceTextMapper);
    sliceTextActor->SetPosition(15, 0);
    renderer->AddActor(sliceTextActor);

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
    renderer->AddActor(usageTextActor);

    style = vtkSmartPointer<InteractorStyleImage>::New();
    style->SetStatusMapper(sliceTextMapper);
    style->SetWidget(this);
    GetInteractor()->SetInteractorStyle(style);

    vtkSmartPointer<vtkPointPicker> picker = vtkSmartPointer<vtkPointPicker>::New();
    picker->SetTolerance(1e-10);
    GetInteractor()->SetPicker(picker);
}


CT_2d_Widget::~CT_2d_Widget()
{

}

void CT_2d_Widget::setCTImage(CT_Image *img)
{
    std::string msg = StatusMessage::Format(0,img->getNumberOfImages());
    sliceTextMapper->SetInput(msg.c_str());
    if (WindowType == 1)
        setCTImage(img->getCTImageDataVtk_Registered(0));
    else if (WindowType == 2)
        setCTImage(img->getCTImageDataVtk_Segmented(0));
    else
		setCTImage(img->getCTImageDataVtk(0));
    mImage = img;
    if (WindowType == 2)
    {
        style->showSegmentActor = 1;
    }
    style->setImageActor(imageActor);
}

void CT_2d_Widget::setCTImage(vtkSmartPointer<vtkImageData> image)
{
    if (!image || image->GetNumberOfPoints()==0)
        return;
    if(image->GetScalarType()!=VTK_UNSIGNED_CHAR)
    {
        vtkSmartPointer<vtkImageData> img = vtkSmartPointer<vtkImageData>::New();
        img->DeepCopy(image);
        if(WindowType!=0)
        {
            img->SetOrigin(0, 0, 0);
            img->SetSpacing(1, 1, 1);
        }
        double max = img->GetScalarRange()[1];
        double min = img->GetScalarRange()[0];
        int* dims = img->GetDimensions();
        for(int i=0;i<dims[2];++i)
        {
	        for(int j=0;j<dims[1];++j)
	        {
		        for(int k=0;k<dims[0];++k)
		        {
                    short * v= (short *)img->GetScalarPointer(k, j, i);
                    *v = (*v - min) * 255 / (max - min) ;
		        }
	        }
        }
        img->Modified();
        vtkSmartPointer<vtkImageCast> cast = vtkSmartPointer<vtkImageCast>::New();
        cast->SetInputData(img);
        cast->SetOutputScalarTypeToUnsignedChar();
        cast->Update();
        imageActor->SetInputData(cast->GetOutput());
    }
    else
    {
        vtkSmartPointer<vtkImageData> img = vtkSmartPointer<vtkImageData>::New();
        img->DeepCopy(image);
        if (WindowType != 0)
        {
            img->SetOrigin(0, 0, 0);
            img->SetSpacing(1, 1, 1);
        }
        imageActor->SetInputData(img);
    }
    imageActor->SetPosition(0, 0, 0);
    imageActor->GetProperty()->SetColorLevel(127);
    imageActor->GetProperty()->SetColorWindow(255);
    Render();
}

void CT_2d_Widget::Render()
{
    renderWindow->Render();
}

void CT_2d_Widget::Reset()
{
    imageActor->GetProperty()->SetColorLevel(127);
    imageActor->GetProperty()->SetColorWindow(255);
    renderer->ResetCamera();
    Render();
}

void CT_2d_Widget::setSlice(int index)
{
    if (mImage->getNumberOfImages() == 1)
        return;
    if(WindowType==1)
    {
        setCTImage(mImage->getCTImageDataVtk_Registered(index));
    }
    else if(WindowType==2)
    {
        setCTImage(mImage->getCTImageDataVtk_Segmented_Src(index));
    }
    else
    {
        setCTImage(mImage->getCTImageDataVtk(index));
    }
    slice = index;
}

vtkSmartPointer<vtkImageActor> CT_2d_Widget::getImageActor()
{
    return imageActor;
}

void CT_2d_Widget::setDrawModeToDrawRectangle()
{
    style->drawState = 1;
    style->drawMode = InteractorStyleImage::DRAW_RECTANGLE;
}

void CT_2d_Widget::setDrawModeToDrawEllipse()
{
    style->drawState = 1;
    style->drawMode = InteractorStyleImage::DRAW_ELLIPSE;
}

void CT_2d_Widget::endDraw()
{
    style->drawState = 0;
    style->drawMode = InteractorStyleImage::DRAW_NOTUSED;
}

void CT_2d_Widget::setToImageProcessWidget()
{
    WindowType = 1;
}

void CT_2d_Widget::setToSegmentWidget()
{
    WindowType = 2;
}

int CT_2d_Widget::getSlice()
{
    return slice;
}

void CT_2d_Widget::applyROI()
{
    if(WindowType!=2)
        return;
    style->applyROI();
}
