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
        m_appenderTranslate = vtkSmartPointer<vtkAppendPolyData>::New();
        m_appenderRotate = vtkSmartPointer<vtkAppendPolyData>::New();
	for (auto i = 0; i < 2; ++i)
	{
	    m_cursorLines[i] = vtkSmartPointer<vtkLineSource>::New();
            m_appenderTranslate->AddInputConnection(m_cursorLines[i]->GetOutputPort());
	}
        for (auto i = 0; i < 4; ++i) {
            m_cursorLines2[i] = vtkSmartPointer<vtkLineSource>::New();
            m_appenderRotate->AddInputConnection(m_cursorLines2[i]->GetOutputPort());
        }

        vtkSmartPointer<vtkPolyDataMapper> m_mapper =
		vtkSmartPointer<vtkPolyDataMapper>::New();
        m_mapper->SetInputConnection(m_appenderTranslate->GetOutputPort());
	m_mapper->ScalarVisibilityOn();
	m_mapper->SetScalarModeToUsePointFieldData();
	m_mapper->SelectColorArray("Colors");
        m_actorTranslate =
		vtkSmartPointer<vtkActor>::New();
        m_actorTranslate->SetMapper(m_mapper);
        m_actorTranslate->GetProperty()->SetInterpolationToGouraud();


	vtkSmartPointer<vtkPolyDataMapper> m_mapper2 = vtkSmartPointer<vtkPolyDataMapper>::New();
        m_mapper2->SetInputConnection(m_appenderRotate->GetOutputPort());
        m_mapper2->ScalarVisibilityOn();
        m_mapper2->SetScalarModeToUsePointFieldData();
        m_mapper2->SelectColorArray("Colors");
        m_actorRotate = vtkSmartPointer<vtkActor>::New();
        m_actorRotate->SetMapper(m_mapper2);
        m_actorRotate->GetProperty()->SetInterpolationToGouraud();
}

//-----------------------------------------------------------------------------
void asclepios::gui::vtkResliceActor::reset() const
{
    auto* const orientation = m_actorTranslate->GetOrientation();
    m_actorTranslate->RotateZ(-orientation[2]);
    m_actorTranslate->SetPosition(0, 0, 0);
    m_actorRotate->RotateZ(-orientation[2]);
    m_actorRotate->SetPosition(0, 0, 0);
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
            int factor = 10;
            m_cursorLines[0]->SetPoint1(m_centerPointDisplayPosition[0] - m_windowSize[0] / factor,
			m_centerPointDisplayPosition[1], 0.01);
            m_cursorLines[0]->SetPoint2(m_centerPointDisplayPosition[0] + m_windowSize[0] / factor,
			m_centerPointDisplayPosition[1], 0.01);
	    m_cursorLines[0]->Update();
	    m_cursorLines[0]->GetOutput()->GetPointData()->AddArray(m_colors[1]);
            AssignScalarValueTo(m_cursorLines[0]->GetOutput(), 0);

	    m_cursorLines2[0]->SetPoint1(m_centerPointDisplayPosition[0] - m_windowSize[0] / factor*2,
                                         m_centerPointDisplayPosition[1], 0.01);
            m_cursorLines2[0]->SetPoint2(m_centerPointDisplayPosition[0] - m_windowSize[0] / factor,
                                         m_centerPointDisplayPosition[1], 0.01);
            m_cursorLines2[0]->Update();
            m_cursorLines2[0]->GetOutput()->GetPointData()->AddArray(m_colors[1]);

            m_cursorLines2[2]->SetPoint1(m_centerPointDisplayPosition[0] + m_windowSize[0] / factor * 2,
                                         m_centerPointDisplayPosition[1], 0.01);
            m_cursorLines2[2]->SetPoint2(m_centerPointDisplayPosition[0] + m_windowSize[0] / factor,
                                         m_centerPointDisplayPosition[1], 0.01);
            m_cursorLines2[2]->Update();
            m_cursorLines2[2]->GetOutput()->GetPointData()->AddArray(m_colors[1]);


	    m_cursorLines[1]->SetPoint1(m_centerPointDisplayPosition[0],
                                        m_centerPointDisplayPosition[1] -m_windowSize[1] / factor, 0.01);
            m_cursorLines[1]->SetPoint2(m_centerPointDisplayPosition[0],
                                        m_centerPointDisplayPosition[1] +m_windowSize[1] / factor, 0.01);
	    m_cursorLines[1]->Update();
	    m_cursorLines[1]->GetOutput()->GetPointData()->AddArray(m_colors[0]);
            AssignScalarValueTo(m_cursorLines[1]->GetOutput(), 1);


            m_cursorLines2[1]->SetPoint1(m_centerPointDisplayPosition[0],
                                         m_centerPointDisplayPosition[0] - m_windowSize[1] / factor*2,
                                         0.01);
            m_cursorLines2[1]->SetPoint2(m_centerPointDisplayPosition[0],
                                         m_centerPointDisplayPosition[0] - m_windowSize[1] / factor,
                                         0.01);
            m_cursorLines2[1]->Update();
            m_cursorLines2[1]->GetOutput()->GetPointData()->AddArray(m_colors[0]);

            m_cursorLines2[3]->SetPoint1(m_centerPointDisplayPosition[0],
                                         m_centerPointDisplayPosition[0] + m_windowSize[1] / factor * 2, 0.01);
            m_cursorLines2[3]->SetPoint2(m_centerPointDisplayPosition[0],
                                         m_centerPointDisplayPosition[0] + m_windowSize[1] / factor, 0.01);
            m_cursorLines2[3]->Update();
            m_cursorLines2[3]->GetOutput()->GetPointData()->AddArray(m_colors[0]);

	    m_actorTranslate->SetScale(5);
            m_actorRotate->SetScale(5);

	    m_start = 1;
	}
	else
	{
            m_actorTranslate->SetPosition(
			m_centerPointDisplayPosition[0],
			m_centerPointDisplayPosition[1],
			0.01);
            m_actorRotate->SetPosition(m_centerPointDisplayPosition[0], m_centerPointDisplayPosition[1], 0.01);
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
