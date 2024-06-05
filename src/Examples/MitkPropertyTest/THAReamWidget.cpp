#include "THAReamWidget.h"
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkSmartPointer.h>
#include <vtkImageStencil.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkSphereSource.h>
#include <vtkPointData.h>
#include <vtkDiscreteFlyingEdges3D.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkInformation.h>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QHBoxLayout>
#include "CustomSurfaceVtkMapper3D.h"
#include <vtkClipPolyData.h>
#include <vtkBox.h>

namespace {

vtkSmartPointer<vtkPolyData> transformPolyData(vtkMatrix4x4 *mt,
                                                           vtkPolyData* p)
{
    if (!p) return nullptr;
    if (!mt) mt = vtkSmartPointer<vtkMatrix4x4>::New();
    vtkSmartPointer<vtkPolyData> ret = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
    transform->SetMatrix(mt);
    vtkSmartPointer<vtkTransformPolyDataFilter> fi = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    fi->SetTransform(transform);
    fi->SetInputData(p);
    fi->Update();
    vtkSmartPointer<vtkPolyData> out = vtkSmartPointer<vtkPolyData>::New();
    out->DeepCopy(fi->GetOutput());
    return out;
}
vtkSmartPointer<vtkImageData> _generateImageData(double *spacing,double *origin,int *extent,int *dimensions)
{
    vtkSmartPointer<vtkImageData> ret = vtkSmartPointer<vtkImageData>::New();
    vtkSmartPointer<vtkInformation> in = vtkSmartPointer<vtkInformation>::New();
    ret->SetScalarType(VTK_UNSIGNED_CHAR, in);
    ret->SetSpacing(spacing[0], spacing[1], spacing[2]);
    ret->SetOrigin(origin[0], origin[1], origin[2]);
    ret->SetExtent(extent);
    ret->SetDimensions(dimensions);
    ret->SetNumberOfScalarComponents(1, in);
    ret->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
    vtkIdType count = ret->GetNumberOfPoints();
#pragma omp parallel for
    for (vtkIdType i = 0; i < count; ++i) {
        ret->GetPointData()->GetScalars()->SetTuple1(i, 1);
    }
    return ret;
}
vtkSmartPointer<vtkImageData> _polyDataToImageData(vtkSmartPointer<vtkPolyData> polydata, double *spacing,
                                                  double *origin, int *extent, int *dimensions)
{
    if (!polydata) return nullptr;
    vtkSmartPointer<vtkImageData> imageData = _generateImageData(spacing, origin, extent, dimensions);
    unsigned char outval = 0;
    vtkSmartPointer<vtkPolyDataToImageStencil> pti = vtkSmartPointer<vtkPolyDataToImageStencil>::New();
    pti->SetInputData(polydata);
    pti->SetOutputOrigin(origin);
    pti->SetOutputSpacing(spacing);
    pti->SetOutputWholeExtent(extent);
    pti->Update();
    vtkSmartPointer<vtkImageStencil> stencil = vtkSmartPointer<vtkImageStencil>::New();
    stencil->SetInputData(imageData);
    stencil->SetStencilData(pti->GetOutput());
    stencil->ReverseStencilOff();
    stencil->SetBackgroundValue(outval);
    stencil->Update();
    vtkSmartPointer<vtkImageData> out = vtkSmartPointer<vtkImageData>::New();
    out->DeepCopy(stencil->GetOutput());
    return out;
}
vtkSmartPointer<vtkPolyData> generateSphere(int radius,double *center=nullptr)
{
    vtkSmartPointer<vtkSphereSource> spGreen = vtkSmartPointer<vtkSphereSource>::New();
    if (center)
        spGreen->SetCenter(center);
    spGreen->SetRadius(radius);
    spGreen->SetThetaResolution(40);
    spGreen->SetPhiResolution(40);
    spGreen->Update();
    vtkSmartPointer<vtkPolyData> out = vtkSmartPointer<vtkPolyData>::New();
    out->DeepCopy(spGreen->GetOutput());
    return out;
}
}  // namespace

