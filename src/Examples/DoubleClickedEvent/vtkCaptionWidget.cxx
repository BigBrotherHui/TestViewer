/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkCaptionWidget.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "./CaptionWidget.h"
#include "vtkCallbackCommand.h"
#include "vtkCaptionRepresentation.h"
#include "vtkCommand.h"
#include "vtkHandleWidget.h"
#include "vtkObjectFactory.h"
#include "vtkPointHandleRepresentation3D.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkWidgetCallbackMapper.h"
#include "vtkWidgetEvent.h"
#include <vtkRenderer.h>
vtkStandardNewMacro(CaptionWidget);

// The point widget invokes events that we watch for. Basically
// the attachment/anchor point is moved with the point widget.
class vtkCaptionAnchorCallback : public vtkCommand
{
public:
  static vtkCaptionAnchorCallback* New() { return new vtkCaptionAnchorCallback; }
  void Execute(vtkObject*, unsigned long eventId, void*) override
  {
    switch (eventId)
    {
      case vtkCommand::StartInteractionEvent:
        this->CaptionWidget->StartAnchorInteraction();
        break;
      case vtkCommand::InteractionEvent:
        this->CaptionWidget->AnchorInteraction();
        break;
      case vtkCommand::EndInteractionEvent:
        this->CaptionWidget->EndAnchorInteraction();
        break;
    }
  }
  vtkCaptionAnchorCallback()
    : CaptionWidget(nullptr)
  {
  }
  CaptionWidget* CaptionWidget;
};
void CaptionWidget::popAction(vtkAbstractWidget*w)
{
    CaptionWidget* self = reinterpret_cast<CaptionWidget*>(w);

    if (self->SubclassTranslateAction() || self->WidgetRep->GetInteractionState() == vtkBorderRepresentation::Outside) {
        return;
    }

    // We are definitely selected
    self->GrabFocus(self->EventCallbackCommand);
    self->WidgetState = vtkBorderWidget::Selected;
    reinterpret_cast<vtkBorderRepresentation*>(self->WidgetRep)->MovingOn();

    // Picked something inside the widget
    int X = self->Interactor->GetEventPosition()[0];
    int Y = self->Interactor->GetEventPosition()[1];

    // This is redundant but necessary on some systems (windows) because the
    // cursor is switched during OS event processing and reverts to the default
    // cursor.
    self->SetCursor(self->WidgetRep->GetInteractionState());

    // convert to normalized viewport coordinates
    double XF = static_cast<double>(X);
    double YF = static_cast<double>(Y);
    self->CurrentRenderer->DisplayToNormalizedDisplay(XF, YF);
    self->CurrentRenderer->NormalizedDisplayToViewport(XF, YF);
    self->CurrentRenderer->ViewportToNormalizedViewport(XF, YF);
    double eventPos[2];
    eventPos[0] = XF;
    eventPos[1] = YF;
    self->WidgetRep->StartWidgetInteraction(eventPos);

    self->EventCallbackCommand->SetAbortFlag(1);
    self->StartInteraction();
    self->InvokeEvent(vtkCommand::StartInteractionEvent, nullptr);

    self->NumberOfClicks++;
    int pickPosition[2];
    self->GetInteractor()->GetEventPosition(pickPosition);

    int xdist = pickPosition[0] - self->PreviousPosition[0];
    int ydist = pickPosition[1] - self->PreviousPosition[1];

    self->PreviousPosition[0] = pickPosition[0];
    self->PreviousPosition[1] = pickPosition[1];

    int moveDistance = (int)sqrt((double)(xdist * xdist + ydist * ydist));
    if (moveDistance > self->ResetPixelDistance) {
        self->NumberOfClicks = 1;
    }

    if (self->NumberOfClicks == 2) {
        self->InvokeEvent(vtkCommand::LeftButtonDoubleClickEvent, nullptr);
        self->NumberOfClicks = 0;
    }
}
    //------------------------------------------------------------------------------
