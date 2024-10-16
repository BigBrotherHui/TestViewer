#include "vtkreslicewidgetrepresentation.h"
#include <vtkCamera.h>
#include <vtkCellPicker.h>
#include <vtkCoordinate.h>
#include <vtkEllipseArcSource.h>
#include <vtkHandleRepresentation.h>
#include <vtkObjectFactory.h>
#include <vtkPointHandleRepresentation3D.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRegularPolygonSource.h>
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>
#include <QDebug>
#include <qmath.h>
#include <vtkPolyDataMapper.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include "vtkAlgorithm.h"
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkProperty.h>
vtkStandardNewMacro(asclepios::gui::vtkResliceWidgetRepresentation);

asclepios::gui::vtkResliceWidgetRepresentation::vtkResliceWidgetRepresentation()
{
	initializeRepresentation();
}

//-----------------------------------------------------------------------------
void asclepios::gui::vtkResliceWidgetRepresentation::initializeRepresentation()
{
	createCursor();
	instantiateHandleRepresentation();
}

//-----------------------------------------------------------------------------
void asclepios::gui::vtkResliceWidgetRepresentation::createCursor()
{
	m_cursorActor = vtkSmartPointer<vtkResliceActor>::New();
	m_cursorVisibility = true;
}

//-----------------------------------------------------------------------------
void asclepios::gui::vtkResliceWidgetRepresentation::SetVisibility(const int _arg)
{
	m_cursorVisibility = static_cast<bool>(_arg);
}

//-----------------------------------------------------------------------------
int asclepios::gui::vtkResliceWidgetRepresentation::GetVisibility()
{
	return static_cast<int>(m_cursorVisibility);
}

//-----------------------------------------------------------------------------
int asclepios::gui::vtkResliceWidgetRepresentation::ComputeInteractionState(
	const int X, const int Y, int modify)
{
	if (m_centerMovementPointRepresentation->ComputeInteractionState(X, Y)
		!= vtkHandleRepresentation::Outside)
	{
		return VTK_CURSOR_SIZEALL;
	}
	vtkNew<vtkCellPicker> picker;
	picker->SetTolerance(0.01);
	picker->InitializePickList();
	picker->PickFromListOn();
	if(m_cursorActor->getInteractionMode()==0){
		picker->AddPickList(m_cursorActor->getActorTranslate());
		picker->AddPickList(m_cursorActor->getActorRotate());
	}
	picker->AddPickList(m_cursorActor->getActorLattice());
	if (picker->Pick(X, Y, 0, Renderer))
	{
            if (picker->GetActor() == m_cursorActor->getActorTranslate()) {
		    auto id=picker->GetPointId();
		    auto v = static_cast<vtkPolyDataMapper*>(picker->GetActor()->GetMapper())
									    ->GetInput()
									    ->GetPointData()
									    ->GetScalars()
									    ->GetTuple1(id);
                if (v == 0) {
		        return calculateCursorState(m_rotationAngle * 180 / M_PI);
	        }
	        else if (v == 1) {
		        return calculateCursorState(m_rotationAngle * 180 / M_PI + 90);
	        }
	        else {
		        return VTK_CURSOR_DEFAULT;
	        }    
	    }
		else if(picker->GetActor()==m_cursorActor->getActorLattice()){
			auto id=picker->GetPointId();
		    auto v = static_cast<vtkPolyDataMapper*>(picker->GetActor()->GetMapper())
									    ->GetInput()
									    ->GetPointData()
									    ->GetScalars()
									    ->GetTuple1(id);
										if(v == 11 || v==12){
											return calculateCursorState(m_rotationAngle * 180 / M_PI);
										}
										else if(v==m_cursorActor->getPickedSliceTupleValue())
										{
											return VTK_CURSOR_HAND;
										}
										else {
		        return VTK_CURSOR_DEFAULT;
	        }  
		}
	    else if (picker->GetActor() == m_cursorActor->getActorRotate()) {
		    return VTK_CURSOR_HAND;
	    }
	}
    return VTK_CURSOR_DEFAULT;
}

//-----------------------------------------------------------------------------
void asclepios::gui::vtkResliceWidgetRepresentation::instantiateHandleRepresentation()
{
	if (!m_centerMovementPointRepresentation)
	{
        m_centerMovementPointRepresentation = vtkSmartPointer<SphereHandleRepresentation>::New();
		//m_centerMovementPointRepresentation->SetSmoothMotion(1);
		m_centerMovementPointRepresentation->SetHandleSize(20);
		m_centerMovementPointRepresentation->SetTolerance(15);
		//m_centerMovementPointRepresentation->AllOff();
		m_centerMovementPointRepresentation->TranslationModeOn();
	}
}