class THAReamWidget::Impl {
public:
    QmitkRenderWindow *m_renderwindow;
    bool m_dataComplete{1};
    double m_origin[3], m_spacing[3]{0.5,0.5,0.5};
    int m_extent[6], m_dimensions[3];
    double m_cupCenter[3];
    int m_toolRadius;
    vtkSmartPointer<vtkPolyData> m_sourcePolyData{nullptr};
    //use to show
    vtkSmartPointer<vtkPolyData> m_toolPolyData{nullptr};
    //use to ream
    vtkSmartPointer<vtkPolyData> m_realToolPolyData{nullptr};
    vtkSmartPointer<vtkPolyData> m_Green{nullptr};
    vtkSmartPointer<vtkPolyData> m_Safe{nullptr};
    //待处理骨盆部分mesh
    std::string m_resultNodeName = "result";
    //其它骨盆部分mesh
    std::string m_resultCombineNodeName = "resultCombine";
    //骨盆图像
    std::string m_resultImageNodeName = "resultImage";
    //打磨工具
    std::string m_toolName = "tool";
    QFutureWatcher<void> m_futureWatcher;
    THAReamWidget::Impl()
    {
        memset(m_extent, 0, sizeof(int) * 6);
    }
};

THAReamWidget::THAReamWidget(QWidget *parent) : QWidget(parent),m_impl(new Impl)
{
    m_impl->m_renderwindow=createSingleWidget(this);
    setWidgetType(m_impl->m_renderwindow, WidgetType_3D);
    configRenderWindow(m_impl->m_renderwindow);
    QHBoxLayout *l = new QHBoxLayout(this);
    l->addWidget(m_impl->m_renderwindow);
    connect(&m_impl->m_futureWatcher, &QFutureWatcher<void>::finished, this, [&] { requestUpdateRender(); });
}

THAReamWidget::~THAReamWidget()
{
    if (m_impl)
    {
        delete m_impl;
        m_impl = nullptr;
    } 
}

void THAReamWidget::setSourcePolyData(vtkPolyData* src, vtkMatrix4x4* vmt)
{
    if (!src) {
        m_impl->m_dataComplete = 0;
        return;
    }
    if (vmt)
        m_impl->m_sourcePolyData = transformPolyData(vmt, src);
    else {
        m_impl->m_sourcePolyData = vtkSmartPointer<vtkPolyData>::New();
        m_impl->m_sourcePolyData->DeepCopy(src);
    }   
}

void THAReamWidget::setCupRadius(unsigned int radius,double *cupCenter)
{
    if (radius == 0 || !cupCenter) {
        m_impl->m_dataComplete = 0;
        return;
    }
    m_impl->m_Green = generateSphere(radius, cupCenter);
    m_impl->m_Safe = generateSphere(radius + 1, cupCenter);

    double *bounds = m_impl->m_Safe->GetBounds();
    m_impl->m_dimensions[0] = std::ceil((bounds[1] - bounds[0]) / m_impl->m_spacing[0]) + 50;
    m_impl->m_dimensions[1] = std::ceil((bounds[3] - bounds[2]) / m_impl->m_spacing[1]) + 50;
    m_impl->m_dimensions[2] = std::ceil((bounds[5] - bounds[4]) / m_impl->m_spacing[2]) + 50;
    m_impl->m_extent[1] = m_impl->m_dimensions[0];
    m_impl->m_extent[3] = m_impl->m_dimensions[1];
    m_impl->m_extent[5] = m_impl->m_dimensions[2];
    m_impl->m_origin[0] = 0.5 * m_impl->m_spacing[0] + bounds[0] - 20;
    m_impl->m_origin[1] = 0.5 * m_impl->m_spacing[1] + bounds[2] - 20;
    m_impl->m_origin[2] = 0.5 * m_impl->m_spacing[2] + bounds[4] - 20;
    m_impl->m_cupCenter[0] = cupCenter[0];
    m_impl->m_cupCenter[1] = cupCenter[1];
    m_impl->m_cupCenter[2] = cupCenter[2];
}

void THAReamWidget::setTool(unsigned int toolRadius,vtkPolyData *src, vtkMatrix4x4 *vmt)
{
    if (!src || toolRadius==0) {
        m_impl->m_dataComplete = 0;
        return;
    }
    if (vmt)
        m_impl->m_toolPolyData = transformPolyData(vmt, src);
    else {
        m_impl->m_toolPolyData = vtkSmartPointer<vtkPolyData>::New();
        m_impl->m_toolPolyData->DeepCopy(src);
    }
    m_impl->m_toolRadius = toolRadius;
    m_impl->m_realToolPolyData = generateSphere(m_impl->m_toolRadius);
}

