#include "vtkresliceplanecursorwidget.h"
#include <vtkCallbackCommand.h>
#include <vtkCellPicker.h>
#include <vtkCommand.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkWidgetCallbackMapper.h>
#include <vtkWidgetEvent.h>
#include <vtkWidgetEventTranslator.h>
#include <vtkWidgetRepresentation.h>
#include "utils.h"
#include "vtkreslicewidgetrepresentation.h"
#include <vtkRendererCollection.h>
#include <QApplication>
#include <vtkPointData.h>
#include <vtkCamera.h>
#include <QDebug>
vtkStandardNewMacro(asclepios::gui::vtkReslicePlaneCursorWidget);

double asclepios::gui::vtkReslicePlaneCursorWidget::__lastCursorPos[3]{};
class vtkReslicePlaneCursorCallback final : public vtkCommand {
public:
	static vtkReslicePlaneCursorCallback* New()
	{
		return new vtkReslicePlaneCursorCallback;
	}

	void Execute(vtkObject*, const unsigned long eventId, void*) override
	{
		switch (eventId)
		{
		case StartInteractionEvent:
			m_resliceWidget->startWidgetInteraction(m_handleNumber);
			break;
		case InteractionEvent:
			m_resliceWidget->widgetInteraction(m_handleNumber);
			break;
		case EndInteractionEvent:
			m_resliceWidget->endWidgetInteraction(m_handleNumber);
			break;
		default:
			break;
		}
	}

	int m_handleNumber = {};
	asclepios::gui::vtkReslicePlaneCursorWidget* m_resliceWidget = {};
};


asclepios::gui::vtkReslicePlaneCursorWidget::vtkReslicePlaneCursorWidget()
{
	initializeWidget();
}


asclepios::gui::vtkReslicePlaneCursorWidget::~vtkReslicePlaneCursorWidget()
{
	RemoveAllObservers();
}

//-----------------------------------------------------------------------------
void asclepios::gui::vtkReslicePlaneCursorWidget::initializeWidget()
{
	m_centerMovementWidget = vtkSmartPointer<vtkHandleWidget>::New();
	m_centerMovementWidget->SetParent(this);
	m_centerMovementCallback =
		vtkReslicePlaneCursorCallback::New();
	m_centerMovementCallback->m_handleNumber = 1;
	m_centerMovementCallback->m_resliceWidget = this;
	m_centerMovementWidget->AddObserver(
		vtkCommand::StartInteractionEvent,
		m_centerMovementCallback,
		Priority);
	m_centerMovementWidget->AddObserver(
		vtkCommand::InteractionEvent,
		m_centerMovementCallback,
		Priority);
	m_centerMovementWidget->AddObserver(
		vtkCommand::EndInteractionEvent,
		m_centerMovementCallback,
		Priority);
	m_centerMovementWidget->AddObserver(
		vtkCommand::Move3DEvent,
		m_centerMovementCallback,
		Priority);
	CallbackMapper->SetCallbackMethod(
		vtkCommand::LeftButtonPressEvent,
		vtkWidgetEvent::AddPoint, this,
		leftMouseDownAction);
	CallbackMapper->SetCallbackMethod(
		vtkCommand::MouseMoveEvent,
		vtkWidgetEvent::Move, this,
		moveMouse);
	CallbackMapper->SetCallbackMethod(
		vtkCommand::LeftButtonReleaseEvent,
		vtkWidgetEvent::EndSelect,
		this, leftMouseUpAction);
}

