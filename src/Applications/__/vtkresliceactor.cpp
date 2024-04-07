#include "vtkresliceactor.h"
#include <vtkImageSlice.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkUnsignedCharArray.h>
#include <vtkRegularPolygonSource.h>
#include <QDebug>
#include <vtkPolyData.h>
#include <vtkContourFilter.h>
#include <vtkCharArray.h>
vtkStandardNewMacro(asclepios::gui::vtkResliceActor);
namespace {
void AssignScalarValueTo(vtkPolyData* polydata, char value)
{
    vtkSmartPointer<vtkCharArray> pointData = vtkSmartPointer<vtkCharArray>::New();

    int numberOfPoints = polydata->GetNumberOfPoints();
    pointData->SetNumberOfComponents(1);
    pointData->SetNumberOfTuples(numberOfPoints);
    pointData->FillComponent(0, value);
    polydata->GetPointData()->SetScalars(pointData);
}
}  // namespace
void asclepios::gui::vtkResliceActor::createActor()
{
	m_appender = vtkSmartPointer<vtkAppendPolyData>::New();
	for (auto i = 0; i < 2; ++i)
	{
		m_cursorLines[i] = vtkSmartPointer<vtkLineSource>::New();
		m_appender->AddInputConnection(m_cursorLines[i]->GetOutputPort());
	}
	m_mapper =
		vtkSmartPointer<vtkPolyDataMapper>::New();
	m_mapper->SetInputConnection(m_appender->GetOutputPort());
	m_mapper->ScalarVisibilityOn();
	m_mapper->SetScalarModeToUsePointFieldData();
	m_mapper->SelectColorArray("Colors");
	m_actor =
		vtkSmartPointer<vtkActor>::New();
	m_actor->SetMapper(m_mapper);
	m_actor->GetProperty()->SetInterpolationToGouraud();


	circleactor = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkNew<vtkRegularPolygonSource> source;
	source->SetNumberOfSides(30);
	source->SetRadius(10);
	source->SetCenter(0, 0, 0);
	source->Update();
	
	mapper->SetInputData(source->GetOutput());
	circleactor->SetMapper(mapper);
	circleactor->GetProperty()->SetRepresentationToWireframe();

	resultactor = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkPolyDataMapper> mapper2 = vtkSmartPointer<vtkPolyDataMapper>::New();
	resultactor->SetMapper(mapper2);
	append = vtkSmartPointer<vtkAppendPolyData>::New();
	mapper2->SetInputConnection(append->GetOutputPort());
	resultactor->GetProperty()->SetRepresentationToWireframe();
}

//-----------------------------------------------------------------------------
void asclepios::gui::vtkResliceActor::reset() const
{
	auto* const orientation = m_actor->GetOrientation();
	m_actor->RotateZ(-orientation[2]);
	m_actor->SetPosition(0, 0, 0);

	auto* const ori = circleactor->GetOrientation();
	circleactor->RotateZ(-ori[2]);
	circleactor->SetPosition(0, 0, 0);
}

//-----------------------------------------------------------------------------
void asclepios::gui::vtkResliceActor::setDisplaySize(const double* t_size)
{
	m_windowSize[0] = t_size[0];
	m_windowSize[1] = t_size[1];
	m_windowSize[2] = t_size[2];
}

//-----------------------------------------------------------------------------
void asclepios::gui::vtkResliceActor::setDisplayOriginPoint(const double* t_point)
{
	m_windowOrigin[0] = t_point[0];
	m_windowOrigin[1] = t_point[1];
	m_windowOrigin[2] = t_point[2];
}

//-----------------------------------------------------------------------------
void asclepios::gui::vtkResliceActor::setCenterPosition(const double* t_center)
{
	m_centerPointDisplayPosition[0] = t_center[0];
	m_centerPointDisplayPosition[1] = t_center[1];
	m_centerPointDisplayPosition[2] = t_center[2];
}

//-----------------------------------------------------------------------------
void asclepios::gui::vtkResliceActor::update()
{
	if (m_start == 0)
	{
		m_cursorLines[0]->SetPoint1(m_windowOrigin[0],
			m_centerPointDisplayPosition[1], 0.01);
		m_cursorLines[0]->SetPoint2(m_windowSize[0],
			m_centerPointDisplayPosition[1], 0.01);
		m_cursorLines[0]->Update();
		m_cursorLines[0]->GetOutput()->GetPointData()->AddArray(m_colors[1]);
                AssignScalarValueTo(m_cursorLines[0]->GetOutput(),0);

		m_cursorLines[1]->SetPoint1(m_centerPointDisplayPosition[0],
			m_windowOrigin[1], 0.01);
		m_cursorLines[1]->SetPoint2(m_centerPointDisplayPosition[0],
			m_windowSize[1], 0.01);
		m_cursorLines[1]->Update();
		m_cursorLines[1]->GetOutput()->GetPointData()->AddArray(m_colors[0]);
                AssignScalarValueTo(m_cursorLines[1]->GetOutput(), 1);

		m_actor->SetScale(5);
		//circleactor->SetScale(5);
		//resultactor->SetScale(5);
		m_start = 1;
	}
	else
	{
		m_actor->SetPosition(
			m_centerPointDisplayPosition[0],
			m_centerPointDisplayPosition[1],
			0.01);
		circleactor->SetPosition(
			m_centerPointDisplayPosition[0],
			m_centerPointDisplayPosition[1],
			0.01);

		vtkSmartPointer<vtkRegularPolygonSource> source= vtkSmartPointer<vtkRegularPolygonSource>::New();
		source->SetNumberOfSides(30);
		source->SetRadius(10);
		source->SetCenter(m_centerPointDisplayPosition[0],
			m_centerPointDisplayPosition[1],0);
		append->AddInputConnection(source->GetOutputPort());
	}
}

//-----------------------------------------------------------------------------
void asclepios::gui::vtkResliceActor::createColors(double* t_color1, double* t_color2)
{
	m_colors[0] =
		vtkSmartPointer<vtkUnsignedCharArray>::New();
	m_colors[0]->SetName("Colors");
	m_colors[0]->SetNumberOfComponents(3);
	m_colors[0]->SetNumberOfTuples(100);
	for (auto j = 0; j < 100; j++)
	{
		m_colors[0]->InsertTuple3(j, t_color1[0],
		                          t_color1[1], t_color1[2]);
	}
	m_colors[1] =
		vtkSmartPointer<vtkUnsignedCharArray>::New();
	m_colors[1]->SetName("Colors");
	m_colors[1]->SetNumberOfComponents(3);
	m_colors[1]->SetNumberOfTuples(100);
	for (auto j = 0; j < 100; j++)
	{
		m_colors[1]->InsertTuple3(j, t_color2[0],
		                          t_color2[1], t_color2[2]);
	}
}
