/*=========================================================================

  Program:   Visualization Toolkit
  Module:    splineRepresentation.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "splineRepresentation.h"
#include "vtkBox.h"
#include "vtkCoordinate.h"
#include "vtkEventData.h"
#include "vtkHandleRepresentation.h"
#include "vtkInteractorObserver.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkWindow.h"

vtkCxxSetObjectMacro(splineRepresentation, HandleRepresentation, vtkHandleRepresentation);

//------------------------------------------------------------------------------
splineRepresentation::splineRepresentation()
{
  this->HandleRepresentation = nullptr;

  this->Tolerance = 5;
  this->Placed = 0;

  this->LabelFormat = new char[8];
  snprintf(this->LabelFormat, 8, "%s", "%-#6.3g");

  this->Scale = 1.0;
  this->RulerMode = 0;
  this->RulerDistance = 1.0;
  this->NumberOfRulerTicks = 5;
}

//------------------------------------------------------------------------------
splineRepresentation::~splineRepresentation()
{
  if (this->HandleRepresentation)
  {
    this->HandleRepresentation->Delete();
  }
  for (int i = 0; i < PointRepresentation.size(); ++i) PointRepresentation[i]->Delete();

  delete[] this->LabelFormat;
  this->LabelFormat = nullptr;
}

//------------------------------------------------------------------------------
void splineRepresentation::InstantiateHandleRepresentation()
{
    addPointRepresentation();
    addPointRepresentation();
}

vtkHandleRepresentation *splineRepresentation::GetPointRepresentation(int i)
{
    return PointRepresentation[i];
}

void splineRepresentation::GetPointWorldPosition(int no,double pos[3])
{
    for (int i = 0; i < PointRepresentation.size(); ++i) this->PointRepresentation[i]->GetWorldPosition(pos);
}

void splineRepresentation::SetPointDisplayPosition(int no, double pos[3])
{
    if (PointRepresentation.size() < no + 1) return;

}


//------------------------------------------------------------------------------
int splineRepresentation::ComputeInteractionState(
  int vtkNotUsed(X), int vtkNotUsed(Y), int vtkNotUsed(modify))
{
  if (this->PointRepresentation.size()==0)
  {
    this->InteractionState = splineRepresentation::Outside;
    return this->InteractionState;
  }

  int hState = Outside;
  for (int i = 0; i < PointRepresentation.size(); ++i) {
      hState = PointRepresentation[i]->GetInteractionState();
      if (hState == vtkHandleRepresentation::Nearby) {
          hState = Near;
          break;
      }
    }
  if (hState == vtkHandleRepresentation::Nearby)
  {
    this->InteractionState = splineRepresentation::Near;
  }
  else
  {
    this->InteractionState = splineRepresentation::Outside;
  }

  return this->InteractionState;
}

int splineRepresentation::ComputeComplexInteractionState(
  vtkRenderWindowInteractor*, vtkAbstractWidget*, unsigned long, void*, int)
{
    if (this->PointRepresentation.size() == 0)
  {
    this->InteractionState = splineRepresentation::Outside;
    return this->InteractionState;
  }

  int hState = Outside;
  for (int i = 0; i < PointRepresentation.size(); ++i) {
      hState = PointRepresentation[i]->GetInteractionState();
      if (hState == vtkHandleRepresentation::Nearby) {
          hState = Near;
          break;
      }
  }
  if (hState == vtkHandleRepresentation::Nearby) {
      this->InteractionState = splineRepresentation::Near;
  }
  else
  {
    this->InteractionState = splineRepresentation::Outside;
  }

  return this->InteractionState;
}

void splineRepresentation::addPointRepresentation()
{
    vtkHandleRepresentation* rep = this->HandleRepresentation->NewInstance();
    rep->ShallowCopy(this->HandleRepresentation);
    PointRepresentation.push_back(rep);
}

//------------------------------------------------------------------------------
void splineRepresentation::StartWidgetInteraction(double e[2])
{
  double pos[3];
  pos[0] = e[0];
  pos[1] = e[1];
  pos[2] = 0.0;
  for (int i = 0; i < PointRepresentation.size(); ++i) {
      SetPointDisplayPosition(i, pos);
    }
}

void splineRepresentation::StartComplexInteraction(
  vtkRenderWindowInteractor*, vtkAbstractWidget*, unsigned long, void* calldata)
{
  vtkEventData* edata = static_cast<vtkEventData*>(calldata);
  vtkEventDataDevice3D* edd = edata->GetAsEventDataDevice3D();
  if (edd)
  {
    double pos[3];
    edd->GetWorldPosition(pos);
    for (int i = 0; i < PointRepresentation.size(); ++i) {
        SetPointWorldPosition(i, pos);
    }
  }
}

//------------------------------------------------------------------------------
void splineRepresentation::WidgetInteraction(double e[2])
{
  double pos[3];
  pos[0] = e[0];
  pos[1] = e[1];
  pos[2] = 0.0;
  //this->SetPoint2DisplayPosition(pos);
}
void splineRepresentation::ComplexInteraction(
  vtkRenderWindowInteractor*, vtkAbstractWidget*, unsigned long, void* calldata)
{
  vtkEventData* edata = static_cast<vtkEventData*>(calldata);
  vtkEventDataDevice3D* edd = edata->GetAsEventDataDevice3D();
  if (edd)
  {
    double pos[3];
    edd->GetWorldPosition(pos);
    //this->SetPoint2WorldPosition(pos);
  }
}

//------------------------------------------------------------------------------
void splineRepresentation::BuildRepresentation()
{
  // Make sure that tolerance is consistent between handles and this representation
    for (int i = 0; i < PointRepresentation.size(); ++i) {
      PointRepresentation[i]->SetTolerance(this->Tolerance);
    }
}

//------------------------------------------------------------------------------
void splineRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  // Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Distance: " << this->GetDistance() << "\n";
  os << indent << "Tolerance: " << this->Tolerance << "\n";
  os << indent << "Handle Representation: " << this->HandleRepresentation << "\n";

  os << indent << "Label Format: ";
  if (this->LabelFormat)
  {
    os << this->LabelFormat << "\n";
  }
  else
  {
    os << "(none)\n";
  }

  os << indent << "Scale: " << this->GetScale() << "\n";
  os << indent << "Ruler Mode: " << (this->RulerMode ? "On" : "Off") << "\n";
  os << indent << "Ruler Distance: " << this->GetRulerDistance() << "\n";
  os << indent << "Number of Ruler Ticks: " << this->GetNumberOfRulerTicks() << "\n";

  os << indent << "Point Representation: ";
  for (int i = 0; i < PointRepresentation.size(); ++i) {
      PointRepresentation[i]->PrintSelf(os, indent.GetNextIndent());
  }
}
