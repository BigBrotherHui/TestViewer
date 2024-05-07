#pragma once
#include "AbstractInteractorStyle3D.h"

#include <vtkContourWidget.h>
#include <vtkOrientedGlyphContourRepresentation.h>



class vtkInteractorStyle3dContour2 :
	public AbstractInteractorStyle3D
{
public:
	vtkTypeMacro(vtkInteractorStyle3dContour2, AbstractInteractorStyle3D);
	static vtkInteractorStyle3dContour2* New();



	void OnKeyPress();
	void OnLeftButtonDown();

protected:
	vtkInteractorStyle3dContour2();
	~vtkInteractorStyle3dContour2();

	void EnableContourWidget(bool flag);
	void CutAlongLinks();

	vtkSmartPointer<vtkContourWidget> m_contourWidget = nullptr;
	vtkSmartPointer<vtkOrientedGlyphContourRepresentation> m_contourWidgetRep = nullptr;
	bool m_enableFlag = false;

	vtkSmartPointer<vtkActor> m_mainActor = nullptr;

};

