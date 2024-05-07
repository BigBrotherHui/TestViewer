/*
Author:		Wong, Matthew Lun
Date:		16th, June 2016
Occupation:	Chinese University of Hong Kong,
Department of Imaging and Inteventional Radiology,
Junior Research Assistant


The abstract interactor class used in medical viewers.
This class is the ultimate parent of all interactor classes.


Wong Matthew Lun
Copyright (C) 2016
*/

#ifndef ABSTRACT_INTERACTOR_STYLE_H
#define ABSTRACT_INTERACTOR_STYLE_H

#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkSmartPointer.h>
#include <vtkPropPicker.h>
#include <vtkObjectFactory.h>
//#include "MyViewer.h"


class MyViewer;

class AbstractInteractorStyle
{
public:
	void SetViewers(MyViewer**);
	virtual void SetOrientation(int);
	
	MyViewer**	GetViewers();
	MyViewer*	GetViewer(int);
	int GetOrientation();

	vtkActor* PickActor(int x, int y);

protected:
	AbstractInteractorStyle();
	~AbstractInteractorStyle();

	virtual void OnLeftButtonDown();
	virtual void OnRightButtonDown();
	virtual void OnMiddleButtonDown();
	virtual void OnLeftButtonUp();
	virtual void OnRightButtonUp();
	virtual void OnMiddleButtonUp();

	MyViewer*					m_viewers[4];
	
	int			m_orientation;

	bool m_rightFunctioning;
	bool m_leftFunctioning;
	bool m_middleFunctioning;
};

#endif //ABSTRACT_INTERACTOR_STYLE_H