//-----------------------------------------------------------------------------
void asclepios::gui::vtkReslicePlaneCursorWidget::SetEnabled(const int enabling)
{
	if (enabling) //----------------
	{
		vtkDebugMacro(<< "Enabling widget");
		if (Enabled)
		{
			return;
		}
		if (!Interactor)
		{
			vtkErrorMacro(<< "The interactor must be set prior to enabling the widget");
			return;
		}
		if (m_plane == -1)
		{
			return;
		}
		const int X = Interactor->GetEventPosition()[0];
		const int Y = Interactor->GetEventPosition()[1];
		if (!CurrentRenderer)
		{
			SetCurrentRenderer(Interactor->FindPokedRenderer(X, Y));

			if (!CurrentRenderer)
			{
				return;
			}
		}

		Enabled = 1;
		CreateDefaultRepresentation();
		WidgetRep->SetRenderer(CurrentRenderer);

		m_centerMovementWidget->SetRepresentation(
			reinterpret_cast<vtkResliceWidgetRepresentation*>(WidgetRep)->
			getCenterMovementRepresentation());
		reinterpret_cast<vtkResliceWidgetRepresentation*>(WidgetRep)->setPlane(m_plane);
		m_centerMovementWidget->SetInteractor(Interactor);
		m_centerMovementWidget->GetRepresentation()->SetRenderer(CurrentRenderer);
		m_centerMovementWidget->SetEnabled(1);
		m_centerMovementWidget->ManagesCursorOff();
		if (!Parent)
		{
			EventTranslator->AddEventsToInteractor(Interactor,
			                                       EventCallbackCommand, Priority);
		}
		else
		{
			EventTranslator->AddEventsToParent(Parent,
			                                   EventCallbackCommand, Priority);
		}
		if (ManagesCursor)
		{
			WidgetRep->ComputeInteractionState(X, Y);
			SetCursor(WidgetRep->GetInteractionState());
		}
		WidgetRep->BuildRepresentation();
		CurrentRenderer->AddViewProp(WidgetRep);
		InvokeEvent(vtkCommand::EnableEvent, nullptr);
	}
	else //disabling------------------
	{
		vtkDebugMacro(<< "Disabling widget");

		if (!Enabled)
		{
			return;
		}
		Enabled = 0;
		if (!Parent)
		{
			Interactor->RemoveObserver(EventCallbackCommand);
		}
		else
		{
			Parent->RemoveObserver(EventCallbackCommand);
		}
		CurrentRenderer->RemoveViewProp(WidgetRep);
		InvokeEvent(vtkCommand::DisableEvent, nullptr);
		SetCurrentRenderer(nullptr);
	}
}

//-----------------------------------------------------------------------------
void asclepios::gui::vtkReslicePlaneCursorWidget::CreateDefaultRepresentation()
{
	if (!WidgetRep)
	{
		WidgetRep = vtkResliceWidgetRepresentation::New();
	}
}

