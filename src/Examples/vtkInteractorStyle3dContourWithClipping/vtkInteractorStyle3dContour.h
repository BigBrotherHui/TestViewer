#pragma once


#include <list>

#include <vtkPoints.h>


#include <vtkSliderWidget.h>
#include <vtkSliderRepresentation2D.h>
#include <vtkCommand.h>

#include "AbstractInteractorStyle3D.h"



class vtkSliderCallback;

class vtkInteractorStyle3dContour :
	public AbstractInteractorStyle3D
{
public:
	vtkTypeMacro(vtkInteractorStyle3dContour, AbstractInteractorStyle3D);
	static vtkInteractorStyle3dContour* New();

	virtual void OnLeftButtonDown();
	virtual void OnKeyPress();


protected:
	vtkInteractorStyle3dContour();
	~vtkInteractorStyle3dContour();

	virtual void DropSpheres();
	virtual void ClearSpheres();
	virtual void LinkSpheres();
	virtual void ClearLinks();
	virtual void CutAlongLinks();
	virtual void SmoothLinks();


	std::list<vtkSmartPointer<vtkActor>> m_sphereActors;
	std::list<vtkSmartPointer<vtkActor>> m_linkActors;
	vtkSmartPointer<vtkActor> m_linkActorAppended = nullptr;
	vtkSmartPointer<vtkActor> m_linkActorAppendedDownSampled = nullptr;
	std::list<vtkIdType> m_sphereCenterPointIds;
	vtkSmartPointer<vtkActor> m_mainActor = nullptr;

	vtkSmartPointer<vtkSliderWidget> m_sliderWidget = nullptr;


	int m_clearFilterTolerance = 1;

	friend class vtkSliderCallback;

};

// This does the actual work.
// Callback for the interaction
class vtkSliderCallback : public vtkCommand
{
public:
	static vtkSliderCallback *New()
	{
		return new vtkSliderCallback;
	}
	virtual void Execute(vtkObject *caller, unsigned long, void*)
	{
		vtkSliderWidget *sliderWidget =
			reinterpret_cast<vtkSliderWidget*>(caller);
		style->m_clearFilterTolerance = sliderWidget->GetSliderRepresentation()->GetValue();
		style->SmoothLinks();
	}
	vtkSliderCallback() :style(0) {};
	vtkInteractorStyle3dContour* style = nullptr;
};