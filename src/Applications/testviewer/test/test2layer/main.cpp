
#include <vtkAutoInit.h>
#include <QApplication>
#include <vtkObject.h>
#include <vtkOBBTree.h>
#include <vtkTransformPolyDataFilter.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
#include <vtkCommand.h>
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
#include <vtkAutoInit.h>
#include <vtkAxesActor.h>
#include <vtkCubeAxesActor.h>
#include <vtkCylinderSource.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkObjectFactory.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPointPicker.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkSTLReader.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkProperty.h>
#include <vtkCamera.h>
#include <QDebug>
#include <vtkLineSource.h>
#include <vtkArrowSource.h>
#include <vtkMinimalStandardRandomSequence.h>
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>
#include <vtkAppendPolyData.h>

vtkSmartPointer<vtkPolyData> CreateArrow(double &pdLength, std::array<double, 3> &startPoint,
                                         std::array<double, 3> &endPoint)
{
    vtkSmartPointer<vtkPolyData> polyData;

    // Create an arrow.
    vtkNew<vtkArrowSource> arrowSource;
    arrowSource->SetShaftRadius(pdLength * .01);
    arrowSource->SetShaftResolution(20);
    arrowSource->SetTipLength(pdLength * .1);
    arrowSource->SetTipRadius(pdLength * .05);
    arrowSource->SetTipResolution(20);

    // Compute a basis
    std::array<double, 3> normalizedX;
    std::array<double, 3> normalizedY;
    std::array<double, 3> normalizedZ;

    // The X axis is a vector from start to end
    vtkMath::Subtract(endPoint.data(), startPoint.data(), normalizedX.data());
    double length = vtkMath::Norm(normalizedX.data());
    vtkMath::Normalize(normalizedX.data());

    // The Z axis is an arbitrary vector cross X
    vtkNew<vtkMinimalStandardRandomSequence> rng;
    rng->SetSeed(8775070);  // For testing.

    std::array<double, 3> arbitrary;
    for (auto i = 0; i < 3; ++i) {
        rng->Next();
        arbitrary[i] = rng->GetRangeValue(-10, 10);
    }
    vtkMath::Cross(normalizedX.data(), arbitrary.data(), normalizedZ.data());
    vtkMath::Normalize(normalizedZ.data());

    // The Y axis is Z cross X
    vtkMath::Cross(normalizedZ.data(), normalizedX.data(), normalizedY.data());
    vtkNew<vtkMatrix4x4> matrix;

    // Create the direction cosine matrix
    matrix->Identity();
    for (auto i = 0; i < 3; i++) {
        matrix->SetElement(i, 0, normalizedX[i]);
        matrix->SetElement(i, 1, normalizedY[i]);
        matrix->SetElement(i, 2, normalizedZ[i]);
    }

    // Apply the transforms
    vtkNew<vtkTransform> transform;
    transform->Translate(startPoint.data());
    transform->Concatenate(matrix);
    transform->Scale(length, length, length);

    // Transform the polydata
    vtkNew<vtkTransformPolyDataFilter> transformPD;
    transformPD->SetTransform(transform);
    transformPD->SetInputConnection(arrowSource->GetOutputPort());
    transformPD->Update();
    polyData = transformPD->GetOutput();
    return polyData;
}
#define vsp vtkSmartPointer
class MyCamerCallback : public vtkCommand {
public:
    vtkRenderWindow *m_widget = nullptr;

    static MyCamerCallback *New() { return new MyCamerCallback; }

    virtual void Execute(vtkObject *caller, unsigned long, void *)
    {
        // set one camera;
        vtkCamera *camera = static_cast<vtkCamera *>(caller);

        auto renders = m_widget->GetRenderers();
        int n = renders->GetNumberOfItems();
        auto render = renders->GetFirstRenderer();
        double allBounds[6] = {0};
        allBounds[0] = allBounds[2] = allBounds[4] = VTK_DOUBLE_MAX;
        allBounds[1] = allBounds[3] = allBounds[5] = -VTK_DOUBLE_MAX;
        bool enabled = false;
        for (int i = 0; i < n; i++) {
            auto render = static_cast<vtkRenderer *>(renders->GetItemAsObject(i));
            double bounds[6] = {0};
            render->ComputeVisiblePropBounds(bounds);
            if (vtkMath::AreBoundsInitialized(bounds)) {
                enabled = true;

                if (bounds[0] < allBounds[0]) {
                    allBounds[0] = bounds[0];
                }
                if (bounds[1] > allBounds[1]) {
                    allBounds[1] = bounds[1];
                }
                if (bounds[2] < allBounds[2]) {
                    allBounds[2] = bounds[2];
                }
                if (bounds[3] > allBounds[3]) {
                    allBounds[3] = bounds[3];
                }
                if (bounds[4] < allBounds[4]) {
                    allBounds[4] = bounds[4];
                }
                if (bounds[5] > allBounds[5]) {
                    allBounds[5] = bounds[5];
                }
            }
        }
        if (camera && render && enabled) {
            render->ResetCameraClippingRange(allBounds);
        }
    }
};