//-----------------------------------------------------------------------------
void asclepios::gui::vtkReslicePlaneCursorWidget::leftMouseDownAction(vtkAbstractWidget* w)
{
	vtkReslicePlaneCursorWidget* self =
		reinterpret_cast<vtkReslicePlaneCursorWidget*>(w);
	const int X = self->Interactor->GetEventPosition()[0];
	const int Y = self->Interactor->GetEventPosition()[1];
	auto* const rep =
		dynamic_cast<vtkResliceWidgetRepresentation*>(self->WidgetRep);
	if (!rep)
	{
		self->InvokeEvent(vtkCommand::LeftButtonPressEvent, nullptr);
	}
        vtkSmartPointer<vtkCellPicker> picker = vtkSmartPointer<vtkCellPicker>::New();
	picker->SetTolerance(0.01);
	picker->InitializePickList();
	picker->PickFromListOn();
	const auto resliceActor =
		rep->getResliceActor();
	if (resliceActor && rep->GetVisibility())
	{
            if (resliceActor->getInteractionMode() == 0) {
            picker->AddPickList(resliceActor->getActorTranslate());
			picker->AddPickList(resliceActor->getActorRotate());
		}
		picker->AddPickList(resliceActor->getActorLattice());
	}
	if (picker->Pick(X, Y, 0, self->GetCurrentRenderer()))
	{
	    if (self->m_state != bothTranslate)
	    {
                if (picker->GetActor() == resliceActor->getActorTranslate())
		{
		    auto id=picker->GetPointId();
		    int value=static_cast<vtkPolyDataMapper*>(picker->GetActor()->GetMapper())
									    ->GetInput()
									    ->GetPointData()
									    ->GetScalars()
									    ->GetTuple1(id);
                    if (value == 0 || value == 1 ) {
			self->m_state = translate;
		    }
		    self->m_selectedAxis = value;
		}
		else if(picker->GetActor() == resliceActor->getActorRotate()){
			self->m_state = rotate;
		}
                else if (picker->GetActor() == resliceActor->getActorLattice()) {
                    auto id = picker->GetPointId();
                    int value = static_cast<vtkPolyDataMapper*>(picker->GetActor()->GetMapper())
                                    ->GetInput()
                                    ->GetPointData()
                                    ->GetScalars()
                                    ->GetTuple1(id);
                    if (self->WidgetRep->ComputeInteractionState(X, Y) == VTK_CURSOR_SIZEALL)
                    {
                        self->m_state = translate;
                    }
		    else if(value==11 || value==12){//wall的前后2根线
		        self->m_state=expand;
		    }
		    else if(value==resliceActor->getPickedSliceTupleValue()){
			    self->m_state=rotate;
		    }
		    else if(value==255){//wall的其它线
			auto renderer =
				self->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
                        vtkSmartPointer<vtkCoordinate> coordinate = vtkSmartPointer<vtkCoordinate>::New();
			coordinate->SetCoordinateSystemToDisplay();
			coordinate->SetValue(X, Y);
			double* worldCoordinate = coordinate->GetComputedWorldValue(renderer);
			double cd[3]{worldCoordinate[0], worldCoordinate[1], worldCoordinate[2]};
			self->pickCurrentSlice(cd[0], cd[1], cd[2],value);
		    }
		    self->m_selectedAxis = value;
		}
	    }
	    self->InvokeEvent(qualityLow, &self->m_plane);
	    self->GrabFocus(self->EventCallbackCommand);
	    self->InvokeEvent(vtkCommand::LeftButtonPressEvent, nullptr);
	    self->EventCallbackCommand->SetAbortFlag(1);
	}
	else
	{
            auto renderer = self->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
            vtkSmartPointer<vtkCoordinate> coordinate = vtkSmartPointer<vtkCoordinate>::New();
            coordinate->SetCoordinateSystemToDisplay();
            coordinate->SetValue(X, Y);
            double* worldCoordinate = coordinate->GetComputedWorldValue(renderer);
            double cd[3]{worldCoordinate[0], worldCoordinate[1], worldCoordinate[2]};
            self->pickCurrentSlice(cd[0], cd[1], cd[2], 255);
	    self->GrabFocus(self->EventCallbackCommand);
	    self->InvokeEvent(vtkCommand::LeftButtonPressEvent, nullptr);
	}
	self->Render();
}

