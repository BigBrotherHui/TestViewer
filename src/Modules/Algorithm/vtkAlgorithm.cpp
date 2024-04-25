#include "vtkAlgorithm.h"
#include <math.h>
#include <Eigen/Eigen>
#include <vtkRenderWindow.h>
#include <vtkCellData.h>
#include <vtkMath.h>
#include <vtkImplicitPolyDataDistance.h>
#include <vtkSMPTools.h>
#include "ClipPolyData.h"
#include <vtkBox.h>
#include <QDebug>
#include <vtkClipPolyData.h>
#include <vtkLoopSubdivisionFilter.h>
#include <vtkTriangleFilter.h>
#include <vtkLinearSubdivisionFilter.h>
#include <vtkExtractVOI.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkImageStencil.h>
#define M_PI 3.1415926
void calculateTranslateX(double &x, double &y, double angle)
{
    double nx = x * cos(angle) + y * sin(angle);
    x = nx * cos(angle);
    y = nx * sin(angle);
}
void calculateTranslateY(double &x, double &y, double angle)
{
    double ny = x * -sin(angle) + y * cos(angle);
    x = -ny * sin(angle);
    y = ny * cos(angle);
}

int calculateCursorState(double pangle)
{
    double angle = fmod(pangle, 180);
    if ((fabs(angle) > 180.0 - 22.5) || (fabs(angle) < 22.5)) {
        return VTK_CURSOR_SIZENS;
    }
    if (fabs(fabs(angle) - 90.0) < 22.5) {
        return VTK_CURSOR_SIZEWE;
    }
    if ((fabs(angle - 135.0) < 22.5) || (fabs(angle + 45.0) < 22.5)) {
        return VTK_CURSOR_SIZENE;
    }
    return VTK_CURSOR_SIZENW;
}

Eigen::Vector4d fitPlane(const Eigen::MatrixX3d& points)
{
    Eigen::Vector4d out;
    if (points.rows() < 3) {
        std::cout << "at least input 3 points" << std::endl;
        return out;
    }
    Eigen::Vector3d center = points.colwise().mean().eval();
    Eigen::MatrixX3d tmp = points;
    tmp.rowwise() -= center.transpose();

    Eigen::JacobiSVD<Eigen::MatrixX3d> svd(tmp, Eigen::ComputeThinV);
    out.head(3) = svd.matrixV().block<3, 1>(0, 2);
    out[3] = -(out[0] * center[0] + out[1] * center[1] + out[2] * center[2]);

    return out;
}

// Step 2: Project points onto the fitted plane
Eigen::MatrixX3d projectPoints(const Eigen::MatrixX3d& points, const Eigen::Vector4d& parameters)
{
    int numPoints = points.rows();
    Eigen::MatrixX3d projectedPoints(numPoints, 3);

    // Compute the distance from each point to the plane
    for (int i = 0; i < numPoints; ++i) {
        Eigen::Vector3d point = points.row(i);
        float t = -(parameters[0] * point[0] + parameters[1] * point[1] + parameters[2] * point[2] + parameters[3]) /
                  (parameters[0] * parameters[0] + parameters[1] * parameters[1] + parameters[2] * parameters[2]);
        projectedPoints.row(i)=Eigen::Vector3d(point[0] + parameters[0] * t, point[1] + parameters[1] * t, point[2] + parameters[2] * t);
    }

    return projectedPoints;
}

void ProjectToPlane(const double x[3], const double origin[3], const double normal[3], double xproj[3])
{
    double t, xo[3];

    xo[0] = x[0] - origin[0];
    xo[1] = x[1] - origin[1];
    xo[2] = x[2] - origin[2];

    Eigen::Vector3d nl(normal);
    nl.normalize();
    Eigen::Vector3d xv(xo);

    t = xv.dot(nl);

    xproj[0] = x[0] - t * nl[0];
    xproj[1] = x[1] - t * nl[1];
    xproj[2] = x[2] - t * nl[2];
}

// Step 3: Perform SVD transformation
Eigen::MatrixX3d svdTransform(const Eigen::MatrixX3d& points)
{
    Eigen::JacobiSVD<Eigen::MatrixX3d> svd(points, Eigen::ComputeThinU | Eigen::ComputeThinV);
    Eigen::MatrixX3d transformedPoints = svd.matrixU().transpose() * points; 
    return transformedPoints;
}

// Step 4: Center the points
Eigen::MatrixX3d centerPoints(const Eigen::MatrixX3d& points)
{
    Eigen::Vector3d mean = points.colwise().mean();
    Eigen::MatrixX3d centeredPoints = points.rowwise() - mean.transpose();
    return centeredPoints;
}

