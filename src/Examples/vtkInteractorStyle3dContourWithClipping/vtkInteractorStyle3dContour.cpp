#include "vtkInteractorStyle3dContour.h"
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkKdTreePointLocator.h>
#include <vtkPropPicker.h>
#include <vtkSmartPointer.h>
#include <vtkDijkstraGraphGeodesicPath.h>
#include <vtkProperty.h>
#include <vtkConnectivityFilter.h>
#include <vtkGeometryFilter.h>
#include <vtkAppendPolyData.h>
#include <vtkCleanPolyData.h>

#include <vtkProperty2D.h>
#include <vtkTextProperty.h>


#include <vtkSplineFilter.h>
#include <vtkParametricSpline.h>
#include <vtkParametricFunctionSource.h>
#include <vtkKochanekSpline.h>

#include "CutAlongPolyLineFilter.h"

vtkStandardNewMacro(vtkInteractorStyle3dContour);

using namespace std;

void vtkInteractorStyle3dContour::OnLeftButtonDown()
{
	
	AbstractInteractorStyle3D::OnLeftButtonDown();
}

void vtkInteractorStyle3dContour::OnKeyPress()
{
	std::string key = this->Interactor->GetKeySym();
	cout << key << endl;
	if (key == "A") {
		DropSpheres();
		cout << "DropSpheres" << endl;
	}
	else if (key == "W") {
		ClearSpheres();
		//ClearLinks();
		cout << "ClearSpheres" << endl;
	}
	else if (key == "S") {
		LinkSpheres();
		cout << "LinkSpheres" << endl;
	}
	else if (key == "X") {
		SmoothLinks();
		cout << "SmoothLinks" << endl;
	}
	else if (key == "D") {
		CutAlongLinks();
		cout << "CutAlongLinks" << endl;
	}
	else if (key == "E") {
		if (m_linkActorAppendedDownSampled != nullptr) {
			this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor(m_linkActorAppendedDownSampled);
		}

		cout << "Remove smoothLInks" << endl;
	}
	else if (key == "Return") {
		m_sliderWidget->SetInteractor(this->Interactor);
		m_sliderWidget->EnabledOn();
		this->Interactor->Render();
	}

	//AbstractInteractorStyle3D::OnKeyPress();
}

vtkInteractorStyle3dContour::vtkInteractorStyle3dContour()
{

	vtkSmartPointer<vtkSliderRepresentation2D> sliderRep =
		vtkSmartPointer<vtkSliderRepresentation2D>::New();

	sliderRep->SetMinimumValue(0.0);
	sliderRep->SetMaximumValue(30);
	sliderRep->SetValue(5.0);
	sliderRep->SetTitleText("Sample Rate");

	// Set color properties:
	// Change the color of the knob that slides
	sliderRep->GetSliderProperty()->SetColor(1, 0, 0);//red

													  // Change the color of the text indicating what the slider controls
	sliderRep->GetTitleProperty()->SetColor(1, 0, 0);//red

													 // Change the color of the text displaying the value
	sliderRep->GetLabelProperty()->SetColor(1, 0, 0);//red

													 // Change the color of the knob when the mouse is held on it
	sliderRep->GetSelectedProperty()->SetColor(0, 1, 0);//green

														// Change the color of the bar
	sliderRep->GetTubeProperty()->SetColor(1, 1, 0);//yellow

													// Change the color of the ends of the bar
	sliderRep->GetCapProperty()->SetColor(1, 1, 0);//yellow

	sliderRep->GetPoint1Coordinate()->SetCoordinateSystemToDisplay();
	sliderRep->GetPoint1Coordinate()->SetValue(40, 40);
	sliderRep->GetPoint2Coordinate()->SetCoordinateSystemToDisplay();
	sliderRep->GetPoint2Coordinate()->SetValue(200, 40);

	m_sliderWidget =
		vtkSmartPointer<vtkSliderWidget>::New();
	m_sliderWidget->SetInteractor(this->Interactor);
	m_sliderWidget->SetRepresentation(sliderRep);
	m_sliderWidget->SetAnimationModeToAnimate();

	vtkSmartPointer<vtkSliderCallback> callback =
		vtkSmartPointer<vtkSliderCallback>::New();
	callback->style = this;
	m_sliderWidget->AddObserver(vtkCommand::InteractionEvent, callback);
}


vtkInteractorStyle3dContour::~vtkInteractorStyle3dContour()
{
}

