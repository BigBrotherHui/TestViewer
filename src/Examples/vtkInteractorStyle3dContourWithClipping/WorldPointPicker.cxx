#include <vtkSmartPointer.h>
#include <vtkRendererCollection.h>
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkActor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkObjectFactory.h>
#include <vtkSTLReader.h>

#include "vtkInteractorStyle3dContour.h"
#include "vtkInteractorStyle3dContour2.h"
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);
// Define interaction style
class MouseInteractorStyle : public vtkInteractorStyleTrackballCamera
{
  public:
    static MouseInteractorStyle* New();
    vtkTypeMacro(MouseInteractorStyle, vtkInteractorStyleTrackballCamera);
 
    virtual void OnLeftButtonDown() 
    {
      
      std::cout << "Picking pixel: " << this->Interactor->GetEventPosition()[0] << " " << this->Interactor->GetEventPosition()[1] << std::endl;
      this->Interactor->GetPicker()->Pick(this->Interactor->GetEventPosition()[0], 
                         this->Interactor->GetEventPosition()[1], 
                         0,  // always zero.
                         this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
      double picked[3];
      this->Interactor->GetPicker()->GetPickPosition(picked);
      std::cout << "Picked value: " << picked[0] << " " << picked[1] << " " << picked[2] << std::endl;
      // Forward events
      vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
    }
 
};
vtkStandardNewMacro(MouseInteractorStyle);

int main(int, char *[])
{
  vtkSmartPointer<vtkSphereSource> sphereSource = 
    vtkSmartPointer<vtkSphereSource>::New();
  sphereSource->Update();
  sphereSource->SetRadius(10);
  sphereSource->SetPhiResolution(100);
  sphereSource->SetThetaResolution(100);
	std::string inputFileName = "D:/Images/femur_R.stl";
	//std::string inputFileName = "C:\\Users\\jieji\\Desktop\\ccode\\WorldPointPicker\\shimingzhong.stl";


	vtkSmartPointer<vtkSTLReader> reader =
		vtkSmartPointer<vtkSTLReader>::New();
	reader->SetFileName(inputFileName.c_str());
	reader->Update();


  // Create a mapper and actor
  vtkSmartPointer<vtkPolyDataMapper> mapper = 
    vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection(reader->GetOutputPort());
  //mapper->SetInputConnection(sphereSource->GetOutputPort());
  vtkSmartPointer<vtkActor> actor = 
    vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(mapper);
  // Create a renderer, render window, and interactor
  vtkSmartPointer<vtkRenderer> renderer = 
    vtkSmartPointer<vtkRenderer>::New();
  vtkSmartPointer<vtkRenderWindow> renderWindow = 
    vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->AddRenderer(renderer);
  renderWindow->SetSize(1000, 1000);
  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = 
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  renderWindowInteractor->SetRenderWindow(renderWindow);
  
  //renderWindowInteractor->SetInteractorStyle(vtkSmartPointer<vtkInteractorStyle3dContour>::New());
  renderWindowInteractor->SetInteractorStyle(vtkSmartPointer<vtkInteractorStyle3dContour2>::New());
  
  // Add the actor to the scene
  renderer->AddActor(actor);
  renderer->SetBackground(0,0,0); // Background color white
  // Render and interact
  renderWindow->Render();
  renderWindowInteractor->Initialize();
  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}
