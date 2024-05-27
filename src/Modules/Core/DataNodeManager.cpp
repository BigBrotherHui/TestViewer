#include "DataNodeManager.h"
#include <mitkProperties.h>
#include <mitkImage.h>

template <DataNodeManager::DataType T>
mitk::DataNode::Pointer DataNodeManager::createNodeWithName(const std::string& name)
{
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetName(name);
    mitk::BaseData::Pointer baseData;
    switch (T) {
        case DataType_Point:
        case DataType_Line:
            baseData = mitk::PointSet::New();
            break;
        case DataType_Mesh:
            baseData = mitk::Surface::New();
            break;
        case DataType_Volume:
            baseData = mitk::Image::New();
            break;
    }
    node->SetData(baseData);
    return node;
}

mitk::DataNode::Pointer DataNodeManager::constructDataNode(vtkImageData* pImage, const std::string& name)
{
    mitk::DataNode::Pointer node =
        createNodeWithName<DataType::DataType_Volume>(name);
    if (!node) {
        Error(name + "failed to create node");
        return nullptr;
    }
    mitk::Image* image = castToImage(node->GetData());
    if (!image) {
        Error("failed to cast to image");
        return nullptr;
    }
    image->Initialize(pImage);
    image->SetVolume(pImage->GetScalarPointer());
    return node;
}

mitk::DataNode::Pointer DataNodeManager::constructDataNode(vtkPolyData* pPolyData, const std::string& name)
{
    mitk::DataNode::Pointer node =
        createNodeWithName<DataType::DataType_Mesh>(name);
    if (!node) {
        Error(name + "failed to create node");
        return nullptr;
    }
    mitk::Surface* surface = castToSurface(node->GetData());
    if (!surface) {
        Error("failed to cast to surface");
        return nullptr;
    }
    surface->SetVtkPolyData(pPolyData);
    node->SetProperty("material.specularCoefficient", mitk::FloatProperty::New(0.0f));
    return node;
}

template <typename T>
mitk::DataNode::Pointer DataNodeManager::constructDataNode(const typename T::Pointer pImage, const std::string& name)
{
    mitk::DataNode::Pointer node =
        createNodeWithName<DataType::DataType_Volume>(name);
    if (!node) {
        Error(name + "failed to create node");
        return nullptr;
    }
    mitk::Image* image = DataNodeManagerPrivate::castToImage(node->GetData());
    if (!image) {
        Error("failed to cast to image");
        return nullptr;
    }
    image->InitializeByItk(pImage);
    image->SetVolume(pImage->GetBufferPointer());
    return node;
}

template <typename T>
mitk::DataNode::Pointer DataNodeManager::constructDataNode(const T* const pt, const std::string& name)
{
    mitk::DataNode::Pointer node =
        createNodeWithName<DataType::DataType_Point>(name);
    if (!node) {
        Error(name + "failed to create node");
        return nullptr;
    }
    mitk::PointSet* pointSet = castToPoint(node->GetData());
    if (!pointSet) {
        Error("failed to cast to pointSet");
        return nullptr;
    }
    mitk::Point<T, 3> mpt(pt);
    pointSet->SetPoint(0, mpt);
    return node;
}

template <typename T>
mitk::DataNode::Pointer DataNodeManager::constructDataNode(const T* const startPt, const T* const endPt,
                                                           const std::string& name)
{
    mitk::DataNode::Pointer node = createNodeWithName<DataType::DataType_Line>(name);
    if (!node) {
        Error(name + "failed to create node");
        return nullptr;
    }
    mitk::PointSet* pointSet = castToLine(node->GetData());
    if (!pointSet) {
        Error("failed to cast to pointSet(line)");
        return nullptr;
    }
    mitk::Point<T, 3> mptStart(startPt);
    mitk::Point<T, 3> mptEnd(endPt);
    pointSet->SetPoint(0, mptStart);
    pointSet->SetPoint(1, mptEnd);
    node->SetProperty("show contour", mitk::BoolProperty::New(true));
    return node;
}

void DataNodeManager::transformDataNode(mitk::DataNode::Pointer pNode, vtkMatrix4x4* const vmt)
{
    if (!pNode) return;
    mitk::BaseData::Pointer data = pNode->GetData();
    if (!data) return;
    mitk::BaseGeometry::Pointer geo = data->GetGeometry();
    if (!geo) return;
    geo->SetIndexToWorldTransformByVtkMatrix(vmt);
    std::cout << "transformDataNode success " << std::endl;
}

