#include <vtkActor.h>
// #include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkCaptionActor2D.h>
#include <vtkCaptionRepresentation.h>
#include "./CaptionWidget.h"
#include <vtkNamedColors.h>
#include <vtkNew.h>
// #include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);

class Callback : public vtkCommand {
public:
    static Callback* New() { return new Callback; }
    void Execute(vtkObject*, unsigned long eventId, void*) override
    {
        switch (eventId) {
            case vtkCommand::LeftButtonDoubleClickEvent:
                std::cout << "fuck" << std::endl;
                break;
        }
    }
    Callback(){}
};
int main(int, char*[])
{
    vtkNew<vtkNamedColors> colors;

    // Sphere
    vtkNew<vtkSphereSource> sphereSource;
    sphereSource->Update();

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(sphereSource->GetOutputPort());

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(colors->GetColor3d("DarkOliveGreen").GetData());

    // A renderer and render window.
    vtkNew<vtkRenderer> renderer;
    vtkNew<vtkRenderWindow> renderWindow;
    renderWindow->AddRenderer(renderer);
    renderWindow->SetWindowName("CaptionWidget");

    // An interactor
    vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
    renderWindowInteractor->SetRenderWindow(renderWindow);

    // Create the widget and its representation.
    vtkNew<vtkCaptionRepresentation> captionRepresentation;
    captionRepresentation->GetCaptionActor2D()->SetCaption("Test caption");
    captionRepresentation->GetCaptionActor2D()->GetTextActor()->GetTextProperty()->SetFontSize(100);

    double pos[3] = {.5, 0, 0};
    captionRepresentation->SetAnchorPosition(pos);

    vtkNew<CaptionWidget> captionWidget;
    captionWidget->SetInteractor(renderWindowInteractor);

    captionWidget->SetRepresentation(captionRepresentation);
    vtkSmartPointer<Callback> callback = vtkSmartPointer<Callback>::New();
    captionWidget->AddObserver(vtkCommand::LeftButtonDoubleClickEvent, callback);

    // Add the actors to the scene.
    renderer->AddActor(actor);
    renderer->SetBackground(colors->GetColor3d("Blue").GetData());

    renderWindow->Render();

    // Rotate the camera to bring the point the caption is pointing to into view.
    renderer->GetActiveCamera()->Azimuth(90);

    captionWidget->On();
    captionWidget->SetSelectable(false);
    captionWidget->SetResizable(false);
    // captionWidget->Off();
    captionWidget->PickingManagedOff();
    captionWidget->SetEnabled(true);
    // Begin mouse interaction
    renderWindowInteractor->Start();

    return EXIT_SUCCESS;
}