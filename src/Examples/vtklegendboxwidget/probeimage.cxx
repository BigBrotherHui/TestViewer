
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
#include "vtklegendboxwidget.h"
#include <vtkLegendBoxActor.h>
#include "vtklegendboxrepresentation.h"
#include <vtkSphereSource.h>
#include <vtkCylinderSource.h>
#include "vtkROIWidget.h"
#include "vtkContourWidgetSeries.h"
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

    //vtkSmartPointer<vtkLegendBoxWidget> distanceWidget = vtkSmartPointer<vtkLegendBoxWidget>::New();
    //
    //vtkLegendBoxActor* legend = vtkLegendBoxActor::New();
    //legend->SetNumberOfEntries(3);
    //double color0[3]{1, 0, 0}, color1[3]{0, 1, 0}, color2[3]{0, 0, 1};
    //auto sp1 = vtkSmartPointer<vtkSphereSource>::New();
    //sp1->Update();
    //auto sp2 = vtkSmartPointer<vtkSphereSource>::New();
    //sp2->Update();
    //auto sp3 = vtkSmartPointer<vtkSphereSource>::New();
    //sp3->Update();
    //legend->SetEntry(0,sp1->GetOutput() , "sphere1",color0);
    //legend->SetEntry(1, sp2->GetOutput(), "sphere2", color1);
    //legend->SetEntry(2, sp3->GetOutput(), "sphere3", color2);
    //distanceWidget->GetLegendBoxRepresentation()->SetLegendBoxActor(legend);
    //distanceWidget->SetInteractor(rwi);
    //distanceWidget->On();

     vtkSmartPointer<vtkContourWidgetSeries> distanceWidget = vtkSmartPointer<vtkContourWidgetSeries>::New();
     distanceWidget->SetInteractor(rwi);
     distanceWidget->On();

    rw->Render();
    rwi->Initialize();
    rwi->Start();

    return 0;
}