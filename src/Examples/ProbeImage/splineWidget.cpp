/*=========================================================================

  Program:   Visualization Toolkit
  Module:    splineWidget.cxx,v

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "splineWidget.h"
#include "vtkCallbackCommand.h"
#include "vtkCoordinate.h"
#include "vtkDistanceRepresentation2D.h"
#include "vtkEventData.h"
#include "vtkHandleRepresentation.h"
#include "vtkHandleWidget.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkWidgetCallbackMapper.h"
#include "vtkWidgetEvent.h"
#include "vtkWidgetEventTranslator.h"

vtkStandardNewMacro(splineWidget);

// The distance widget observes its two handles.
// Here we create the command/observer classes to respond to the
// handle widgets.
class splineWidgetCallback : public vtkCommand
{
public:
  static splineWidgetCallback* New() { return new splineWidgetCallback; }
  void Execute(vtkObject*, unsigned long eventId, void*) override
  {
    switch (eventId)
    {
      case vtkCommand::StartInteractionEvent:
        this->DistanceWidget->StartDistanceInteraction(this->HandleNumber);
        break;
      case vtkCommand::InteractionEvent:
        this->DistanceWidget->DistanceInteraction(this->HandleNumber);
        break;
      case vtkCommand::EndInteractionEvent:
        this->DistanceWidget->EndDistanceInteraction(this->HandleNumber);
        break;
    }
  }
  int HandleNumber;
  splineWidget* DistanceWidget;
};

//------------------------------------------------------------------------------
splineWidget::splineWidget()
{
  this->ManagesCursor = 0;

  this->WidgetState = splineWidget::Start;
  this->CurrentHandle = 0;

  addHandle();

  // These are the event callbacks supported by this widget
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
    vtkWidgetEvent::AddPoint, this, splineWidget::AddPointAction);
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::MouseMoveEvent, vtkWidgetEvent::Move, this, splineWidget::MoveAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
    vtkWidgetEvent::EndSelect, this, splineWidget::EndSelectAction);

  {
    vtkNew<vtkEventDataDevice3D> ed;
    ed->SetDevice(vtkEventDataDevice::Any);
    ed->SetInput(vtkEventDataDeviceInput::Any);
    ed->SetAction(vtkEventDataAction::Press);
    this->CallbackMapper->SetCallbackMethod(vtkCommand::Select3DEvent, ed,
      vtkWidgetEvent::AddPoint3D, this, splineWidget::AddPointAction3D);
  }

  {
    vtkNew<vtkEventDataDevice3D> ed;
    ed->SetDevice(vtkEventDataDevice::Any);
    ed->SetInput(vtkEventDataDeviceInput::Any);
    ed->SetAction(vtkEventDataAction::Release);
    this->CallbackMapper->SetCallbackMethod(vtkCommand::Select3DEvent, ed,
      vtkWidgetEvent::EndSelect3D, this, splineWidget::EndSelectAction3D);
  }

  {
    vtkNew<vtkEventDataDevice3D> ed;
    ed->SetDevice(vtkEventDataDevice::Any);
    ed->SetInput(vtkEventDataDeviceInput::Any);
    this->CallbackMapper->SetCallbackMethod(
      vtkCommand::Move3DEvent, ed, vtkWidgetEvent::Move3D, this, splineWidget::MoveAction3D);
  }
}

//------------------------------------------------------------------------------
splineWidget::~splineWidget()
{
    for (int i=0;i<PointWidget.size();++i)
    {
        PointWidget[i]->RemoveObserver(DistanceWidgetCallback[i]);
        PointWidget[i]->Delete();
        PointWidget.removeOne(PointWidget[i]);
        DistanceWidgetCallback[i]->Delete();
        DistanceWidgetCallback.removeOne(DistanceWidgetCallback[i]);
    }
}

//------------------------------------------------------------------------------
void splineWidget::CreateDefaultRepresentation()
{
  if (!this->WidgetRep)
  {
    this->WidgetRep = vtkDistanceRepresentation2D::New();
  }
  reinterpret_cast<vtkDistanceRepresentation*>(this->WidgetRep)->InstantiateHandleRepresentation();
}

//------------------------------------------------------------------------------
void splineWidget::SetEnabled(int enabling)
{
  // The handle widgets are not actually enabled until they are placed.
  // The handle widgets take their representation from the
  // vtkDistanceRepresentation.
  if (enabling)
  {
    if (this->WidgetState == splineWidget::Start)
    {
      reinterpret_cast<vtkDistanceRepresentation*>(this->WidgetRep)->VisibilityOff();
    }
    else
    {
      // The interactor must be set prior to enabling the widget.
      if (this->Interactor)
      {
          for (int i = 0; i < PointWidget.size(); ++i) {
              PointWidget[i]->SetInteractor(this->Interactor);
        }
      }

       for (int i = 0; i < PointWidget.size(); ++i) {
          PointWidget[i]->SetEnabled(1);
      }
    }
  }

  if (enabling) //----------------
  {
    if (this->Enabled) // already enabled, just return
    {
      return;
    }

    if (!this->Interactor)
    {
      vtkErrorMacro(<< "The interactor must be set prior to enabling the widget");
      return;
    }

    int X = this->Interactor->GetEventPosition()[0];
    int Y = this->Interactor->GetEventPosition()[1];

    if (!this->CurrentRenderer)
    {
      this->SetCurrentRenderer(this->Interactor->FindPokedRenderer(X, Y));
      if (this->CurrentRenderer == nullptr)
      {
        return;
      }
    }

    // We're ready to enable
    this->Enabled = 1;
    this->CreateDefaultRepresentation();
    this->WidgetRep->SetRenderer(this->CurrentRenderer);

    // Set the renderer, interactor and representation on the two handle
    // widgets.
    for (int i = 0; i < PointWidget.size(); ++i) {
        PointWidget[i]->SetRepresentation(
            reinterpret_cast<vtkDistanceRepresentation*>(this->WidgetRep)->GetPoint1Representation());
        PointWidget[i]->SetInteractor(this->Interactor);
        PointWidget[i]->GetRepresentation()->SetRenderer(this->CurrentRenderer);
    }
    /*this->Point2Widget->SetRepresentation(
      reinterpret_cast<vtkDistanceRepresentation*>(this->WidgetRep)->GetPoint2Representation());
    this->Point2Widget->SetInteractor(this->Interactor);
    this->Point2Widget->GetRepresentation()->SetRenderer(this->CurrentRenderer);*/

    // listen for the events found in the EventTranslator
    if (!this->Parent)
    {
      this->EventTranslator->AddEventsToInteractor(
        this->Interactor, this->EventCallbackCommand, this->Priority);
    }
    else
    {
      this->EventTranslator->AddEventsToParent(
        this->Parent, this->EventCallbackCommand, this->Priority);
    }

    if (this->ManagesCursor)
    {
      this->WidgetRep->ComputeInteractionState(X, Y);
      this->SetCursor(this->WidgetRep->GetInteractionState());
    }

    this->WidgetRep->BuildRepresentation();
    this->CurrentRenderer->AddViewProp(this->WidgetRep);

    if (this->WidgetState == splineWidget::Start)
    {
      reinterpret_cast<vtkDistanceRepresentation*>(this->WidgetRep)->VisibilityOff();
    }
    else
    {
        for (int i = 0; i < PointWidget.size(); ++i) {
            PointWidget[i]->SetEnabled(1);
        }
    }

    this->InvokeEvent(vtkCommand::EnableEvent, nullptr);
  }

  else // disabling------------------
  {
    vtkDebugMacro(<< "Disabling widget");

    if (!this->Enabled) // already disabled, just return
    {
      return;
    }

    this->Enabled = 0;

    // don't listen for events any more
    if (!this->Parent)
    {
      this->Interactor->RemoveObserver(this->EventCallbackCommand);
    }
    else
    {
      this->Parent->RemoveObserver(this->EventCallbackCommand);
    }

    this->CurrentRenderer->RemoveViewProp(this->WidgetRep);

 for (int i = 0; i < PointWidget.size(); ++i) {
        PointWidget[i]->SetEnabled(0);
    }

    this->InvokeEvent(vtkCommand::DisableEvent, nullptr);
    this->SetCurrentRenderer(nullptr);
  }

  // Should only render if there is no parent
  if (this->Interactor && !this->Parent)
  {
    this->Interactor->Render();
  }
}