void THAReamWidget::setToolPosture(vtkMatrix4x4* vmt)
{
    if (!vmt || !m_impl->m_dataComplete) return;
    auto node = getDataNodeByName(m_impl->m_toolName);
    if (node)
    {
        transformDataNode(node, vmt);
    }
    else
    {
        auto node = constructDataNode(m_impl->m_toolPolyData, m_impl->m_toolName);
        addDataNode(node,true);
    }
    requestUpdateRender();
}

void THAReamWidget::renderResult()
{
    if (!m_impl->m_dataComplete)
        return;
    auto pelvisImage = polyDataToImageData(m_impl->m_sourcePolyData);
    auto result = generateImageData();
    int radius = m_impl->m_toolRadius+1;
#pragma omp parallel for
    for (int i = m_impl->m_extent[0]; i < m_impl->m_extent[1]; i++) {
        for (int j = m_impl->m_extent[2]; j < m_impl->m_extent[3]; j++) {
            for (int k = m_impl->m_extent[4]; k < m_impl->m_extent[5]; k++) {
                uchar *pAll = (uchar *)(result->GetScalarPointer(i, j, k));
                uchar *pPelvis = (uchar *)(pelvisImage->GetScalarPointer(i, j, k));
                if (*pPelvis == 0) {
                    *pAll = 0;
                    continue;
                }
                double pixelPos[3] = {i * m_impl->m_spacing[0] + m_impl->m_spacing[0] / 2+m_impl->m_origin[0],
                                      j * m_impl->m_spacing[1] + m_impl->m_spacing[1] / 2+m_impl->m_origin[1],
                                      k * m_impl->m_spacing[2] + m_impl->m_spacing[2] / 2+m_impl->m_origin[2]};
                const double dis = vtkMath::Distance2BetweenPoints(pixelPos, m_impl->m_cupCenter);
                if (dis < radius * radius) {
                    int _radius = radius - 1;
                    if (dis < _radius * _radius) {
                        *pAll = 2;
                    }
                }
                else
                {
                    *pAll = 1;
                }
            }
        }
    }
    result->Modified();

    //extract iso 
    vtkSmartPointer<vtkDiscreteFlyingEdges3D> flyingEdges3D = vtkDiscreteFlyingEdges3D ::New();
    flyingEdges3D->SetNumberOfContours(2);
    flyingEdges3D->SetValue(0, 1);
    flyingEdges3D->SetValue(1, 2);
    flyingEdges3D->SetComputeGradients(false);
    flyingEdges3D->SetComputeNormals(false);
    flyingEdges3D->SetComputeScalars(true);
    flyingEdges3D->SetInputData(result);
    flyingEdges3D->Update();
    //smooth
    vtkSmartPointer<vtkSmoothPolyDataFilter> smooth = vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
    smooth->SetInputData(flyingEdges3D->GetOutput());
    smooth->SetNumberOfIterations(10);
    smooth->SetRelaxationFactor(.5);
    smooth->Update();

    vtkSmartPointer<vtkClipPolyData> clipper = vtkSmartPointer<vtkClipPolyData>::New();
    vtkSmartPointer<vtkBox> box = vtkSmartPointer<vtkBox>::New();
    box->SetBounds(smooth->GetOutput()->GetBounds());
    clipper->SetClipFunction(box);
    clipper->SetInputData(m_impl->m_sourcePolyData);
    clipper->Update();

    if (getDataNodeByName(m_impl->m_resultCombineNodeName)) {
        coverSurface(getDataNodeByName(m_impl->m_resultCombineNodeName), clipper->GetOutput());
    }
    else {
        auto resultNode = constructDataNode(clipper->GetOutput(), m_impl->m_resultCombineNodeName);
        addDataNode(resultNode, true);
    }

    if (getDataNodeByName(m_impl->m_resultNodeName)) {
        coverSurface(getDataNodeByName(m_impl->m_resultNodeName), smooth->GetOutput());
    }
    else {
        auto resultNode = constructDataNode(smooth->GetOutput(), m_impl->m_resultNodeName);
        mitk::CustomSurfaceVtkMapper3D::Pointer mapper = mitk::CustomSurfaceVtkMapper3D::New();
        resultNode->SetMapper(2, mapper);
        addDataNode(resultNode, true);
    }

    if (getDataNodeByName(m_impl->m_resultImageNodeName)) {
        coverImage(getDataNodeByName(m_impl->m_resultImageNodeName), result);
    }
    else {
        auto resultNode = constructDataNode(result, m_impl->m_resultImageNodeName);
        addDataNode(resultNode, true);
        setDataNodeVisibility(resultNode,false);
        mitk::Point3D origin=castToImage(resultNode->GetData())->GetGeometry()->GetOrigin();
        castToImage(resultNode->GetData())->GetVtkImageData()->SetOrigin(origin[0],origin[1],origin[2]);
        castToImage(resultNode->GetData())->GetGeometry()->SetOrigin(mitk::Point3D());
    }
    
    requestUpdateRender();
}

