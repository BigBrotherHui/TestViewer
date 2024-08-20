#include <vtkSmartPointer.h>
#include <vtkWidgetCallbackMapper.h>
#include <vtkCommand.h>
#include <vtkWidgetEvent.h>
#include <vtkObjectFactory.h>
#include <vtkActor.h>
#include <vtkBorderRepresentation.h>
#include <vtkBorderWidget.h>
#include <vtkEllipseRepresentation.h>
#include <vtkEllipseWidget.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>
#include <vtkActor2D.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkActor2D.h>
#include "vtkAutoInit.h" 
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
VTK_MODULE_INIT(vtkRenderingFreeType);
VTK_MODULE_INIT(vtkInteractionStyle);

int main(int, char *[])
{
	// Sphere
	vtkSmartPointer<vtkSphereSource> sphereSource =
		vtkSmartPointer<vtkSphereSource>::New();
	sphereSource->SetRadius(0.1);
	sphereSource->SetCenter(0, 0, 0);
	sphereSource->Update();

	vtkSmartPointer<vtkPolyDataMapper> mapper =
		vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(sphereSource->GetOutputPort());

	vtkSmartPointer<vtkActor> actor =
		vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	// A renderer and render window
	vtkSmartPointer<vtkRenderer> renderer =
		vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renderWindow =
		vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);

	// An interactor
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);

	vtkSmartPointer<vtkEllipseWidget> ellipseWidget =
		vtkSmartPointer<vtkEllipseWidget>::New();
	ellipseWidget->SetInteractor(renderWindowInteractor);
	ellipseWidget->CreateDefaultRepresentation();
	ellipseWidget->SelectableOff();
	//ellipseWidget->GetEllipseRepresentation()->MovingOff();

	// Add the actors to the scene
	renderer->AddActor(actor);
	//vtkEllipseRepresentation* representation = static_cast<vtkEllipseRepresentation*>(ellipseWidget->GetRepresentation());

	//auto actor2D= vtkSmartPointer<vtkActor2D>::New();
	//actor2D->SetMapper(representation->EWMapper);
	//renderer->AddActor2D(actor2D);

	// Render an image (lights and cameras are created automatically)
	renderWindowInteractor->Initialize();
	renderWindow->Render();
	ellipseWidget->On();

	renderer->ResetCamera();
	// Begin mouse interaction
	renderWindowInteractor->Start();

	return EXIT_SUCCESS;
}