void DataNodeManager::transformDataNode(mitk::DataNode::Pointer pNode, Eigen::Matrix4d mt)
{
    vtkSmartPointer<vtkMatrix4x4> vmt = vtkSmartPointer<vtkMatrix4x4>::New();
    EigenToVtkMatrix4x4(mt, vmt);
    transformDataNode(pNode, vmt);
}

void DataNodeManager::getDataNodeTransform(mitk::DataNode::Pointer pNode, Eigen::Matrix4d& mt)
{
    if (!pNode) return;
    mitk::BaseData::Pointer data = pNode->GetData();
    if (!data) return;
    mitk::BaseGeometry::Pointer geo = data->GetGeometry();
    if (!geo) return;
    vtkMatrix4x4* vmt = geo->GetVtkMatrix();
    VtkMatrix4x4ToEigen(mt, vmt);
}

void DataNodeManager::getDataNodeTransform(mitk::DataNode::Pointer pNode, vtkMatrix4x4* const vmt)
{
    if (!vmt) return;
    if (!pNode) return;
    mitk::BaseData::Pointer data = pNode->GetData();
    if (!data) return;
    mitk::BaseGeometry::Pointer geo = data->GetGeometry();
    if (!geo) return;
    vmt->DeepCopy(geo->GetVtkMatrix());
}

void DataNodeManager::setDataNodeVisibility(mitk::DataNode::Pointer pNode, bool v)
{
    if (!pNode) return;
    pNode->SetVisibility(v);
}

bool DataNodeManager::getDataNodeVisibility(mitk::DataNode::Pointer pNode)
{
    if (!pNode) return 0;
    bool v;
    pNode->GetBoolProperty("visible", v);
    return v;
}

void DataNodeManager::setDataNodeOpacity(mitk::DataNode::Pointer pNode, float opacity)
{
    if (!pNode) return;
    pNode->SetOpacity(opacity);
}

float DataNodeManager::getDataNodeOpacity(mitk::DataNode::Pointer pNode)
{
    if (!pNode) return 0.0;
    float v;
    pNode->GetFloatProperty("opacity", v);
    return v;
}

void DataNodeManager::setDataNodeColor(mitk::DataNode::Pointer pNode, const float* const color)
{
    if (!pNode) return;
    pNode->SetColor(color);
}

void DataNodeManager::setDataNodeColor(mitk::DataNode::Pointer pNode, float r, float g, float b)
{
    if (!pNode) return;
    pNode->SetColor(r, g, b);
}

float* DataNodeManager::getDataNodeColor(mitk::DataNode::Pointer pNode)
{
    if (!pNode) return nullptr;
    float* color = new float[3];
    pNode->GetColor(color);
    return color;
}

void DataNodeManager::getDataNodeColor(mitk::DataNode::Pointer pNode, float* color)
{
    if (!pNode) return;
    pNode->GetColor(color);
}

mitk::Surface* DataNodeManager::castToSurface(mitk::BaseData::Pointer data)
{
    return dynamic_cast<mitk::Surface*>(data.GetPointer());
}

mitk::Image* DataNodeManager::castToImage(mitk::BaseData::Pointer data)
{
    return dynamic_cast<mitk::Image*>(data.GetPointer());
}

mitk::PointSet* DataNodeManager::castToPoint(mitk::BaseData::Pointer data)
{
    return dynamic_cast<mitk::PointSet*>(data.GetPointer());
}

mitk::PointSet* DataNodeManager::castToLine(mitk::BaseData::Pointer data)
{
    return dynamic_cast<mitk::PointSet*>(data.GetPointer());
}

void DataNodeManager::EigenToVtkMatrix4x4(Eigen::Matrix4d mt, vtkMatrix4x4* vmt)
{
    memcpy(vmt->GetData(), mt.data(), sizeof(double) * 16);
    vmt->Transpose();
}

void DataNodeManager::VtkMatrix4x4ToEigen(Eigen::Matrix4d& mt, vtkMatrix4x4* vmt)
{
    memcpy(mt.data(), vmt->GetData(), sizeof(double) * 16);
    mt.transposeInPlace();
}