int main(int, char *[])
{
    // Read a stl file.
    vtkNew<vtkSphereSource> sppp;
    sppp->SetRadius(10);
    sppp->SetThetaResolution(50);
    sppp->SetPhiResolution(50);
    sppp->Update();
    vtkSmartPointer<vtkPolyData> input1 = vtkSmartPointer<vtkPolyData>::New();
    /*vtkSmartPointer<vtkSTLReader> reader1 = vtkSmartPointer<vtkSTLReader>::New();
    QString path = "D:\\kasystem\\build\\bin\\x64\\Release\\prosthesisdata\\MeshSTL\\Femur_left.stl";
    reader1->SetFileName(path.toStdString().c_str());
    reader1->Update();*/
    input1->DeepCopy(sppp->GetOutput());

    // Create a mapper and actor
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(sppp->GetOutputPort());
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetOpacity(.99);

    // Create a renderer, render window, and interactor
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderer> renderer2 = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->Render();
    renderWindow->SetWindowName("PointPicker");
    renderWindow->AddRenderer(renderer);
    renderWindow->AddRenderer(renderer2);

    vtkSmartPointer<vtkPointPicker> pointPicker = vtkSmartPointer<vtkPointPicker>::New();

    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
        vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindowInteractor->SetPicker(pointPicker);
    renderWindowInteractor->SetRenderWindow(renderWindow);
    vtkSmartPointer<vtkInteractorStyleTrackballCamera> style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    renderWindowInteractor->SetInteractorStyle(style);
    // Add the actor to the scene
    renderer->AddActor(actor);
    //renderer->SetBackground(0, 0, 0);
    renderer->SetLayer(0);
    vtkSmartPointer<MyCamerCallback> cameraCallback = vtkSmartPointer<MyCamerCallback>::New();
    cameraCallback->m_widget = renderWindow;
    renderer->GetActiveCamera()->AddObserver(vtkCommand::ModifiedEvent, cameraCallback);
    //renderer->PreserveDepthBufferOn();
    /*vtkNew<vtkSphereSource> s;
    s->SetCenter(reader1->GetOutput()->GetCenter());
    s->SetRadius(2);
    s->Update();*/
    vtkNew<vtkPolyDataMapper> mm;
    //mm->SetInputData(s->GetOutput());
    vtkNew<vtkActor> sp;
    sp->SetMapper(mm);
    sp->GetProperty()->SetColor(1, 0, 0);
    renderer2->AddActor(sp);
    //renderer2->SetBackground(0, 0, 0);
    renderer2->SetLayer(1);
    renderer2->SetActiveCamera(renderer->GetActiveCamera());
    renderer2->GetActiveCamera()->SetFocalPoint(sppp->GetOutput()->GetCenter());

    /*renderer2->PreserveDepthBufferOn();
    renderer2->EraseOff();
    renderer2->SetUseDepthPeeling(1);
    renderer2->SetOcclusionRatio(0.2);*/
    // Render and interact
    double l{2};
    auto x = CreateArrow(l, std::array<double, 3>{0, 0, 0}, std::array<double, 3>{1, 0, 0});
    auto y = CreateArrow(l, std::array<double, 3>{0, 0, 0}, std::array<double, 3>{0, 1, 0});
auto z = CreateArrow(l, std::array<double, 3>{0, 0, 0}, std::array<double, 3>{0, 0, 1});
    vtkNew<vtkAppendPolyData> ap;
ap->AddInputData(x);
    ap->AddInputData(y);
ap->AddInputData(z);
    ap->Update();
    mm->SetInputData(ap->GetOutput());
    renderWindow->SetNumberOfLayers(2);
    renderWindow->Render();
    renderWindowInteractor->Start();
    return EXIT_SUCCESS;
}