void THAReamWidget::runReam()
{
    if (!m_impl->m_futureWatcher.isFinished()) return;
    auto future = QtConcurrent::run([&] {
        auto resultNode = getDataNodeByName(m_impl->m_resultNodeName);
        auto toolNode = getDataNodeByName(m_impl->m_toolName);
        auto resultImageNode = getDataNodeByName(m_impl->m_resultImageNodeName);
        if (!resultNode || !toolNode || !resultImageNode) return;
        vtkSmartPointer<vtkMatrix4x4> toolMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
        getDataNodeTransformByName(m_impl->m_toolName, toolMatrix);

        auto resultImage = castToImage(resultImageNode->GetData());
        vtkImageData *resultImageData = resultImage->GetVtkImageData();

        double center[3]{toolMatrix->GetElement(0, 3), toolMatrix->GetElement(1, 3), toolMatrix->GetElement(2,3)};
        int radius = m_impl->m_toolRadius+1;
#pragma omp parallel for
        for (int i = m_impl->m_extent[0]; i < m_impl->m_extent[1]; i++) {
            for (int j = m_impl->m_extent[2]; j < m_impl->m_extent[3]; j++) {
                for (int k = m_impl->m_extent[4]; k < m_impl->m_extent[5]; k++) {
                    uchar *pAll = (uchar *)(resultImageData->GetScalarPointer(i, j, k));
                    int vv = *pAll;
                    if (vv==0)
                        continue;
                    double pixelPos[3] = {i * m_impl->m_spacing[0] + m_impl->m_spacing[0] / 2 + m_impl->m_origin[0],
                                          j * m_impl->m_spacing[1] + m_impl->m_spacing[1] / 2 + m_impl->m_origin[1],
                                          k * m_impl->m_spacing[2] + m_impl->m_spacing[2] / 2 + m_impl->m_origin[2]};
                    double dis = vtkMath::Distance2BetweenPoints(pixelPos,m_impl->m_cupCenter);
                    double _dis = vtkMath::Distance2BetweenPoints(pixelPos, center);
                    if (_dis < (radius - 1) * (radius - 1)) {
                        *pAll = 0;
                    }
                    else {
                        if(dis < radius * radius)
                            continue;
                        if (*pAll == 1 && _dis < radius * radius) {
                            *pAll = 3;
                        }
                    }
                }
            }
        }
        resultImageData->Modified();
        vtkSmartPointer<vtkDiscreteFlyingEdges3D> flyingEdges3D = vtkDiscreteFlyingEdges3D ::New();
        flyingEdges3D->SetNumberOfContours(3);
        flyingEdges3D->SetValue(0, 1);
        flyingEdges3D->SetValue(1, 2);
        flyingEdges3D->SetValue(2, 3);
        flyingEdges3D->SetComputeGradients(false);
        flyingEdges3D->SetComputeNormals(false);
        flyingEdges3D->SetComputeScalars(true);
        flyingEdges3D->SetInputData(resultImageData);
        flyingEdges3D->Update();
        // smooth
        vtkSmartPointer<vtkSmoothPolyDataFilter> smooth = vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
        smooth->SetInputData(flyingEdges3D->GetOutput());
        smooth->SetNumberOfIterations(10);
        smooth->SetRelaxationFactor(.5);
        smooth->Update();
        castToSurface(resultNode->GetData())->SetVtkPolyData(smooth->GetOutput());
    });
    m_impl->m_futureWatcher.setFuture(future);
}

vtkSmartPointer<vtkImageData> THAReamWidget::polyDataToImageData(vtkPolyData *polydata)
{
    if (!polydata || !m_impl->m_dataComplete) return nullptr;
    return _polyDataToImageData(polydata,m_impl->m_spacing,m_impl->m_origin,m_impl->m_extent,m_impl->m_dimensions);
}

vtkSmartPointer<vtkImageData> THAReamWidget::generateImageData()
{
    return _generateImageData(m_impl->m_spacing, m_impl->m_origin, m_impl->m_extent, m_impl->m_dimensions);
}