//-----------------------------------------------------------------------------
void asclepios::gui::vtkReslicePlaneCursorWidget::moveMouse(vtkAbstractWidget* w)
{
	auto* const self = reinterpret_cast<vtkReslicePlaneCursorWidget*>(w);
	auto* const rep = dynamic_cast<vtkResliceWidgetRepresentation*>(self->WidgetRep);
	const auto x = self->Interactor->GetEventPosition()[0];
	const auto y = self->Interactor->GetEventPosition()[1];
	const auto lastX = self->Interactor->GetLastEventPosition()[0];
	const auto lastY = self->Interactor->GetLastEventPosition()[1];
	vtkNew<vtkCoordinate> actorCoordinates;
	switch (self->m_state)
	{
		case start: {
			self->SetCursor(self->WidgetRep->ComputeInteractionState(x, y));
			if (!self->m_centerMovementWidget->GetEnabled()) {
				rep->BuildRepresentation();
				self->m_centerMovementWidget->SetEnabled(1);
			}
			auto* const representation = dynamic_cast<vtkResliceWidgetRepresentation*>(self->WidgetRep);
			__lastCursorPos[0] = representation->getResliceActor()->getActorTranslate()->GetPosition()[0];
			__lastCursorPos[1] = representation->getResliceActor()->getActorTranslate()->GetPosition()[1];
			__lastCursorPos[2] = representation->getResliceActor()->getActorTranslate()->GetPosition()[2];
			}
		break;
	    case translate:
	    {
	        auto renderer = self->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
		vtkNew<vtkCoordinate> coordinate;
		coordinate->SetCoordinateSystemToDisplay();
		coordinate->SetValue(x, y);
		double* worldCoordinate = coordinate->GetComputedWorldValue(renderer);
		double cd[3]{worldCoordinate[0], worldCoordinate[1], worldCoordinate[2]};	
		coordinate->SetValue(lastX, lastY);
		double* lastworldCoordinate = coordinate->GetComputedWorldValue(renderer);
		self->translateCursor(cd[0] - lastworldCoordinate[0], cd[1] - lastworldCoordinate[1],
								cd[2] - lastworldCoordinate[2],self->m_selectedAxis);
		auto* const representation = dynamic_cast<vtkResliceWidgetRepresentation*>(self->WidgetRep);
		double pos[3] = {0.00, 0.00, 0.00};
		pos[0] = representation->getResliceActor()->getActorTranslate()->GetPosition()[0] - __lastCursorPos[0];
		pos[1] = representation->getResliceActor()->getActorTranslate()->GetPosition()[1] - __lastCursorPos[1];
		pos[2] = representation->getResliceActor()->getActorTranslate()->GetPosition()[2] - __lastCursorPos[2];
		self->InvokeEvent(cursorMove, pos);
	    }
	    break;
	    case rotate:
	    {
		double* centerActor = rep->getResliceActor()->getActorRotate()->GetPosition();
		actorCoordinates->SetCoordinateSystemToWorld();
		actorCoordinates->SetValue(centerActor[0], centerActor[1]);
		const auto* center = actorCoordinates->GetComputedDisplayValue(self->CurrentRenderer);
		const auto newAngle = atan2(y - center[1], x - center[0]);
		const auto oldAngle = atan2(lastY - center[1], lastX - center[0]);
		self->rotateCursor(newAngle - oldAngle);
		double angle[1] = {vtkMath::DegreesFromRadians(newAngle - oldAngle)};
		self->InvokeEvent(cursorRotate, angle);
		
		// auto renderer =
		// 	self->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
		// vtkNew<vtkCoordinate> coordinate;
		// coordinate->SetCoordinateSystemToDisplay();
		// coordinate->SetValue(x, y);
		// double* worldCoordinate = coordinate->GetComputedWorldValue(renderer);
		// double cd[3]{worldCoordinate[0], worldCoordinate[1], worldCoordinate[2]};
		// self->pickCurrentSlice(cd[0], cd[1], cd[2],255);
			
	    }
	    break;
	    case expand:{
                auto renderer = self->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
                vtkNew<vtkCoordinate> coordinate;
                coordinate->SetCoordinateSystemToDisplay();
                coordinate->SetValue(x, y);
                double* worldCoordinate = coordinate->GetComputedWorldValue(renderer);
                double cd[3]{worldCoordinate[0], worldCoordinate[1], worldCoordinate[2]};
                self->expandWall(cd[0] , cd[1],cd[2] , self->m_selectedAxis);
                self->InvokeEvent(cursorFinishMovement, &self->m_plane);
	    }
	    break;
	default:
		break;
	}
	self->InvokeEvent(vtkCommand::MouseMoveEvent, nullptr);
	self->Render();
}

//-----------------------------------------------------------------------------
void asclepios::gui::vtkReslicePlaneCursorWidget::leftMouseUpAction(vtkAbstractWidget* w)
{
    vtkReslicePlaneCursorWidget* self = reinterpret_cast<vtkReslicePlaneCursorWidget*>(w);
    self->SetCursor(0);
    self->m_state = start;
    self->ReleaseFocus();
    self->InvokeEvent(qualityHigh, &self->m_plane);
    self->InvokeEvent(vtkCommand::LeftButtonReleaseEvent, nullptr);
    self->InvokeEvent(cursorFinishMovement, &self->m_plane);
    self->EventCallbackCommand->SetAbortFlag(1);
    self->Render();
}

//-----------------------------------------------------------------------------
void asclepios::gui::vtkReslicePlaneCursorWidget::rotateCursor(double t_angle) const
{
    dynamic_cast<vtkResliceWidgetRepresentation*>(WidgetRep)->rotate(t_angle);
}

void asclepios::gui::vtkReslicePlaneCursorWidget::translateCursor(double x, double y,double z, char moveAxes) const
{
    dynamic_cast<vtkResliceWidgetRepresentation*>(WidgetRep)->translate(x, y,z, moveAxes);
}

void asclepios::gui::vtkReslicePlaneCursorWidget::expandWall(double x, double y, double z, unsigned char moveAxes) const
{
    dynamic_cast<vtkResliceWidgetRepresentation*>(WidgetRep)->expand(x,y,z, moveAxes);
}

