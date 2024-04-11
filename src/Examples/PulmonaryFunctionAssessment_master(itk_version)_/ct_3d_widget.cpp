#include <vtkAutoInit.h> 
#include <vtkTextMapper.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
VTK_MODULE_INIT(vtkRenderingFreeType);
#include "ct_3d_widget.h"
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
#include <vtkTextProperty.h>
#include <vtkActor2D.h>

namespace {
    // this is a private class for progress update
    class ReadCTProgressUpdate : public vtkCommand
    {
    public:
        static ReadCTProgressUpdate* New()
        {
            return new ReadCTProgressUpdate;
        }
        void setDialog(QProgressDialog* dialog)
        {
            this->dialog = dialog;
        }
        virtual void Execute(vtkObject*caller, unsigned long eventId, void* callData)
        {
            double* progress = static_cast<double*>(callData);
            this->dialog->setValue((int)(*progress * 100));
        }
    private:
        QProgressDialog* dialog;
    };
}

CT_3d_Widget::CT_3d_Widget(QWidget *parent = Q_NULLPTR)
{
    // create the vtk window
    vtkNew<vtkGenericOpenGLRenderWindow> win;
    win->SetMultiSamples(0);
    this->renWin = win;
    this->setRenderWindow(this->renWin);

    // create the render
    vtkNew<vtkRenderer> render;
    this->renWin->AddRenderer(render);
    this->ren = render;

    // init last picked screw vtk property (nasty design)
    this->lastPickedProperty = vtkProperty::New();

    vtkNew<vtkTextProperty> usageTextProp;
    usageTextProp->SetFontFamilyToCourier();
    usageTextProp->SetFontSize(18);
    usageTextProp->SetVerticalJustificationToTop();
    usageTextProp->SetJustificationToLeft();

    vtkNew<vtkTextMapper> usageTextMapper;
    usageTextMapper->SetInput("3D");
    usageTextMapper->SetTextProperty(usageTextProp);

    vtkNew<vtkActor2D> usageTextActor;
    usageTextActor->SetMapper(usageTextMapper);
    usageTextActor->GetPositionCoordinate()
        ->SetCoordinateSystemToNormalizedDisplay();
    usageTextActor->GetPositionCoordinate()->SetValue(0.90, 0.05);
    vtkNew<vtkInteractorStyleTrackballCamera> style;
    this->interactor = GetInteractor();
    interactor->SetInteractorStyle(style);
    ren->AddActor2D(usageTextActor);
}


CT_3d_Widget::~CT_3d_Widget()
{
    this->lastPickedProperty->Delete();
}

void CT_3d_Widget::setRenderWindowSize(int height, int width)
{
    this->renWin->SetSize(height, width);
}

vtkGenericOpenGLRenderWindow * CT_3d_Widget::getRenderWindow()
{
    return this->renWin;
}

void CT_3d_Widget::setCTImage(CT_Image* ctImage)
{
    setCTImage(ctImage->getCTImageDataVtk());
}

void CT_3d_Widget::setCTImage(vtkSmartPointer<vtkImageData> image)
{
    this->ctImage = image;
    // create mapper, filters to render Dicom data
    vtkNew<vtkPiecewiseFunction> opacityTransfunc;
    opacityTransfunc->AddPoint(70, 0.0);
    opacityTransfunc->AddPoint(90, 0.4);
    opacityTransfunc->AddPoint(180, 0.6);

    vtkNew<vtkColorTransferFunction> colorTransferFunction;
    colorTransferFunction->AddRGBPoint(0.0, 0.0, 0.0, 0.0);
    colorTransferFunction->AddRGBPoint(64.0, 0.25, 0.25, 0.25);
    colorTransferFunction->AddRGBPoint(190.0, 0.5, 0.5, 0.5);
    colorTransferFunction->AddRGBPoint(220.0, 1, 1, 1);

    vtkNew<vtkVolumeProperty> ctVolumeProperty;
    ctVolumeProperty->SetColor(colorTransferFunction);
    ctVolumeProperty->SetScalarOpacity(opacityTransfunc);
    ctVolumeProperty->ShadeOn();
    ctVolumeProperty->SetInterpolationTypeToLinear();
    ctVolumeProperty->SetAmbient(0.2);
    ctVolumeProperty->SetDiffuse(0.9);
    ctVolumeProperty->SetSpecular(0.5);
    ctVolumeProperty->SetSpecularPower(50);

    // connect the pipeline and create volume actor
    vtkNew<vtkSmartVolumeMapper> ctMapper;
    ctMapper->SetInputData(this->ctImage);
    vtkNew<vtkVolume> ctVolume;
    ctVolume->SetMapper(ctMapper);
    ctVolume->SetProperty(ctVolumeProperty);
    ctVolume->PickableOff();

    // add actor to render
    this->ren->AddActor(ctVolume);
    this->ren->ResetCamera();

    vtkCamera* camera = ren->GetActiveCamera();
    camera->SetViewUp(0, 0, 1);
    camera->SetPosition(0, 800, 0);
    camera->SetFocalPoint(ctImage->GetCenter());

    ren->ResetCameraClippingRange();

    this->renWin->Render();
}

void CT_3d_Widget::setMesh(vtkSmartPointer<vtkPolyData> p)
{
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    actor->SetMapper(mapper);
    mapper->SetInputData(p);
    this->ren->AddActor(actor);
    vtkCamera* camera = ren->GetActiveCamera();
    camera->SetViewUp(0, 0, 1);
    camera->SetPosition(0, 800, 0);
    camera->SetFocalPoint(p->GetCenter());

    ren->ResetCameraClippingRange();
    this->renderWindow()->Render();
}

vtkProperty * CT_3d_Widget::getLastPickedProperty()
{
    return this->lastPickedProperty;
}

void CT_3d_Widget::setLastPickedProperty(vtkProperty * lastPickedProperty)
{
    this->lastPickedProperty = lastPickedProperty;
}

void CT_3d_Widget::reset()
{
    this->ren->ResetCamera();
}

void CT_3d_Widget::getCameraSettings(double * position, double * focalPoint)
{
    position[0] = this->ren->GetActiveCamera()->GetPosition()[0];
    position[1] = this->ren->GetActiveCamera()->GetPosition()[1];
    position[2] = this->ren->GetActiveCamera()->GetPosition()[2];

    focalPoint[0] = this->ren->GetActiveCamera()->GetFocalPoint()[0];
    focalPoint[1] = this->ren->GetActiveCamera()->GetFocalPoint()[1];
    focalPoint[2] = this->ren->GetActiveCamera()->GetFocalPoint()[2];
}