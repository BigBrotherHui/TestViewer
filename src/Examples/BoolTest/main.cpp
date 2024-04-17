#include "vtkProperty.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "vtkSphereSource.h"
#include "vtkMatrix4x4.h"
#include "vtkGlyph3D.h"
#include "vtkInteractorStyleJoystickActor.h"
#include "vtkTextActor.h"
#include "vtkCommand.h"
#include <ostream>
#include <vtkAutoInit.h>
#include <vtkSTLReader.h>
#include <vtkPlaneSource.h>
#include <vtkLinearExtrusionFilter.h>
#include <Eigen/Eigen>
#include <vtkPolyLine.h>
#include <vtkCellData.h>
#include <vtkInteractorStyleTrackballActor.h>
#include <vtkBooleanOperationPolyDataFilter.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);//Failed getting the TextRenderer instance!
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
VTK_MODULE_INIT(vtkRenderingContextOpenGL2);


int main()
{
    vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
    reader->SetFileName("D:\\kasystem\\build\\bin\\x64\\Release\\prosthesisdata\\MeshSTL\\Femur_left.stl");
    reader->Update();

    vtkNew<vtkSphereSource> sp;
    sp->SetRadius(10);
    sp->SetCenter(reader->GetOutput()->GetCenter());

   vtkMatrix4x4 *matrix0 = vtkMatrix4x4::New();
   vtkMatrix4x4 *matrix1 = vtkMatrix4x4::New();

   vtkSmartPointer<vtkBooleanOperationPolyDataFilter> filter =
       vtkSmartPointer<vtkBooleanOperationPolyDataFilter>::New();
   filter->SetOperationToDifference();
   filter->SetInputConnection(1, reader->GetOutputPort());
   filter->SetInputConnection(0, sp->GetOutputPort());

   vtkPolyDataMapper *mapper1 = vtkPolyDataMapper::New();
   mapper1->SetInputConnection(reader->GetOutputPort());
   vtkActor *actor1 = vtkActor::New();
   actor1->SetMapper(mapper1);
   //actor1->GetProperty()->BackfaceCullingOn();
   actor1->SetUserMatrix(matrix0);
   actor1->GetProperty()->SetOpacity(.2);
   vtkPolyDataMapper *mapper2 = vtkPolyDataMapper::New();

   mapper2->SetInputConnection(sp->GetOutputPort());

   vtkActor *actor2 = vtkActor::New();
   //mapper2->SetScalarModeToUseCellData();
   actor2->SetMapper(mapper2);
   //actor2->GetProperty()->BackfaceCullingOn();
   actor2->SetUserMatrix(matrix1);

   vtkPolyDataMapper *mapper3 = vtkPolyDataMapper::New();
   mapper3->SetInputConnection(filter->GetOutputPort());
   vtkActor *actor3 = vtkActor::New();
   actor3->SetMapper(mapper3);
   actor3->GetProperty()->SetColor(1,0,0);
   //actor3->GetProperty()->SetLineWidth(3.0);

   vtkRenderer *ren = vtkRenderer::New();
   ren->AddActor(actor1);
   ren->AddActor(actor2);
   ren->AddActor(actor3);
   ren->SetBackground(0.5,0.5,0.5);

   vtkRenderWindow *renWin = vtkRenderWindow::New();
   renWin->AddRenderer(ren);

   vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();
   iren->SetRenderWindow(renWin);
   vtkNew<vtkInteractorStyleTrackballActor> style;
   iren->SetInteractorStyle(style);
   renWin->Render();
   iren->Start(); 
 
   matrix0->Delete();
   matrix1->Delete();
   //mapper1->Delete();
   mapper2->Delete();
   //mapper3->Delete();
   //actor1->Delete();
   actor2->Delete();
   //actor3->Delete();
   ren->Delete();
   //cbCollide->Delete();
   renWin->Delete();
   style->Delete();
   iren->Delete();
}