// The following methods are the callbacks that the measure widget responds to
//------------------------------------------------------------------------------
void splineWidget::AddPointAction(vtkAbstractWidget* w)
{
  splineWidget* self = reinterpret_cast<splineWidget*>(w);
  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];

  // Freshly enabled and placing the first point
  if (self->WidgetState == splineWidget::Start)
  {
    self->GrabFocus(self->EventCallbackCommand);
    self->WidgetState = splineWidget::Define;
    self->InvokeEvent(vtkCommand::StartInteractionEvent, nullptr);
    reinterpret_cast<vtkDistanceRepresentation*>(self->WidgetRep)->VisibilityOn();
    double e[2];
    e[0] = static_cast<double>(X);
    e[1] = static_cast<double>(Y);
    reinterpret_cast<vtkDistanceRepresentation*>(self->WidgetRep)->StartWidgetInteraction(e);
    self->CurrentHandle = 0;
    self->InvokeEvent(vtkCommand::PlacePointEvent, &(self->CurrentHandle));
  }

  // Placing the second point is easy
  else if (self->WidgetState == splineWidget::Define)
  {
    self->CurrentHandle = 1;
    self->InvokeEvent(vtkCommand::PlacePointEvent, &(self->CurrentHandle));
    self->WidgetState = splineWidget::Manipulate;
    for (int i = 0; i < self->PointWidget.size(); ++i) {
        self->PointWidget[i]->SetEnabled(1);
    }
    self->CurrentHandle = -1;
    self->ReleaseFocus();
    self->InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
  }

  // Maybe we are trying to manipulate the widget handles
  else // if ( self->WidgetState == splineWidget::Manipulate )
  {
    int state = self->WidgetRep->ComputeInteractionState(X, Y);

    if (state == vtkDistanceRepresentation::Outside)
    {
      self->CurrentHandle = -1;
      return;
    }

    self->GrabFocus(self->EventCallbackCommand);
    if (state == vtkDistanceRepresentation::NearP1)
    {
      self->CurrentHandle = 0;
    }
    else if (state == vtkDistanceRepresentation::NearP2)
    {
      self->CurrentHandle = 1;
    }
    self->InvokeEvent(vtkCommand::LeftButtonPressEvent, nullptr);
  }

  // Clean up
  self->EventCallbackCommand->SetAbortFlag(1);
  self->Render();
}