// Step 5: Calculate angle of each point with respect to origin
Eigen::VectorXf calculateAngles(const Eigen::MatrixX3d& points)
{
    int numPoints = points.rows();
    Eigen::VectorXf angles(numPoints);
    for (int i = 0; i < numPoints; ++i) {
        angles(i) = std::atan2(points(i, 1), points(i, 0)) * 180 / M_PI;
        if (angles(i) < 0) angles(i) += 360;  // Ensure angle is in [0, 360) range
    }
    return angles;
}

// Step 6: Renumber the points based on angle
Eigen::VectorXi renumberPoints(const Eigen::VectorXf& angles)
{
    // Initialize vector with indices from 0 to numPoints-1
    Eigen::VectorXi indices = Eigen::VectorXi::LinSpaced(angles.size(), 0, angles.size() - 1);
    // Sort indices based on angles
    std::sort(indices.data(), indices.data() + indices.size(), [&](int i, int j) { return angles(i) < angles(j); });
    return indices;
}

void sortVtkPoints(vtkPoints* vtkpoints)
{
    int numPoints = vtkpoints->GetNumberOfPoints();
    Eigen::MatrixX3d points(numPoints, 3);
    // Populate matrix with point coordinates
    for (int i = 0; i < numPoints; ++i) {
        points.row(i) << vtkpoints->GetPoint(i)[0], vtkpoints->GetPoint(i)[1], vtkpoints->GetPoint(i)[2];
    }

    // Step 1: Perform SVD transformation
    /*Eigen::MatrixX3d transformedPoints = svdTransform(points);

    for (int i = 0; i < transformedPoints.rows(); ++i)
    {
        transformedPoints.row(i)[2] = 0;
    }*/
    // Step 2: Fit points to a plane
    Eigen::Vector4d planeParameters = fitPlane(points);

    // Step 3: Project points onto the fitted plane
    Eigen::MatrixX3d projectedPoints = projectPoints(points, planeParameters);

    // Step 4: Center the points
    Eigen::MatrixX3d centeredPoints = centerPoints(projectedPoints);

    // Step 5: Calculate angle of each point with respect to origin
    Eigen::VectorXf angles = calculateAngles(centeredPoints);

    // Step 6: Renumber the points based on angle
    Eigen::VectorXi renumberedIndices = renumberPoints(angles);

    // Create a vector of points sorted by angle
    vtkPoints* sortedPoints = vtkPoints::New();
    sortedPoints->SetNumberOfPoints(numPoints);
    for (int i = 0; i < numPoints; ++i) {
        int index = renumberedIndices(i);
        sortedPoints->SetPoint(i,vtkpoints->GetPoint(index));
    }
    vtkpoints->DeepCopy(sortedPoints);
    sortedPoints->Delete();
}

