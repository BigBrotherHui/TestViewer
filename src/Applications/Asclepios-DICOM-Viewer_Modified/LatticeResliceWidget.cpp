#include "LatticeResliceWidget.h"
#include <vtkImageResliceMapper.h>
#include <vtkInteractorStyleImage.h>

#include "utils.h"
 class InteractorStyleImage : public vtkInteractorStyleImage {
 public:
     static InteractorStyleImage* New(){ 
         InteractorStyleImage* style = new InteractorStyleImage;
         return style;
     }
     vtkTypeMacro(InteractorStyleImage, vtkInteractorStyleImage);
     InteractorStyleImage() = default;
     ~InteractorStyleImage() = default;
     void OnMouseWheelForward() override;
     void OnMouseWheelBackward() override;
 };

 class LatticeResliceWidget_Callback : public vtkCommand {
 public:
     static LatticeResliceWidget_Callback* New() { return new LatticeResliceWidget_Callback; }
     vtkTypeMacro(LatticeResliceWidget_Callback, vtkCommand);
     void Execute(vtkObject* caller, unsigned long eventId, void* callData) override
     { 
         if (!m_latticeResliceWidget) return;
         if (eventId == wallUp)
         {
             emit m_latticeResliceWidget->signal_wallChanged(true);
         }
         else if (eventId == wallDown) {
             emit m_latticeResliceWidget->signal_wallChanged(false);
         }
     }
     LatticeResliceWidget* m_latticeResliceWidget{nullptr};
 };

 void InteractorStyleImage::OnMouseWheelForward()
 {
     InvokeEvent(wallUp);
 }

 void InteractorStyleImage::OnMouseWheelBackward()
 {
     InvokeEvent(wallDown);
 }

LatticeResliceWidget::LatticeResliceWidget(QWidget *parent)
    : QVTKOpenGLNativeWidget(parent)
{
    m_renderwindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    setRenderWindow(m_renderwindow);
    m_renderer = vtkSmartPointer<vtkRenderer>::New();
    m_renderwindow->AddRenderer(m_renderer);
    m_actor = vtkSmartPointer<vtkImageActor>::New();
    m_renderer->AddActor(m_actor);
     auto style = vtkSmartPointer<InteractorStyleImage>::New();
    auto callback = vtkSmartPointer<LatticeResliceWidget_Callback>::New();
     callback->m_latticeResliceWidget = this;
     style->AddObserver(wallUp,callback);
     style->AddObserver(wallDown, callback);
     interactor()->SetInteractorStyle(style);
}

LatticeResliceWidget::~LatticeResliceWidget()
{
}

void LatticeResliceWidget::setImageReslicer(const vtkSmartPointer<vtkImageResliceToColors>& reslicer)
{
    if (!reslicer || m_reslicer == reslicer) return;
    m_reslicer = reslicer;
    static_cast<vtkImageResliceMapper*>(m_actor->GetMapper())->SetInputConnection(m_reslicer->GetOutputPort());
}

void LatticeResliceWidget::setResliceMatrix(vtkMatrix4x4* vmt)
{
    if (!m_reslicer)
       return;
    vtkSmartPointer<vtkImageResliceToColors> reslicer = vtkSmartPointer<vtkImageResliceToColors>::New();
    reslicer->SetInputData(m_reslicer->GetInput());
    reslicer->SetLookupTable(m_reslicer->GetLookupTable());
    reslicer->SetResliceAxes(vmt);
    reslicer->BypassOff();
    reslicer->SetInformationInput(m_reslicer->GetInformationInput());
    reslicer->SetOutputDimensionality(2);
    reslicer->SetSlabNumberOfSlices(m_slabSliceCount);
    if (m_slabSliceCount > 1)
        reslicer->SetSlabModeToMax();
    reslicer->SetOutputFormatToRGB();
    reslicer->Update();

    m_actor->SetInputData(reslicer->GetOutput());

    m_renderer->ResetCamera();
    m_renderwindow->Render();
}

void LatticeResliceWidget::centerImageActor(std::array<double, 3> position)
{
    vtkPropCollection* actorCollection = m_renderer->GetViewProps();
    actorCollection->InitTraversal();
    for (vtkIdType j = 0; j < actorCollection->GetNumberOfItems(); j++) {
        vtkProp* nextProp = actorCollection->GetNextProp();
        if (nextProp->IsA("vtkImageSlice")) {
            auto* const actor = reinterpret_cast<vtkImageActor*>(nextProp);
            auto* const actorPosition = actor->GetPosition();
            double newPosition[3] = {position[0] + actorPosition[0] - actor->GetCenter()[0],
                                     position[1] + actorPosition[1] - actor->GetCenter()[1], 0.1};
            actor->SetPosition(actorPosition[0] - actor->GetCenter()[0], actorPosition[1] - actor->GetCenter()[1],
                               actorPosition[2] - actor->GetCenter()[2]);
        }
    }
}

const vtkSmartPointer<vtkImageResliceToColors> &LatticeResliceWidget::getImageResicer()
{
    return m_reslicer;
}

void LatticeResliceWidget::setSlabSliceCount(int slabSliceCount){
    m_slabSliceCount=slabSliceCount;
}