//------------------------------------------------------------------------------
void splineWidget::AddPointAction3D(vtkAbstractWidget* w)
{
  splineWidget* self = reinterpret_cast<splineWidget*>(w);

  vtkEventData* edata = static_cast<vtkEventData*>(self->CallData);
  vtkEventDataDevice3D* edd = edata->GetAsEventDataDevice3D();
  if (!edd)
  {
    return;
  }

  // Freshly enabled and placing the first point
  if (self->WidgetState == splineWidget::Start)
  {
    self->WidgetState = splineWidget::Define;
    self->InvokeEvent(vtkCommand::StartInteractionEvent, nullptr);
    reinterpret_cast<vtkDistanceRepresentation*>(self->WidgetRep)->VisibilityOn();
    self->WidgetRep->StartComplexInteraction(
      self->Interactor, self, vtkWidgetEvent::AddPoint, self->CallData);
    self->CurrentHandle = 0;
    self->InvokeEvent(vtkCommand::PlacePointEvent, &(self->CurrentHandle));
    self->EventCallbackCommand->SetAbortFlag(1);
    self->LastDevice = static_cast<int>(edd->GetDevice());
  }

  // Placing the second point is easy
  else if (self->WidgetState == splineWidget::Define)
  {
    self->CurrentHandle = 1;
    self->InvokeEvent(vtkCommand::PlacePointEvent, &(self->CurrentHandle));
    self->WidgetState = splineWidget::Manipulate;
    for (int i = 0; i < self->PointWidget.size(); ++i) {
        self->PointWidget[i]->SetEnabled(1);
    }
    self->CurrentHandle = -1;
    self->InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
    self->EventCallbackCommand->SetAbortFlag(1);
  }

  // Maybe we are trying to manipulate the widget handles
  else // if ( self->WidgetState == splineWidget::Manipulate )
  {
    int state = self->WidgetRep->ComputeComplexInteractionState(
      self->Interactor, self, vtkWidgetEvent::AddPoint, self->CallData);

    if (state == vtkDistanceRepresentation::Outside)
    {
      self->CurrentHandle = -1;
      return;
    }

    if (state == vtkDistanceRepresentation::NearP1)
    {
      self->CurrentHandle = 0;
    }
    else if (state == vtkDistanceRepresentation::NearP2)
    {
      self->CurrentHandle = 1;
    }
    self->InvokeEvent(vtkCommand::Select3DEvent, self->CallData);
    self->EventCallbackCommand->SetAbortFlag(1);
  }

  // Clean up
  self->Render();
}

//------------------------------------------------------------------------------
void splineWidget::MoveAction(vtkAbstractWidget* w)
{
  splineWidget* self = reinterpret_cast<splineWidget*>(w);

  // Do nothing if in start mode or valid handle not selected
  if (self->WidgetState == splineWidget::Start)
  {
    return;
  }

  // Delegate the event consistent with the state
  if (self->WidgetState == splineWidget::Define)
  {
    int X = self->Interactor->GetEventPosition()[0];
    int Y = self->Interactor->GetEventPosition()[1];
    double e[2];
    e[0] = static_cast<double>(X);
    e[1] = static_cast<double>(Y);
    reinterpret_cast<vtkDistanceRepresentation*>(self->WidgetRep)->WidgetInteraction(e);
    self->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
    self->EventCallbackCommand->SetAbortFlag(1);
  }
  else // must be moving a handle, invoke a event for the handle widgets
  {
    self->InvokeEvent(vtkCommand::MouseMoveEvent, nullptr);
  }

  self->WidgetRep->BuildRepresentation();
  self->Render();
}

