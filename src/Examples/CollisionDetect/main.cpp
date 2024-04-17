#include "vtkProperty.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "vtkSphereSource.h"
#include "vtkMatrix4x4.h"
#include "CollisionDetectionFilter.h"
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

VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);//Failed getting the TextRenderer instance!
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
VTK_MODULE_INIT(vtkRenderingContextOpenGL2);

class vtkCollisionCallback : public vtkCommand
{
public:
  static vtkCollisionCallback *New() 
    { return new vtkCollisionCallback; }

  void SetTextActor(vtkTextActor *txt)
    {
    this->TextActor = txt;
    }
  void SetRenderWindow(vtkRenderWindow *renWin)
    {
    this->RenWin = renWin;
    }

  virtual void Execute(vtkObject *caller, unsigned long, void*)
    {
      CollisionDetectionFilter *collide = reinterpret_cast<CollisionDetectionFilter *>(caller);
      if (collide->GetNumberOfContacts() > 0)
        {
        sprintf(this->TextBuff, "Number Of Contacts: %d", collide->GetNumberOfContacts());
        }
      else
        {
        sprintf(this->TextBuff, "No Contacts");
        }
      this->TextActor->SetInput(this->TextBuff);
      //this->RenWin->Render();
    }

protected:
  vtkTextActor *TextActor;
  vtkRenderWindow *RenWin;
  char TextBuff[128];
};

int main()
{
   //vtkFileOutputWindow *fout =  vtkFileOutputWindow::New();
   //  fout->SetFileName("squawks.txt");
   //  fout->SetInstance(fout);
  
    vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
    reader->SetFileName("D:\\kasystem\\build\\bin\\x64\\Release\\prosthesisdata\\MeshSTL\\Femur_left.stl");
    reader->Update();

    /*vtkNew<vtkPlaneSource> sp;
    Eigen::Vector3d tcutpoint{ reader->GetOutput()->GetCenter() };
    sp->SetOrigin(tcutpoint.data());
    sp->SetPoint1(Eigen::Vector3d(tcutpoint - 100 * Eigen::Vector3d::UnitX()).data());
    sp->SetPoint2(Eigen::Vector3d(tcutpoint + 100 * Eigen::Vector3d::UnitY()).data());
    sp->Update();*/
    vtkNew<vtkSphereSource> sp;
    sp->SetRadius(10);
    sp->SetCenter(reader->GetOutput()->GetCenter());
    //vtkSmartPointer<vtkLinearExtrusionFilter> ll = vtkSmartPointer<vtkLinearExtrusionFilter>::New();
    //ll->SetInputConnection(sp->GetOutputPort());
    //ll->SetExtrusionTypeToNormalExtrusion();
    //ll->SetVector(0, 0, 1);
    //ll->Update();

   vtkMatrix4x4 *matrix0 = vtkMatrix4x4::New();
   vtkMatrix4x4 *matrix1 = vtkMatrix4x4::New();

   CollisionDetectionFilter *collide = CollisionDetectionFilter::New();
   collide->SetInputConnection(0, sp->GetOutputPort());
   collide->SetMatrix(0, matrix0);
   collide->SetInputConnection(1, reader->GetOutputPort());
   collide->SetMatrix(1, matrix1);
   collide->SetBoxTolerance(0.0);
   collide->SetCellTolerance(0.0);
   collide->SetNumberOfCellsPerNode(2);
   collide->SetCollisionModeToAllContacts();
   float red[3]{255, 0, 0};
   collide->SetCollideCellsColor(red);
   collide->GenerateScalarsOn();

   vtkPolyDataMapper *mapper1 = vtkPolyDataMapper::New();
   mapper1->SetInputConnection(collide->GetOutputPort(0));
   vtkActor *actor1 = vtkActor::New();
   actor1->SetMapper(mapper1);
   //actor1->GetProperty()->BackfaceCullingOn();
   actor1->SetUserMatrix(matrix0);
   actor1->GetProperty()->SetOpacity(.2);
   vtkPolyDataMapper *mapper2 = vtkPolyDataMapper::New();

   mapper2->SetInputConnection(collide->GetOutputPort(1));

   vtkActor *actor2 = vtkActor::New();
   //mapper2->SetScalarModeToUseCellData();
   actor2->SetMapper(mapper2);
   //actor2->GetProperty()->BackfaceCullingOn();
   actor2->SetUserMatrix(matrix1);

   /*vtkSmartPointer<vtkPoints>polyline_pts = vtkSmartPointer<vtkPoints>::New();
   polyline_pts->DeepCopy(collide->GetContactsOutput()->GetPoints());

   vtkSmartPointer<vtkPolyLine> polyline = vtkSmartPointer<vtkPolyLine>::New();
   polyline->GetPointIds()->SetNumberOfIds(polyline_pts->GetNumberOfPoints());
   for (unsigned int i = 0; i < polyline_pts->GetNumberOfPoints(); i++)
   {
       polyline->GetPointIds()->SetId(i, i);
   }

   vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
   cells->InsertNextCell(polyline);

   vtkSmartPointer<vtkPolyData> polylineData = vtkSmartPointer<vtkPolyData>::New();
   polylineData->SetPoints(polyline_pts);
   polylineData->SetLines(cells);*/

   //vtkPolyDataMapper *mapper3 = vtkPolyDataMapper::New();
   //mapper3->SetInputConnection(collide->GetContactsOutputPort());//polylineData
   //mapper3->SetResolveCoincidentTopologyToPolygonOffset();
   //vtkActor *actor3 = vtkActor::New();
   //actor3->SetMapper(mapper3);
   //actor3->GetProperty()->SetColor(1,0,0);
   //actor3->GetProperty()->SetLineWidth(3.0);

   vtkTextActor *txt = vtkTextActor::New();

   vtkRenderer *ren = vtkRenderer::New();
   ren->AddActor(actor1);
   ren->AddActor(actor2);
   //ren->AddActor(actor3);
   ren->AddActor(txt);
   ren->SetBackground(0.5,0.5,0.5);

   vtkRenderWindow *renWin = vtkRenderWindow::New();
   renWin->AddRenderer(ren);

   vtkInteractorStyleTrackballActor *istyle = vtkInteractorStyleTrackballActor::New();
   vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();
   iren->SetRenderWindow(renWin);
   iren->SetInteractorStyle(istyle);

   vtkCollisionCallback *cbCollide = vtkCollisionCallback::New();
   cbCollide->SetTextActor(txt);
   cbCollide->SetRenderWindow(renWin);
   collide->AddObserver(vtkCommand::EndEvent, cbCollide);
   renWin->Render();
   iren->Start(); 
 
   matrix0->Delete();
   matrix1->Delete();
   collide->Delete();
   //mapper1->Delete();
   mapper2->Delete();
   //mapper3->Delete();
   //actor1->Delete();
   actor2->Delete();
   //actor3->Delete();
   txt->Delete();
   ren->Delete();
   //cbCollide->Delete();
   renWin->Delete();
   istyle->Delete();
   iren->Delete();
}