//input1为碰撞者 input2为被碰撞者
vtkSmartPointer<vtkPolyData> extractCollideCellids(vtkPolyData* input1, vtkPolyData* input2,int& totalCollide,
                                                   bool caculateAll)
{
    vtkSmartPointer<vtkPolyData> copy = vtkSmartPointer<vtkPolyData>::New();
    if (!caculateAll) {
        vtkNew<vtkBox> box;
        double* bounds = input1->GetBounds();
        box->SetBounds(bounds);
        //vtkSmartPointer<vtkLinearSubdivisionFilter> loop = vtkSmartPointer<vtkLinearSubdivisionFilter>::New();
        //loop->SetInputData(input2);
        //loop->SetNumberOfSubdivisions(2);
        //loop->Update();
        vtkNew<vtkClipPolyData> cl;
        cl->SetClipFunction(box);
        cl->SetInputData(input2);
        cl->GenerateClippedOutputOn();
        cl->Update();
        vtkPolyData* clPolyData = cl->GetClippedOutput();
        copy->DeepCopy(clPolyData);
    }
    else {
        /*vtkSmartPointer<vtkLoopSubdivisionFilter> loop = vtkSmartPointer<vtkLoopSubdivisionFilter>::New();
        loop->SetInputData(input2);
        loop->SetNumberOfSubdivisions(4);
        loop->Update();*/
        copy->DeepCopy(input2);
    }
    std::unordered_map<int, double> distanceCache;
    vtkSmartPointer<vtkImplicitPolyDataDistance> distanceFilter = vtkSmartPointer<vtkImplicitPolyDataDistance>::New();
    distanceFilter->SetInput(input1);
    for (int i = 0; i < copy->GetNumberOfCells(); ++i) {
        vtkCell* cell = copy->GetCell(i);
        int numPts = cell->GetNumberOfPoints();
        vtkPoints* cellPts = cell->GetPoints();
        bool isMark{false};
        int collideNum = 0;
        for (int j = 0; j < numPts; ++j) {
            int ptid = cell->GetPointId(j);
            double* pt = cellPts->GetPoint(j);
            double distance{0.0};
            if (distanceCache.find(ptid) != distanceCache.end()) {
                distance = distanceCache[ptid];
            }
            else {
                distance = distanceFilter->EvaluateFunction(pt);
                distanceCache[ptid] = distance;
            }
            if (distance < 0) {
                collideNum++;
            }
            if (collideNum > 1) {
                isMark = true;
                break;
            }
        }
        if (isMark) {
            totalCollide++;
        }
        else {
            copy->DeleteCell(i);
        }
    }
    copy->RemoveDeletedCells();
    copy->Modified();
    vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
    colors->SetNumberOfComponents(4);
    colors->SetNumberOfTuples(copy->GetNumberOfCells());
    double markColor[4]{255, 0, 0, 255};
    for (int k = 0; k < copy->GetNumberOfCells(); ++k) {
        colors->SetTuple4(k, markColor[0], markColor[1], markColor[2], markColor[3]);
    }
    copy->GetCellData()->SetScalars(colors);
    qDebug() << "total collide cell number is:" << totalCollide;
    return copy;
}
void cropImageByPolyData(vtkSmartPointer<vtkImageData> sourceImage, vtkSmartPointer<vtkImageData> dstImage,
                    vtkSmartPointer<vtkPolyData> polydata)
{
    if (!sourceImage || !dstImage || !polydata) {
        return;
    }

    double bounds[6];
    polydata->GetBounds(bounds);

    // 获取 imageData 的原点、间距和范围
    double origin[3];
    double spacing[3];
    int extent[6];
    sourceImage->GetOrigin(origin);
    sourceImage->GetSpacing(spacing);
    sourceImage->GetExtent(extent);

    // 计算索引坐标
    int indexBounds[6];
    for (int i = 0; i < 3; ++i) {
        indexBounds[2 * i] = std::ceil((bounds[2 * i] - origin[i]) / spacing[i]);
        indexBounds[2 * i + 1] = std::ceil((bounds[2 * i + 1] - origin[i]) / spacing[i]);

        // 确保索引坐标在有效范围内
        indexBounds[2 * i] = std::max(indexBounds[2 * i], extent[2 * i]);
        indexBounds[2 * i + 1] = std::min(indexBounds[2 * i + 1], extent[2 * i + 1]);
    }

    // 打印结果
    /*std::cout << "Index Bounds: [" << indexBounds[0] << ", " << indexBounds[1] << ", " << indexBounds[2] << ", "
              << indexBounds[3] << ", " << indexBounds[4] << ", " << indexBounds[5] << "]" << std::endl;*/

    // 首先需要将vtkPolyData转换为vtkImageStencilData
    vtkSmartPointer<vtkPolyDataToImageStencil> polyDataToStencil = vtkSmartPointer<vtkPolyDataToImageStencil>::New();
    polyDataToStencil->SetInputData(polydata);
    polyDataToStencil->SetOutputSpacing(sourceImage->GetSpacing());
    polyDataToStencil->SetOutputOrigin(sourceImage->GetOrigin());
    polyDataToStencil->SetOutputWholeExtent(sourceImage->GetExtent());
    polyDataToStencil->Update();

    // 将原始图像中的蒙板区域外部的像素设置为0
    vtkSmartPointer<vtkImageStencil> imageStencil = vtkSmartPointer<vtkImageStencil>::New();
    imageStencil->SetInputData(sourceImage);
    imageStencil->SetStencilData(polyDataToStencil->GetOutput());
    imageStencil->ReverseStencilOff();
    imageStencil->SetBackgroundValue(0);
    imageStencil->Update();

    vtkSmartPointer<vtkExtractVOI> extractVOI = vtkSmartPointer<vtkExtractVOI>::New();
    extractVOI->SetInputData(imageStencil->GetOutput());
    extractVOI->SetVOI(indexBounds[0], indexBounds[1], indexBounds[2], indexBounds[3], indexBounds[4],
                       indexBounds[5]);  // 指定感兴趣区域
    extractVOI->Update();

    dstImage->DeepCopy(extractVOI->GetOutput());

    return;
}