//------------------------------------------------------------------------------
void splineWidget::MoveAction3D(vtkAbstractWidget* w)
{
  splineWidget* self = reinterpret_cast<splineWidget*>(w);

  vtkEventData* edata = static_cast<vtkEventData*>(self->CallData);
  vtkEventDataDevice3D* edd = edata->GetAsEventDataDevice3D();
  if (!edd || static_cast<int>(edd->GetDevice()) != self->LastDevice)
  {
    return;
  }

  // Do nothing if in start mode or valid handle not selected
  if (self->WidgetState == splineWidget::Start)
  {
    return;
  }

  // Delegate the event consistent with the state
  if (self->WidgetState == splineWidget::Define)
  {
    self->WidgetRep->ComplexInteraction(
      self->Interactor, self, vtkWidgetEvent::Move3D, self->CallData);
    self->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
  }
  else // must be moving a handle, invoke a event for the handle widgets
  {
    self->InvokeEvent(vtkCommand::Move3DEvent, self->CallData);
  }

  self->WidgetRep->BuildRepresentation();
  self->Render();
}

//------------------------------------------------------------------------------
void splineWidget::EndSelectAction(vtkAbstractWidget* w)
{
  splineWidget* self = reinterpret_cast<splineWidget*>(w);

  // Do nothing if outside
  if (self->WidgetState == splineWidget::Start ||
    self->WidgetState == splineWidget::Define || self->CurrentHandle < 0)
  {
    return;
  }

  self->ReleaseFocus();
  self->InvokeEvent(vtkCommand::LeftButtonReleaseEvent, nullptr);
  self->CurrentHandle = -1;
  self->WidgetRep->BuildRepresentation();
  self->EventCallbackCommand->SetAbortFlag(1);
  self->Render();
}

//------------------------------------------------------------------------------
void splineWidget::EndSelectAction3D(vtkAbstractWidget* w)
{
  splineWidget* self = reinterpret_cast<splineWidget*>(w);

  // Do nothing if outside
  if (self->WidgetState == splineWidget::Start ||
    self->WidgetState == splineWidget::Define || self->CurrentHandle < 0)
  {
    return;
  }

  self->ReleaseFocus();
  self->InvokeEvent(vtkCommand::Select3DEvent, self->CallData);
  self->CurrentHandle = -1;
  self->WidgetRep->BuildRepresentation();
  self->EventCallbackCommand->SetAbortFlag(1);
  self->Render();
}

void splineWidget::addHandle()
{
    vtkHandleWidget *handle = vtkHandleWidget::New();
    handle->SetParent(this);
    PointWidget.push_back(handle);
    splineWidgetCallback *callback = splineWidgetCallback::New();
    callback->HandleNumber = 0;
    callback->DistanceWidget = this;
    handle->AddObserver(vtkCommand::StartInteractionEvent, callback, this->Priority);
    handle->AddObserver(vtkCommand::InteractionEvent, callback, this->Priority);
    handle->AddObserver(vtkCommand::EndInteractionEvent, callback, this->Priority);
    DistanceWidgetCallback.push_back(callback);
}

// These are callbacks that are active when the user is manipulating the
// handles of the measure widget.
//------------------------------------------------------------------------------
void splineWidget::StartDistanceInteraction(int)
{
  this->Superclass::StartInteraction();
  this->InvokeEvent(vtkCommand::StartInteractionEvent, nullptr);
}

//------------------------------------------------------------------------------
void splineWidget::DistanceInteraction(int)
{
  this->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
}

//------------------------------------------------------------------------------
void splineWidget::EndDistanceInteraction(int)
{
  this->Superclass::EndInteraction();
  this->InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
}

//------------------------------------------------------------------------------
void splineWidget::SetProcessEvents(vtkTypeBool pe)
{
  this->Superclass::SetProcessEvents(pe);

  for (int i = 0; i < PointWidget.size(); ++i) PointWidget[i]->SetProcessEvents(pe);
}

//------------------------------------------------------------------------------
void splineWidget::SetWidgetStateToStart()
{
  this->WidgetState = splineWidget::Start;
  this->CurrentHandle = -1;
  this->ReleaseFocus();
  this->GetRepresentation()->BuildRepresentation(); // update this->Distance
  this->SetEnabled(this->GetEnabled());             // show/hide the handles properly
}

//------------------------------------------------------------------------------
void splineWidget::SetWidgetStateToManipulate()
{
  this->WidgetState = splineWidget::Manipulate;
  this->CurrentHandle = -1;
  this->ReleaseFocus();
  this->GetRepresentation()->BuildRepresentation(); // update this->Distance
  this->SetEnabled(this->GetEnabled());             // show/hide the handles properly
}

//------------------------------------------------------------------------------
void splineWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  // Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os, indent);
}
