

#include <QApplication>

#include <vtkGenericOpenGLRenderWindow.h>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkPolyData.h>
#include <vtkAbstractWidget.h>
#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkCardinalSpline.h>
#include <vtkSplineFilter.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkProperty.h>
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);

//c = a¡Áb = £¨a.y*b.z-b.y*a.z , b.x*a.z-a.x*b.z , a.x*b.y-b.x*a.y£©
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
        // caculate normal direction
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

    auto spline = vtkCardinalSpline::New();
    spline->SetLeftConstraint(2);
    spline->SetLeftValue(0.0);
    spline->SetRightConstraint(2);
    spline->SetRightValue(0.0);

    vtkNew<vtkSplineFilter> splineFilter;
    splineFilter->SetInputData(polyData);
    splineFilter->SetSubdivideToSpecified();
    splineFilter->SetNumberOfSubdivisions(divisionNum);
    splineFilter->SetSpline(spline);
    splineFilter->Update();

    auto spline_PolyData = splineFilter->GetOutput();

    return spline_PolyData;
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


vtkSmartPointer<vtkPolyData> createPolyData(double *spacing, int imageNumFront,int imageNumBack,double *center,double *orientation)
{
    int totalNum = imageNumFront + imageNumBack;
    int totalPoints = totalNum * 2;
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
    lines->InsertNextCell(2);
    double centerLeft[3], centerRight[3];
    for (int i = 0; i < 3; ++i) {
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
    auto spline = vtkCardinalSpline::New();
    spline->SetLeftConstraint(2);
    spline->SetLeftValue(0.0);
    spline->SetRightConstraint(2);
    spline->SetRightValue(0.0);

    double segLength=spacing[0];
    vtkNew<vtkSplineFilter> spline_filter;
    spline_filter->SetSubdivideToLength();
    spline_filter->SetLength(segLength);
    spline_filter->SetInputData(p);
    spline_filter->SetSpline(spline);
    spline_filter->Update();
    auto spline_PolyData = spline_filter->GetOutput();
    double thickness = spacing[1];
    vtkSmartPointer<vtkAppendPolyData> append = vtkSmartPointer<vtkAppendPolyData>::New();
    for (int i = 0; i < imageNumFront; i++) {
        double stepSize = thickness * (-imageNumFront + i);
        auto expandedSpline = ExpandSpline(spline_PolyData, spline_PolyData->GetNumberOfPoints() - 1, stepSize);
        double direction[3];
        direction[0] = 0.0;
        direction[1] = 0.0;
        direction[2] = 1.0;
        unsigned cols = 250;
        double distance = cols * spacing[2];
        auto surfaceFront = SweepLine_2Sides(expandedSpline, direction, distance, cols);
        append->AddInputData(expandedSpline);
    }
    for (int i = 0; i < imageNumBack; i++) {
        double stepSize = thickness * i;
        auto expandedSpline = ExpandSpline(spline_PolyData, spline_PolyData->GetNumberOfPoints() - 1, stepSize);
        double direction[3];
        direction[0] = 0.0;
        direction[1] = 0.0;
        direction[2] = 1.0;
        unsigned cols = 250;
        double distance = cols * spacing[2];
        auto surfaceBack = SweepLine_2Sides(expandedSpline, direction, distance, cols);
        append->AddInputData(expandedSpline);
    }
    append->Update();
    vtkSmartPointer<vtkPolyData> result = vtkSmartPointer<vtkPolyData>::New();
    result->DeepCopy(append->GetOutput());
    return result;
}
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QVTKOpenGLNativeWidget w;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderwindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    w.setRenderWindow(renderwindow);
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    renderwindow->AddRenderer(renderer);
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    actor->SetMapper(mapper);
    double spacing[3]{0.625, 0.625, 1};
    double center[3]{0, 0, 0};
    vtkSmartPointer<vtkPolyData> polydata = createPolyData(spacing, 10, 10, center, nullptr);
    mapper->SetInputData(polydata);
    actor->GetProperty()->SetRepresentationToWireframe();
    actor->GetProperty()->SetOpacity(.7);
    actor->GetProperty()->SetColor(0, 1, 0);
    renderer->AddActor(actor);
    w.show();
    return a.exec();
}