void vtkInteractorStyle3dContour::DropSpheres()
{

	std::cout << "Picking pixel: " << this->Interactor->GetEventPosition()[0] << " " << this->Interactor->GetEventPosition()[1] << std::endl;

	vtkSmartPointer<vtkPropPicker> propPicker = vtkSmartPointer<vtkPropPicker>::New();
	propPicker->Pick(this->Interactor->GetEventPosition()[0],
		this->Interactor->GetEventPosition()[1],
		0,  // always zero.
		this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
	double picked[3];
	propPicker->GetPickPosition(picked);
	std::cout << "Picked value: " << picked[0] << " " << picked[1] << " " << picked[2] << std::endl;

	if (propPicker->GetActor() == NULL) {
		return;
	}
	m_mainActor = propPicker->GetActor();

	vtkSmartPointer<vtkKdTreePointLocator> kDTree =
		vtkSmartPointer<vtkKdTreePointLocator>::New();
	kDTree->SetDataSet(m_mainActor->GetMapper()->GetInput());
	kDTree->BuildLocator();
	vtkIdType id = kDTree->FindClosestPoint(picked);


	cout << "sphere center id: " << id << endl;
	// add a point to the list
	m_sphereCenterPointIds.push_back(id);

	m_mainActor->GetMapper()->GetInput()->GetPoint(id, picked);

	//Create a sphere
	vtkSmartPointer<vtkSphereSource> sphereSource =
		vtkSmartPointer<vtkSphereSource>::New();
	sphereSource->SetCenter(picked);
	sphereSource->SetRadius(1);


	//Create a mapper and actor
	vtkSmartPointer<vtkPolyDataMapper> mapper =
		vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(sphereSource->GetOutputPort());

	vtkSmartPointer<vtkActor> actor =
		vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->SetColor(0, 0, 1);

	// useless
	actor->PickableOff();

	this->GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(actor);
	m_sphereActors.push_back(actor);


	this->Interactor->Render();

}

void vtkInteractorStyle3dContour::ClearSpheres()
{
	m_sphereCenterPointIds.clear();

	for (list<vtkSmartPointer<vtkActor>>::const_iterator cit = m_sphereActors.cbegin();
		cit != m_sphereActors.cend(); ++cit) {
		if ((*cit) != nullptr) {
			this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor(*cit);
		}
	}
	m_sphereActors.clear();
	this->Interactor->Render();
}
#include <vtkXMLPolyDataWriter.h>
void vtkInteractorStyle3dContour::LinkSpheres()
{
	ClearLinks();

	if (m_mainActor == nullptr || m_sphereCenterPointIds.size() < 2 ) {
		return;
	}
	//Append the multi links
	vtkSmartPointer<vtkAppendPolyData> appendFilter =
		vtkSmartPointer<vtkAppendPolyData>::New();


	for (list<vtkIdType>::const_iterator cit1 = m_sphereCenterPointIds.cbegin(), cit2 = ++m_sphereCenterPointIds.cbegin();
		cit1 != (m_sphereCenterPointIds.cend()); ++cit1, ++cit2) {
		cout << "sphere center id: " << *cit1 << endl;

		vtkSmartPointer<vtkDijkstraGraphGeodesicPath> dijkstra =
			vtkSmartPointer<vtkDijkstraGraphGeodesicPath>::New();
		dijkstra->SetInputData(m_mainActor->GetMapper()->GetInput());

		if (cit1 == --m_sphereCenterPointIds.cend()) {
			dijkstra->SetStartVertex(*cit1);
			dijkstra->SetEndVertex(*m_sphereCenterPointIds.cbegin());
			dijkstra->Update();

			// for not out of index 
			--cit2;
		}
		else {
			dijkstra->SetStartVertex(*cit1);
			dijkstra->SetEndVertex(*cit2);
			dijkstra->Update();
		}

		// multiple actor method
		vtkSmartPointer<vtkPolyDataMapper> pathMapper =
			vtkSmartPointer<vtkPolyDataMapper>::New();
		pathMapper->SetInputConnection(dijkstra->GetOutputPort());
		pathMapper->Update();

		vtkSmartPointer<vtkActor> linkActor =
			vtkSmartPointer<vtkActor>::New();
		linkActor->SetMapper(pathMapper);
		linkActor->GetProperty()->SetColor(1, 0, 0); // Red
		linkActor->GetProperty()->SetLineWidth(4);

		m_linkActors.push_back(linkActor);

		// single actor method
		//appendFilter->SetInputData(dijkstra->GetOutput());

		appendFilter->AddInputConnection(dijkstra->GetOutputPort());
	}
	appendFilter->Update();

	//// Write the file
	//vtkSmartPointer<vtkXMLPolyDataWriter> writer =
	//	vtkSmartPointer<vtkXMLPolyDataWriter>::New();
	//writer->SetFileName("test.vtp");
	//writer->SetInputData(appendFilter->GetOutput());
	//writer->Write();




	vtkSmartPointer<vtkPolyDataMapper> linkMapper =
		vtkSmartPointer<vtkPolyDataMapper>::New();
	linkMapper->SetInputConnection(appendFilter->GetOutputPort());
	linkMapper->Update();


	m_linkActorAppended =
		vtkSmartPointer<vtkActor>::New();
	m_linkActorAppended->SetMapper(linkMapper);
	m_linkActorAppended->GetProperty()->SetColor(1, 0, 0); // Red
	m_linkActorAppended->GetProperty()->SetLineWidth(4);
	this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(m_linkActorAppended);

	this->Interactor->Render();

}

void vtkInteractorStyle3dContour::ClearLinks()
{
	this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor(m_linkActorAppended);

	for (list<vtkSmartPointer<vtkActor>>::const_iterator cit = m_linkActors.cbegin();
		cit != m_linkActors.cend(); ++cit) {
		if ((*cit) != nullptr) {
			this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor(*cit);
		}
	}
	m_linkActors.clear();
	this->Interactor->Render();

}

void vtkInteractorStyle3dContour::CutAlongLinks()
{
	if (m_mainActor == nullptr) {
		return;
	}
	vtkSmartPointer<CutAlongPolyLineFilter> cutter = 
		vtkSmartPointer<CutAlongPolyLineFilter>::New();

	cutter->SetInputData(0, m_mainActor->GetMapper()->GetInput());
	cutter->SetInputData(1, m_linkActorAppendedDownSampled->GetMapper()->GetInput());
	cutter->SetClipTolerance(0.1);
	cutter->Update();

	vtkSmartPointer<vtkConnectivityFilter> conn = vtkSmartPointer<vtkConnectivityFilter>::New();
	conn->SetInputConnection(cutter->GetOutputPort());
	conn->ColorRegionsOn();
	//conn->SetExtractionModeToAllRegions();
	conn->SetExtractionModeToLargestRegion();
	conn->Update();

	vtkSmartPointer<vtkGeometryFilter> geom = vtkSmartPointer<vtkGeometryFilter>::New();
	geom->SetInputConnection(conn->GetOutputPort());
	geom->Update();

	this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor(m_mainActor);

	m_mainActor = vtkSmartPointer<vtkActor>::New();
	m_mainActor->SetMapper(vtkSmartPointer<vtkPolyDataMapper>::New());
	m_mainActor->GetMapper()->SetInputConnection(geom->GetOutputPort());
	m_mainActor->GetMapper()->Update();
	this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(m_mainActor);

	
	this->Interactor->Render();
}

void vtkInteractorStyle3dContour::SmoothLinks()
{
	if (m_mainActor == nullptr || m_sphereCenterPointIds.size() < 2) {
		return;
	}
	if (m_linkActorAppendedDownSampled == nullptr) {
		m_linkActorAppendedDownSampled =
			vtkSmartPointer<vtkActor>::New();
		m_linkActorAppendedDownSampled->SetMapper(vtkSmartPointer<vtkPolyDataMapper>::New());
		m_linkActorAppendedDownSampled->GetProperty()->SetColor(0, 1, 0); // Green
		m_linkActorAppendedDownSampled->GetProperty()->SetLineWidth(4);
	}
	this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(m_linkActorAppendedDownSampled);

	// Remove any duplicate points.
	vtkSmartPointer<vtkCleanPolyData> cleanFilter =
		vtkSmartPointer<vtkCleanPolyData>::New();
	cleanFilter->SetInputData(m_linkActorAppended->GetMapper()->GetInput());
	cleanFilter->ToleranceIsAbsoluteOn();
	cleanFilter->SetAbsoluteTolerance(m_clearFilterTolerance);
	cleanFilter->Update();


	vtkSmartPointer<vtkSplineFilter> sf =
		vtkSmartPointer<vtkSplineFilter>::New();
	sf->SetInputConnection(cleanFilter->GetOutputPort());
	sf->Update();

	m_linkActorAppendedDownSampled->GetMapper()->SetInputConnection(sf->GetOutputPort());
	m_linkActorAppendedDownSampled->GetMapper()->Update();
	this->Interactor->Render();


}
