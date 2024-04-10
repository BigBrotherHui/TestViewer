
#include <vtkAutoInit.h>
#include <QApplication>
#include <vtkObject.h>
#include <vtkOBBTree.h>
#include <vtkTransformPolyDataFilter.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
//#include "mainwindow.h"
//int main(int argc, char* argv[])
//{
//    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
//    QApplication application(argc, argv);
//    vtkObject::GlobalWarningDisplayOff();
//    MainWindow w;
//    w.show();
//    return application.exec();
//}
#include <vtkActor.h>
#include <vtkAssembly.h>
#include <vtkAssemblyPath.h>
#include <vtkCellArray.h>
#include <vtkCellPicker.h>
#include <vtkCommand.h>
#include <vtkDataSetMapper.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkExtractPolyDataGeometry.h>
#include <vtkExtractSelection.h>
#include <vtkFollower.h>
#include <vtkIdFilter.h>
#include <vtkIdTypeArray.h>
#include <vtkInteractorStyleRubberBandPick.h>
#include <vtkInteractorStyleTrackballActor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPlaneSource.h>
#include <vtkPlanes.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProp3DCollection.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkSelection.h>
#include <vtkSelectionNode.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkTextActor.h>
#include <vtkTriangle.h>
#include <vtkTriangleFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVectorText.h>
#include <vtkVertexGlyphFilter.h>

#include <vtkAxesActor.h>
#include <vtkPointPicker.h>
class InteractorStyle2 : public vtkInteractorStyleTrackballActor {
public:
    static InteractorStyle2* New();
    vtkTypeMacro(InteractorStyle2, vtkInteractorStyleTrackballActor);
    vtkNew<vtkNamedColors> color;

    InteractorStyle2()
    {
        this->Move = false;
        this->PointPicker = vtkSmartPointer<vtkPointPicker>::New();
    }

    void OnMiddleButtonUp() override
    {
        vtkInteractorStyleTrackballActor::OnMiddleButtonUp();
        // this->EndPan();

        this->Move = false;

        double* newPosition = movableAxes->GetPosition();  // MoveActor->GetPosition();
        MoveActor->SetPosition(newPosition);

        std::cout << "newPosition " << newPosition[0] << " ," << newPosition[1] << " ," << newPosition[2] << std::endl;

        this->GetInteractor()->GetRenderWindow()->Render();
    }
    void OnMiddleButtonDown() override
    {
        vtkInteractorStyleTrackballActor::OnMiddleButtonDown();
        // Get the selected point
        int x = this->Interactor->GetEventPosition()[0];
        int y = this->Interactor->GetEventPosition()[1];
        this->FindPokedRenderer(x, y);

        this->PointPicker->Pick(this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1],
                                0,  // always zero.
                                this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());

        if (this->PointPicker->GetPointId() >= 0) {
            this->MoveActor->VisibilityOn();
            this->Move = true;

            this->GetCurrentRenderer()->AddActor(this->MoveActor);
        }
    }

    vtkSmartPointer<vtkActor> MoveActor;
    vtkSmartPointer<vtkPointPicker> PointPicker;
    bool Move;
    vtkSmartPointer<vtkAssembly> movableAxes;
};
vtkStandardNewMacro(InteractorStyle2);
int main(int, char*[])
{
    // a renderer and render window
    vtkNew<vtkRenderer> renderer;
    vtkNew<vtkRenderWindow> renderWindow;
    renderWindow->AddRenderer(renderer);
    renderWindow->SetWindowName("MovableAxes");

    // an interactor
    vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
    renderWindowInteractor->SetRenderWindow(renderWindow);

    // add the actors to the scene
    // renderer->AddActor(coneActor);
    // renderer->SetBackground(colors->GetColor3d("SlateGray").GetData());

    // vtkAxesActor is currently not designed to work with
    // vtkInteractorStyleTrackballActor since it is a hybrid object containing
    // both vtkProp3D's and vtkActor2D's, the latter of which does not have a 3D
    // position that can be manipulated
    vtkNew<vtkAxesActor> axes;

    // get a copy of the axes' constituent 3D actors and put them into a
    // vtkAssembly so they can be manipulated as one prop
    vtkNew<vtkPropCollection> collection;
    axes->GetActors(collection);

    collection->InitTraversal();

    vtkNew<vtkAssembly> movableAxes;
    int count = collection->GetNumberOfItems();
    for (int i = 0; i < count; ++i) {
        movableAxes->AddPart((vtkProp3D*)collection->GetNextProp());
    }

    renderer->AddActor(movableAxes);


    vtkNew<vtkSphereSource> coneSource;
    coneSource->Update();
    // create a mapper
    vtkNew<vtkPolyDataMapper> coneMapper;
    coneMapper->SetInputConnection(coneSource->GetOutputPort());
    vtkNew<vtkFollower> cone;
    cone->SetMapper(coneMapper);
    cone->SetCamera(renderer->GetActiveCamera());
    cone->SetPosition((movableAxes->GetPosition()));
    cone->PickableOff();
    renderer->AddActor(cone);

    renderer->ResetCamera();
    renderWindow->Render();

    // vtkNew<vtkInteractorStyleTrackballActor> style;
    vtkNew<InteractorStyle2> style;
    style->MoveActor = cone;
    style->movableAxes = movableAxes;
    renderWindowInteractor->SetInteractorStyle(style);
    //  style->AddObserver(vtkCommand::InteractionEvent, callback);

    // begin mouse interaction
    renderWindowInteractor->Start();

    return EXIT_SUCCESS;
}


