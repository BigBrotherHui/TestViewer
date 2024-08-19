
#include <vtkAngleRepresentation2D.h>
#include <vtkAngleWidget.h>
#include <vtkAutoInit.h>
#include <vtkBiDimensionalRepresentation2D.h>
#include <vtkBiDimensionalWidget.h>
#include <vtkCommand.h>
#include <vtkDistanceRepresentation.h>
#include <vtkDistanceWidget.h>
#include <vtkImageActor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkJPEGReader.h>
#include <vtkLeaderActor2D.h>
#include <vtkProperty2D.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkDICOMImageReader.h>
#include "splineWidget.h"
VTK_MODULE_INIT(vtkInteractionStyle)
VTK_MODULE_INIT(vtkRenderingFreeType)
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2)


int main()
{
    vtkSmartPointer<vtkDICOMImageReader> reader = vtkSmartPointer<vtkDICOMImageReader>::New();
    reader->SetFileName("D:/Images/dicom_image/CT0013.dcm");
    reader->Update();

    vtkSmartPointer<vtkImageActor> imgActor = vtkSmartPointer<vtkImageActor>::New();
    imgActor->SetInputData(reader->GetOutput());

    vtkSmartPointer<vtkRenderer> render = vtkSmartPointer<vtkRenderer>::New();
    render->AddActor(imgActor);
    render->SetBackground(0, 0, 0);
    render->ResetCamera();

    vtkSmartPointer<vtkRenderWindow> rw = vtkSmartPointer<vtkRenderWindow>::New();
    rw->AddRenderer(render);
    rw->SetWindowName("MeasurementDistanceApp");
    rw->SetSize(320, 320);
    rw->Render();

    vtkSmartPointer<vtkRenderWindowInteractor> rwi = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    rwi->SetRenderWindow(rw);

    vtkSmartPointer<vtkInteractorStyleImage> style = vtkSmartPointer<vtkInteractorStyleImage>::New();
    rwi->SetInteractorStyle(style);

    vtkSmartPointer<splineWidget> distanceWidget = vtkSmartPointer<splineWidget>::New();
    distanceWidget->CreateDefaultRepresentation();
    distanceWidget->SetInteractor(rwi);
    distanceWidget->On();

    rw->Render();
    rwi->Initialize();
    rwi->Start();

    return 0;
}