//-----------------------------------------------------------------------------
void asclepios::gui::vtkResliceWidgetRepresentation::BuildRepresentation()
{
	if (GetMTime() > BuildTime ||
		getCenterMovementRepresentation()->GetMTime() > BuildTime ||
		Renderer && Renderer->GetVTKWindow() &&
		Renderer->GetVTKWindow()->GetMTime() > BuildTime)
	{
		double centerPos[3];
		m_centerMovementPointRepresentation->GetWorldPosition(centerPos);
		Renderer->GetVTKWindow()->GetSize();
		m_cursorActor->setCameraDistance(
			Renderer->GetActiveCamera()->GetDistance());
		vtkNew<vtkCoordinate> coord;
		coord->SetCoordinateSystemToDisplay();
		coord->SetValue(
			Renderer->GetVTKWindow()->GetSize()[0],
			Renderer->GetVTKWindow()->GetSize()[1], 0);
		double* size = coord->GetComputedWorldValue(Renderer);
		m_cursorActor->setCenterPosition(centerPos);
		m_cursorActor->setDisplaySize(size);
		coord->SetValue(0, 0);
		double* origin = coord->GetComputedWorldValue(Renderer);
		m_cursorActor->setDisplayOriginPoint(origin);
		m_cursorActor->update();
		BuildTime.Modified();
	}
}

//-----------------------------------------------------------------------------
void asclepios::gui::vtkResliceWidgetRepresentation::ReleaseGraphicsResources(
	vtkWindow* w)
{
	if (m_cursorActor && m_cursorVisibility)
	{
        m_cursorActor->getActorTranslate()->ReleaseGraphicsResources(w);
		m_cursorActor->getActorRotate()->ReleaseGraphicsResources(w);
		m_cursorActor->getActorLattice()->ReleaseGraphicsResources(w);
		m_cursorActor->getActorText()->ReleaseGraphicsResources(w);
	}
}

//-----------------------------------------------------------------------------
int asclepios::gui::vtkResliceWidgetRepresentation::RenderOverlay(
	vtkViewport* viewport)
{
	int count = 0;
    if (m_cursorActor->getActorTranslate() && m_cursorVisibility)
	{
		if(m_cursorActor->getInteractionMode()==0){
           count += m_cursorActor->getActorTranslate()->RenderOverlay(viewport);
			count += m_cursorActor->getActorRotate()->RenderOverlay(viewport);
		}else{
count +=m_cursorActor->getActorLattice()->RenderOverlay(viewport);
			count +=m_cursorActor->getActorText()->RenderOverlay(viewport);
		}
			
	}
	return count;
}

//-----------------------------------------------------------------------------
int asclepios::gui::vtkResliceWidgetRepresentation::RenderOpaqueGeometry(
	vtkViewport* viewport)
{
    if (m_cursorActor->getActorTranslate() && m_cursorActor->getActorTranslate()->GetVisibility() &&
		m_cursorVisibility)
	{		
		if(m_cursorActor->getInteractionMode()==0){
			m_cursorActor->getActorTranslate()->RenderOpaqueGeometry(viewport);
			m_cursorActor->getActorRotate()->RenderOpaqueGeometry(viewport);
		}else{
			m_cursorActor->getActorText()->RenderOpaqueGeometry(viewport);
			m_cursorActor->getActorLattice()->RenderOpaqueGeometry(viewport);
		}
		
	}
	else
	{
		return 0;
	}
	return 1;
}

//-----------------------------------------------------------------------------
int asclepios::gui::vtkResliceWidgetRepresentation::HasTranslucentPolygonalGeometry()
{
    BuildRepresentation();
    return m_cursorActor->getActorTranslate()
                   ? m_cursorActor->getActorTranslate()->HasTranslucentPolygonalGeometry()
		       : 0;
}

//-----------------------------------------------------------------------------
void asclepios::gui::vtkResliceWidgetRepresentation::rotate(double t_angle)
{
    m_rotationAngle += t_angle;
    m_cursorActor->getActorRotate()->RotateZ(vtkMath::DegreesFromRadians(t_angle));
    m_cursorActor->getActorTranslate()->RotateZ(vtkMath::DegreesFromRadians(t_angle));
	m_cursorActor->getActorLattice()->RotateZ(vtkMath::DegreesFromRadians(t_angle));
	m_cursorActor->getActorText()->RotateZ(vtkMath::DegreesFromRadians(t_angle));
}

void asclepios::gui::vtkResliceWidgetRepresentation::reset(){
	m_rotationAngle=0;
}

void asclepios::gui::vtkResliceWidgetRepresentation::translate(double x, double y,double z, char moveAxes)
{
    if (moveAxes == 1) {
        calculateTranslateX(x, y, m_rotationAngle);
        m_cursorActor->getActorTranslate()->AddPosition(x, y, 0);
        m_cursorActor->getActorRotate()->AddPosition(x, y, 0);
	m_cursorActor->getActorText()->AddPosition(x, y, 0);
    }   
    else {
        calculateTranslateY(x, y, m_rotationAngle);
        m_cursorActor->getActorTranslate()->AddPosition(x, y, 0);
        m_cursorActor->getActorRotate()->AddPosition(x, y, 0);
	m_cursorActor->getActorText()->AddPosition(x, y, 0);
    }
}

