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
#include <vtkArrowSource.h>
#include <array>
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkMinimalStandardRandomSequence.h>
#include <vtkLine.h>
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

vtkSmartPointer<vtkPolyData> CreateArrow(bool leftforward, std::array<double, 3>& startPoint,
                                         std::array<double, 3>& endPoint)
{
    vtkSmartPointer<vtkPolyData> ret = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    double leftSide[3], rightSide[3];
    int offset = 1;
    if (leftforward) {
        leftSide[0] = endPoint[0] - offset;
        leftSide[1] = endPoint[1] + offset;
        leftSide[2] = endPoint[2];
        rightSide[0] = endPoint[0] + offset;
        rightSide[1] = endPoint[1] + offset;
        rightSide[2] = endPoint[2];
    }
    else {
        leftSide[0] = endPoint[0] + offset;
        leftSide[1] = endPoint[1] + offset;
        leftSide[2] = endPoint[2];
        rightSide[0] = endPoint[0] + offset;
        rightSide[1] = endPoint[1] - offset;
        rightSide[2] = endPoint[2];
    }
    points->InsertNextPoint(startPoint.data());
    points->InsertNextPoint(endPoint.data());
    points->InsertNextPoint(leftSide);
    points->InsertNextPoint(rightSide);
    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkLine> line0 = vtkSmartPointer<vtkLine>::New();
    line0->GetPointIds()->SetId(0,0);
    line0->GetPointIds()->SetId(1, 1);
    vtkSmartPointer<vtkLine> line1 = vtkSmartPointer<vtkLine>::New();
    line1->GetPointIds()->SetId(0, 1);
    line1->GetPointIds()->SetId(1, 2);
    vtkSmartPointer<vtkLine> line2 = vtkSmartPointer<vtkLine>::New();
    line2->GetPointIds()->SetId(0, 1);
    line2->GetPointIds()->SetId(1, 3);
    lines->InsertNextCell(line0);
    lines->InsertNextCell(line1);
    lines->InsertNextCell(line2);
    ret->SetPoints(points);
    ret->SetLines(lines);
    return ret;
    //vtkSmartPointer<vtkPolyData> polyData;

    //// Create an arrow.
    //vtkNew<vtkArrowSource> arrowSource;
    //arrowSource->SetShaftRadius(pdLength * .01);
    //arrowSource->SetShaftResolution(20);
    //arrowSource->SetTipLength(pdLength * .1);
    //arrowSource->SetTipRadius(pdLength * .05);
    //arrowSource->SetTipResolution(20);

    //// Compute a basis
    //std::array<double, 3> normalizedX;
    //std::array<double, 3> normalizedY;
    //std::array<double, 3> normalizedZ;

    //// The X axis is a vector from start to end
    //vtkMath::Subtract(endPoint.data(), startPoint.data(), normalizedX.data());
    //double length = vtkMath::Norm(normalizedX.data());
    //vtkMath::Normalize(normalizedX.data());

    //// The Z axis is an arbitrary vector cross X
    //vtkNew<vtkMinimalStandardRandomSequence> rng;
    //rng->SetSeed(8775070);

    //std::array<double, 3> arbitrary;
    //for (auto i = 0; i < 3; ++i) {
    //    rng->Next();
    //    arbitrary[i] = rng->GetRangeValue(-10, 10);
    //}
    //vtkMath::Cross(normalizedX.data(), arbitrary.data(), normalizedZ.data());
    //vtkMath::Normalize(normalizedZ.data());

    //// The Y axis is Z cross X
    //vtkMath::Cross(normalizedZ.data(), normalizedX.data(), normalizedY.data());
    //vtkNew<vtkMatrix4x4> matrix;

    //// Create the direction cosine matrix
    //matrix->Identity();
    //for (auto i = 0; i < 3; i++) {
    //    matrix->SetElement(i, 0, normalizedX[i]);
    //    matrix->SetElement(i, 1, normalizedY[i]);
    //    matrix->SetElement(i, 2, normalizedZ[i]);
    //}

    //// Apply the transforms
    //vtkNew<vtkTransform> transform;
    //transform->Translate(startPoint.data());
    //transform->Concatenate(matrix);
    //transform->Scale(length, length, length);

    //// Transform the polydata
    //vtkNew<vtkTransformPolyDataFilter> transformPD;
    //transformPD->SetTransform(transform);
    //transformPD->SetInputConnection(arrowSource->GetOutputPort());
    //transformPD->Update();
    //polyData = transformPD->GetOutput();
    //return polyData;
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
            std::array<double, 3> start,end;
            start[0] = m_centerPointDisplayPosition[0] - m_windowSize[0] / factor;
            start[1] = 0;
            start[2] = 0.01;
            end[0] = m_centerPointDisplayPosition[0] - m_windowSize[0] / factor;
            end[1] = -6;
            end[2] = 0.01;
            arrowTop1->DeepCopy(CreateArrow(true, start, end));
            AssignScalarValueTo(arrowTop1, 10);
            m_appenderTranslate->AddInputData(arrowTop1);
            arrowTop1->GetPointData()->AddArray(m_colors[1]);

            start[0] = m_centerPointDisplayPosition[0] + m_windowSize[0] / factor;
            start[1] = 0;
            start[2] = 0.01;
            end[0] = m_centerPointDisplayPosition[0] + m_windowSize[0] / factor;
            end[1] = -6;
            end[2] = 0.01;
            arrowTop2->DeepCopy(CreateArrow(true, start, end));
            AssignScalarValueTo(arrowTop2, 10);
            m_appenderTranslate->AddInputData(arrowTop2);
            arrowTop2->GetPointData()->AddArray(m_colors[1]);

	    m_cursorLines2[0]->SetPoint1(m_centerPointDisplayPosition[0] - m_windowSize[0] / factor*2000,
                                         m_centerPointDisplayPosition[1], 0.01);
            m_cursorLines2[0]->SetPoint2(m_centerPointDisplayPosition[0] - m_windowSize[0] / factor,
                                         m_centerPointDisplayPosition[1], 0.01);
            m_cursorLines2[0]->Update();
            m_cursorLines2[0]->GetOutput()->GetPointData()->AddArray(m_colors[1]);

            m_cursorLines2[2]->SetPoint1(m_centerPointDisplayPosition[0] + m_windowSize[0] / factor * 2000,
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

            start[0] = m_centerPointDisplayPosition[0] + 6;
            start[1] = m_centerPointDisplayPosition[1] - m_windowSize[1] / factor;
            start[2] = 0.01;
            end[0] = m_centerPointDisplayPosition[0];
            end[1] = m_centerPointDisplayPosition[1] - m_windowSize[1] / factor;
            end[2] = 0.01;
            arrowLeft1->DeepCopy(CreateArrow(false, start, end));
            AssignScalarValueTo(arrowLeft1, 10);
            m_appenderTranslate->AddInputData(arrowLeft1);
            arrowLeft1->GetPointData()->AddArray(m_colors[0]);

            start[0] = m_centerPointDisplayPosition[0] + 6;
            start[1] = m_centerPointDisplayPosition[1] + m_windowSize[1] / factor;
            start[2] = 0.01;
            end[0] = m_centerPointDisplayPosition[0];
            end[1] = m_centerPointDisplayPosition[1] + m_windowSize[1] / factor;
            end[2] = 0.01;
            arrowLeft2->DeepCopy(CreateArrow(false, start, end));
            AssignScalarValueTo(arrowLeft2, 10);
            m_appenderTranslate->AddInputData(arrowLeft2);
            arrowLeft2->GetPointData()->AddArray(m_colors[0]);


            m_cursorLines2[1]->SetPoint1(m_centerPointDisplayPosition[0],
                                         m_centerPointDisplayPosition[0] - m_windowSize[1] / factor * 2000,
                                         0.01);
            m_cursorLines2[1]->SetPoint2(m_centerPointDisplayPosition[0],
                                         m_centerPointDisplayPosition[0] - m_windowSize[1] / factor,
                                         0.01);
            m_cursorLines2[1]->Update();
            m_cursorLines2[1]->GetOutput()->GetPointData()->AddArray(m_colors[0]);

            m_cursorLines2[3]->SetPoint1(m_centerPointDisplayPosition[0],
                                         m_centerPointDisplayPosition[0] + m_windowSize[1] / factor * 2000, 0.01);
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
