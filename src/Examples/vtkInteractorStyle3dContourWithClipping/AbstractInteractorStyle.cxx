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

#include "AbstractInteractorStyle.h"

AbstractInteractorStyle::AbstractInteractorStyle() {
	m_rightFunctioning = false;
	m_leftFunctioning = false;
	m_middleFunctioning = false;
}

AbstractInteractorStyle::~AbstractInteractorStyle() {

}

//void AbstractInteractorStyle::SetViewers(MyViewer** viewers)
//{
//	for (int i = 0; i < 4; i++)
//	{
//		m_viewers[i] = viewers[i];
//	}
//}
//
//MyViewer** AbstractInteractorStyle::GetViewers()
//{
//	return m_viewers;
//}
//
//MyViewer * AbstractInteractorStyle::GetViewer(int i)
//{
//	return m_viewers[i];
//}

int AbstractInteractorStyle::GetOrientation()
{
	return m_orientation;
}

void AbstractInteractorStyle::SetOrientation(int i)
{
	m_orientation = i;
}

void AbstractInteractorStyle::OnLeftButtonDown()
{
	m_leftFunctioning = true;
}

void AbstractInteractorStyle::OnRightButtonDown()
{
	m_rightFunctioning = true;
}

void AbstractInteractorStyle::OnMiddleButtonDown()
{
	m_middleFunctioning = true;
}

void AbstractInteractorStyle::OnLeftButtonUp()
{
	m_leftFunctioning = false;
}

void AbstractInteractorStyle::OnRightButtonUp()
{
	m_rightFunctioning = false;
}

void AbstractInteractorStyle::OnMiddleButtonUp()
{
	m_middleFunctioning = false;
}

vtkActor * AbstractInteractorStyle::PickActor(int x, int y)
{
	vtkSmartPointer<vtkPropPicker> picker = vtkSmartPointer<vtkPropPicker>::New();
	//if (this->m_viewers[m_orientation]->GetDataRenderer()) {
	//	picker->Pick(x, y, 0, this->m_viewers[m_orientation]->GetDataRenderer());
	//}
	if (picker->GetActor()) {
		return picker->GetActor();
	}
	else {
		return nullptr;
	}
}