void asclepios::gui::vtkResliceWidgetRepresentation::expand(double x, double y, double z, unsigned char moveAxes)
{
    double diff[3];
    diff[0] = x - getResliceActor()->getCenterPosition()[0];
    diff[1] = y - getResliceActor()->getCenterPosition()[1];
    diff[2] = z - getResliceActor()->getCenterPosition()[2];
    calculateTranslateY(diff[0], diff[1], m_rotationAngle);
    double angle = fmod(m_rotationAngle, 2 * M_PI);
    int quadrant = 0;
    if (std::abs(angle) < 1e-3)
        quadrant = 1;
    else if (angle > -M_PI && angle < -0.5 * M_PI)
        quadrant = 3;
    else if ((angle > M_PI && angle < M_PI * 7 / 4) || (angle > -M_PI && angle < -M_PI / 2))
        quadrant = 3;
    else if ((angle > M_PI * 3. / 2 && angle < 2 * M_PI) || (angle < 0 && angle > -M_PI / 2))
        quadrant = 4;
    else if (angle > 0 && angle < M_PI / 2 || (angle > -2 * M_PI && angle < -3. / 2 * M_PI))
        quadrant = 1;
    else
        quadrant = 2;
    double distance = std::sqrt(diff[0] * diff[0] + diff[1] * diff[1]);
    //qDebug() << "1111111angle:" << angle << "quadrant:" << quadrant << "diff[1]:" << diff[1];
    if (quadrant == 1) {
        if (diff[1] < 0 && moveAxes == 12)
            distance = -distance;
        else if (moveAxes == 11 && diff[1] < 0)
            return;
    }
    if (quadrant == 2) {
        if (diff[1]>0 && moveAxes==12)
            distance = -distance;
        else if (moveAxes == 11 && diff[1]>0)
	    return;
    }
    else if (quadrant==3)
    {
        if (moveAxes == 12 && diff[1] > 0) distance = -distance;
        else if (moveAxes == 11 && diff[1]>0)
	    return;
    }
    else if (quadrant == 4) {
        if (moveAxes == 12 && diff[1] < 0) distance = -distance;
        if (moveAxes == 11 && diff[1]<0)
	    return;
    }
    getResliceActor()->createWallRepresentation(x, distance, z, moveAxes,getResliceActor()->getPickedSlice());
}

void asclepios::gui::vtkResliceWidgetRepresentation::pickCurrentSlice(double x, double y, double z,
                                                                      unsigned char moveAxes)
{
    double diff[3];
    diff[0] = x - getResliceActor()->getCenterPosition()[0];
    diff[1] = y - getResliceActor()->getCenterPosition()[1];
    diff[2] = z - getResliceActor()->getCenterPosition()[2];
    calculateTranslateY(diff[0], diff[1], m_rotationAngle);
    double distance = std::sqrt(diff[0] * diff[0] + diff[1] * diff[1]);
    distance = distance/getResliceActor()->getActorScale()/getResliceActor()->getWallSpacing();
    double roundedDistance =std::round(distance);
    int slice = (int)roundedDistance;
    double angle = fmod(m_rotationAngle, 2*M_PI);
    int quadrant = 0;
    if (std::abs(angle) < 1e-3)
        quadrant = 1;
    else if (angle > -M_PI && angle < -0.5*M_PI)
        quadrant = 3;
    else if ((angle > M_PI && angle < M_PI*3. / 2) || (angle>-M_PI&& angle<-M_PI/2))
        quadrant = 3;
    else if ((angle > M_PI * 7. / 4 && angle < 2*M_PI) || (angle<0 && angle>-M_PI/2))
        quadrant = 4;
    else if (angle > 0 && angle < M_PI / 2 || (angle > -2 * M_PI && angle < -3. / 2 * M_PI))
        quadrant = 1;
    else
        quadrant = 2;
    //qDebug() << "22222222angle:" << angle << "quadrant:" << quadrant << "diff[1]:" << diff[1];
    if (diff[1] < 0 && (quadrant == 4 || quadrant==1)) slice = -slice;
    if (diff[1] > 0 && (quadrant == 2 || quadrant==3)) slice = -slice;
    getResliceActor()->createWallRepresentation(x, distance, z, moveAxes,slice);
}

//-----------------------------------------------------------------------------
void asclepios::gui::vtkResliceWidgetRepresentation::setPlane(const int t_plane)
{
	m_plane = t_plane;
	if (m_cursorActor)
	{
		if(t_plane==0){
			double verticalColor[3] = {170, 255, 190};
			double horizontalColor[3] = {255, 160, 190};
			m_cursorActor->createColors(verticalColor, horizontalColor);
		}
		else if(t_plane==1){
			double verticalColor[3] = {255, 160, 190};
			double horizontalColor[3] = {255, 250, 160};
			m_cursorActor->createColors(verticalColor, horizontalColor);
		}
		else{
			double verticalColor[3] = {170, 255, 190};
			double horizontalColor[3] = {255, 250, 160};
			m_cursorActor->createColors(verticalColor, horizontalColor);
		}
	}
}

//-----------------------------------------------------------------------------
void asclepios::gui::vtkResliceWidgetRepresentation::setCursorPosition(double* t_position)
{
	m_centerMovementPointRepresentation->SetWorldPosition(t_position);
	BuildRepresentation();
}