void asclepios::gui::vtkReslicePlaneCursorWidget::pickCurrentSlice(double x, double y, double z,unsigned char moveaxes)
{
    dynamic_cast<vtkResliceWidgetRepresentation*>(WidgetRep)->pickCurrentSlice(x, y, z,moveaxes);
}

//-----------------------------------------------------------------------------
void asclepios::gui::vtkReslicePlaneCursorWidget::setPlane(int t_plane)
{
	m_plane = t_plane;
}

//-----------------------------------------------------------------------------
void asclepios::gui::vtkReslicePlaneCursorWidget::SetCursor(int t_state)
{
	if (!WidgetRep->GetVisibility())
	{
		return;
	}
	RequestCursorShape(t_state);
	// switch (t_state)
	// {
	// case vtkResliceWidgetRepresentation::outside:
	// 	RequestCursorShape(VTK_CURSOR_DEFAULT);
	// 	break;
	// case vtkResliceWidgetRepresentation::translateCursorHor:
	// 	RequestCursorShape(VTK_CURSOR_SIZENS);
	// 	break;
	// case vtkResliceWidgetRepresentation::translateCursorVer:
	// 	RequestCursorShape(VTK_CURSOR_SIZEWE);
	// 	break;
	// case vtkResliceWidgetRepresentation::rotateCursor:
	// 	RequestCursorShape(VTK_CURSOR_HAND);
	// 	break;
	// case vtkResliceWidgetRepresentation::handleCursor:
	// 	RequestCursorShape(VTK_CURSOR_SIZEALL);
	// 	break;
	// default:
	// 	RequestCursorShape(VTK_CURSOR_DEFAULT);
	// }
}

//-----------------------------------------------------------------------------
void asclepios::gui::vtkReslicePlaneCursorWidget::setCursorPosition(double* t_position) const
{
	dynamic_cast<vtkResliceWidgetRepresentation*>
		(WidgetRep)->setCursorPosition(t_position);
}

//-----------------------------------------------------------------------------
void asclepios::gui::vtkReslicePlaneCursorWidget::setCursorCenterPosition(const double* t_position)
{
	m_cursorCenterPosition[0] = t_position[0];
	m_cursorCenterPosition[1] = t_position[1];
	m_cursorCenterPosition[2] = t_position[2];
}

double* asclepios::gui::vtkReslicePlaneCursorWidget::getCursorCenterPosition()
{
	return m_cursorCenterPosition;
}

//-----------------------------------------------------------------------------
void asclepios::gui::vtkReslicePlaneCursorWidget::startWidgetInteraction(int handleNum)
{
	Superclass::StartInteraction();
	m_state = bothTranslate;
	InvokeEvent(vtkCommand::StartInteractionEvent, nullptr);
	auto* const representation =
		dynamic_cast<vtkResliceWidgetRepresentation*>(WidgetRep);
	lastCursorPos[0] = representation->getResliceActor()->getActorTranslate()->GetPosition()[0];
	lastCursorPos[1] = representation->getResliceActor()->getActorTranslate()->GetPosition()[1];
	lastCursorPos[2] = representation->getResliceActor()->getActorTranslate()->GetPosition()[2];
}

//-----------------------------------------------------------------------------
void asclepios::gui::vtkReslicePlaneCursorWidget::widgetInteraction([[maybe_unused]] int handleNum)
{
	InvokeEvent(vtkCommand::InteractionEvent, nullptr);
	auto* const representation =
		dynamic_cast<vtkResliceWidgetRepresentation*>(WidgetRep);
	double pos[3] = {0.00, 0.00, 0.00};
	pos[0] = representation->getResliceActor()->getActorTranslate()->GetPosition()[0] - lastCursorPos[0];
	pos[1] = representation->getResliceActor()->getActorTranslate()->GetPosition()[1] - lastCursorPos[1];
	pos[2] = representation->getResliceActor()->getActorTranslate()->GetPosition()[2] - lastCursorPos[2];
	InvokeEvent(cursorMove, pos);
}

//-----------------------------------------------------------------------------
void asclepios::gui::vtkReslicePlaneCursorWidget::endWidgetInteraction([[maybe_unused]] int handleNum)
{
	Superclass::EndInteraction();
	InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
	InvokeEvent(cursorFinishMovement, &m_plane);
}