CaptionWidget::CaptionWidget()
{
  // The priority of the point widget is set a little higher than me.
  // This is so Enable/Disable events are caught by the anchor and then
  // dispatched to the BorderWidget.
  this->HandleWidget = vtkHandleWidget::New();
  this->HandleWidget->SetPriority(this->Priority + 0.01);
  this->HandleWidget->KeyPressActivationOff();

  // over ride the call back mapper on the border widget superclass to move
  // the caption widget using the left mouse button (still moves on middle
  // mouse button press). Release is already mapped to end select action
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent, vtkWidgetEvent::Select, this,
                                          CaptionWidget::popAction);
    

  this->AnchorCallback = vtkCaptionAnchorCallback::New();
  this->AnchorCallback->CaptionWidget = this;
  this->HandleWidget->AddObserver(vtkCommand::StartInteractionEvent, this->AnchorCallback, 1.0);
  this->HandleWidget->AddObserver(vtkCommand::InteractionEvent, this->AnchorCallback, 1.0);
  this->HandleWidget->AddObserver(vtkCommand::EndInteractionEvent, this->AnchorCallback, 1.0);
}  

//------------------------------------------------------------------------------
CaptionWidget::~CaptionWidget()
{
  this->HandleWidget->Delete();
  this->AnchorCallback->Delete();
}

//------------------------------------------------------------------------------
void CaptionWidget::SetEnabled(int enabling)
{
  if (this->Interactor)
  {
    this->Interactor->Disable(); // avoid extra renders
  }

  if (enabling)
  {
    this->CreateDefaultRepresentation();
    this->HandleWidget->SetRepresentation(
      reinterpret_cast<vtkCaptionRepresentation*>(this->WidgetRep)->GetAnchorRepresentation());
    this->HandleWidget->SetInteractor(this->Interactor);
    this->HandleWidget->SetEnabled(1);
  }
  else
  {
    this->HandleWidget->SetEnabled(0);
  }
  if (this->Interactor)
  {
    this->Interactor->Enable();
  }

  this->Superclass::SetEnabled(enabling);
}

//------------------------------------------------------------------------------
void CaptionWidget::CreateDefaultRepresentation()
{
  if (!this->WidgetRep)
  {
    this->WidgetRep = vtkCaptionRepresentation::New();
  }
}

//------------------------------------------------------------------------------
void CaptionWidget::SetCaptionActor2D(vtkCaptionActor2D* capActor)
{
  vtkCaptionRepresentation* capRep = reinterpret_cast<vtkCaptionRepresentation*>(this->WidgetRep);
  if (!capRep)
  {
    this->CreateDefaultRepresentation();
    capRep = reinterpret_cast<vtkCaptionRepresentation*>(this->WidgetRep);
  }

  if (capRep->GetCaptionActor2D() != capActor)
  {
    capRep->SetCaptionActor2D(capActor);
    this->Modified();
  }
}

//------------------------------------------------------------------------------
vtkCaptionActor2D* CaptionWidget::GetCaptionActor2D()
{
  vtkCaptionRepresentation* capRep = reinterpret_cast<vtkCaptionRepresentation*>(this->WidgetRep);
  if (!capRep)
  {
    return nullptr;
  }
  else
  {
    return capRep->GetCaptionActor2D();
  }
}

//------------------------------------------------------------------------------
void CaptionWidget::StartAnchorInteraction()
{
  this->Superclass::StartInteraction();
  this->InvokeEvent(vtkCommand::StartInteractionEvent, nullptr);
}

//------------------------------------------------------------------------------
void CaptionWidget::AnchorInteraction()
{
  vtkCaptionRepresentation* rep = reinterpret_cast<vtkCaptionRepresentation*>(this->WidgetRep);
  double pos[3];
  rep->GetAnchorRepresentation()->GetWorldPosition(pos);
  rep->SetAnchorPosition(pos);
  this->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
}

//------------------------------------------------------------------------------
void CaptionWidget::EndAnchorInteraction()
{
  this->Superclass::EndInteraction();
  this->InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
}

//------------------------------------------------------------------------------
void CaptionWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
