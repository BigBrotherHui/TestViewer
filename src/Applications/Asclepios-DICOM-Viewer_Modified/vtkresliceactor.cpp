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
#include <vtkCardinalSpline.h>
#include <vtkSplineFilter.h>
#include <Eigen/Eigen>
vtkStandardNewMacro(asclepios::gui::vtkResliceActor);
namespace {
void AssignScalarValueTo(vtkPolyData* polydata, char value)
{
    if (!polydata) return;
    vtkSmartPointer<vtkCharArray> pointData = vtkSmartPointer<vtkCharArray>::New();
    int numberOfPoints = polydata->GetNumberOfPoints();
    pointData->SetNumberOfComponents(1);
    pointData->SetNumberOfTuples(numberOfPoints);
    pointData->FillComponent(0, value);
    polydata->GetPointData()->SetScalars(pointData);
}
void cross(double *A, double *B, double *C)
{
    C[0] = A[1] * B[2] - B[1] * A[2];
    C[1] = B[0] * A[2] - A[0] * B[2];
    C[2] = A[0] * B[1] - B[0] * A[1];
}

void normalize(double *A)
{
    double norm = std::sqrt(A[0] * A[0] + A[1] * A[1] + A[2] * A[2]);
    for (int i = 0; i < 3; ++i) {
        A[i] = A[i] / norm;
    }
}

vtkSmartPointer<vtkPolyData> ExpandSpline(vtkPolyData *line, int divisionNum, double stepSize)
{
    vtkNew<vtkPoints> points;
    for (int i{0}; i < line->GetNumberOfPoints(); i++) {
        double currentPoint[3];
        currentPoint[0] = line->GetPoint(i)[0];
        currentPoint[1] = line->GetPoint(i)[1];
        currentPoint[2] = line->GetPoint(i)[2];

        double z_axis[3];
        z_axis[0] = 0;
        z_axis[1] = 0;
        z_axis[2] = 1;

        double ptpVector[3];

        if (i == (line->GetNumberOfPoints() - 1)) {
            ptpVector[0] = -line->GetPoint(i - 1)[0] + currentPoint[0];
            ptpVector[1] = -line->GetPoint(i - 1)[1] + currentPoint[1];
            ptpVector[2] = -line->GetPoint(i - 1)[2] + currentPoint[2];
        }
        else {
            ptpVector[0] = line->GetPoint(i + 1)[0] - currentPoint[0];
            ptpVector[1] = line->GetPoint(i + 1)[1] - currentPoint[1];
            ptpVector[2] = line->GetPoint(i + 1)[2] - currentPoint[2];
        }

        double tmpVector[3];
        cross(z_axis, ptpVector, tmpVector);

        normalize(tmpVector);
        points->InsertNextPoint(currentPoint[0] + tmpVector[0] * stepSize, currentPoint[1] + tmpVector[1] * stepSize,
                                currentPoint[2] + tmpVector[2] * stepSize);
    }

    // vtkCellArrays
    vtkNew<vtkCellArray> lines;
    lines->InsertNextCell(points->GetNumberOfPoints());
    for (unsigned int i = 0; i < points->GetNumberOfPoints(); ++i) {
        lines->InsertCellPoint(i);
    }

    // vtkPolyData
    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetLines(lines);

    return polyData;
}
vtkSmartPointer<vtkPolyData> SweepLine_2Sides(vtkPolyData *line, double direction[3], double distance,
                                                          unsigned cols)
{
    unsigned int rows = line->GetNumberOfPoints();
    double spacing = distance / cols;
    vtkNew<vtkPolyData> surface;

    // Generate the points.
    cols++;
    unsigned int numberOfPoints = rows * cols;
    unsigned int numberOfPolys = (rows - 1) * (cols - 1);
    vtkNew<vtkPoints> points;
    points->Allocate(numberOfPoints);
    vtkNew<vtkCellArray> polys;
    polys->Allocate(numberOfPolys * 4);

    double x[3];
    unsigned int cnt = 0;
    for (unsigned int row = 0; row < rows; row++) {
        for (unsigned int col = 0; col < cols; col++) {
            double p[3];
            line->GetPoint(row, p);
            x[0] = p[0] - distance * direction[0] / 2 + direction[0] * col * spacing;
            x[1] = p[1] - distance * direction[1] / 2 + direction[1] * col * spacing;
            x[2] = p[2] - distance * direction[2] / 2 + direction[2] * col * spacing;
            points->InsertPoint(cnt++, x);
        }
    }
    // Generate the quads.
    vtkIdType pts[4];
    for (unsigned int row = 0; row < rows - 1; row++) {
        for (unsigned int col = 0; col < cols - 1; col++) {
            pts[0] = col + row * (cols);
            pts[1] = pts[0] + 1;
            pts[2] = pts[0] + cols + 1;
            pts[3] = pts[0] + cols;
            polys->InsertNextCell(4, pts);
        }
    }
    surface->SetPoints(points);
    surface->SetPolys(polys);

    return surface;
}
}  // namespace
void asclepios::gui::vtkResliceActor::createWallRepresentation(double x, double y, double z,int value)
{
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
    lines->InsertNextCell(2);
    double *center=m_centerPointDisplayPosition;
    double centerLeft[3]{0, 0, 0}, centerRight[3]{0, 0, 0};
    for (int i = 0; i < 2; ++i) {
        centerLeft[i] = center[i];
        centerRight[i] = center[i];
    }
    double bound = 1000;
    centerLeft[0] -= bound;
    centerRight[0] += bound;
    points->InsertNextPoint(centerLeft);
    points->InsertNextPoint(centerRight);
    lines->InsertCellPoint(0);
    lines->InsertCellPoint(1);
    vtkSmartPointer<vtkPolyData> p = vtkSmartPointer<vtkPolyData>::New();
    p->SetPoints(points);
    p->SetLines(lines);

    //if (value==11) 
    //    m_imageNumBack += y / actorScale/m_wallSpacing;
    //else if (value==12)
    //    m_imageNumFront += -y / actorScale / m_wallSpacing;
    if (value == 11)
        m_imageNumBack = y / actorScale / m_wallSpacing;
    else if (value == 12)
        m_imageNumFront = -y / actorScale / m_wallSpacing;
    vtkSmartPointer<vtkAppendPolyData> append1 = vtkSmartPointer<vtkAppendPolyData>::New();
    for (int i = 0; i < m_imageNumFront; i++) {
        double stepSize = m_wallSpacing * (-m_imageNumFront + i);
        auto expandedSpline = ExpandSpline(p, p->GetNumberOfPoints() - 1, stepSize);
        append1->AddInputData(expandedSpline);
    }
    append1->Update();
    if (append1->GetOutput()) {
        AssignScalarValueTo(append1->GetOutput(), 255);
        if (append1->GetOutput()->GetPointData()->GetScalars()->GetNumberOfTuples() > 2) {
            for (int i = 0; i < 2; ++i) {
                append1->GetOutput()->GetPointData()->GetScalars()->SetTuple1(i, 12);
            }
        }
    }
        
    vtkSmartPointer<vtkAppendPolyData> append2 = vtkSmartPointer<vtkAppendPolyData>::New();
    for (int i = 0; i < m_imageNumBack; i++) {
        double stepSize = m_wallSpacing * (i + 1);
        auto expandedSpline = ExpandSpline(p, p->GetNumberOfPoints() - 1, stepSize);
        append2->AddInputData(expandedSpline);
    }
    append2->Update();
    if (append2->GetOutput()) {
        AssignScalarValueTo(append2->GetOutput(), 255);
        if (append2->GetOutput()->GetPointData()->GetScalars()->GetNumberOfTuples() > 2) {
            for (int i = append2->GetOutput()->GetPointData()->GetScalars()->GetNumberOfTuples() - 1, count = 0;
                 i > 0 && count < 2; --i, ++count) {
                append2->GetOutput()->GetPointData()->GetScalars()->SetTuple1(i, 11);
            }
        }
        
    }
    append = vtkSmartPointer<vtkAppendPolyData>::New();
    if (append1->GetOutput())
        append->AddInputData(append1->GetOutput());
    if (append2->GetOutput())
        append->AddInputData(append2->GetOutput());
    append->Update();
    polydataWall->DeepCopy(append->GetOutput());
    double _colorWall[3]{0, 0.3 * 255, 0};
    double _colorWallEdge[3]{0.6*255, 0, 0};
    vtkSmartPointer<vtkUnsignedCharArray> colorWall = vtkSmartPointer<vtkUnsignedCharArray>::New();
    colorWall->SetName("Colors");
    colorWall->SetNumberOfComponents(3);
    colorWall->SetNumberOfTuples(polydataWall->GetNumberOfPoints());
    auto scalars = polydataWall->GetPointData()->GetScalars();
    for (auto j = 0; j < polydataWall->GetNumberOfPoints(); j++) {
        if (scalars->GetTuple1(j) == 11 || scalars->GetTuple1(j) == 12)
        {
            colorWall->InsertTuple3(j, _colorWallEdge[0], _colorWallEdge[1], _colorWallEdge[2]);
        }
        else {
            colorWall->InsertTuple3(j, _colorWall[0], _colorWall[1], _colorWall[2]);
        }
    }
    polydataWall->GetPointData()->AddArray(colorWall);
}

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

double* asclepios::gui::vtkResliceActor::getCenterPosition()
{
    return m_centerPointDisplayPosition;
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

        createWallRepresentation(m_wallSpacing, m_imageNumFront, m_imageNumBack);
        m_appenderTranslate->AddInputData(polydataWall);


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

        m_actorTranslate->SetScale(actorScale);
        m_actorRotate->SetScale(actorScale);

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
