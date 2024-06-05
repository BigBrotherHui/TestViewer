
#pragma once 

#include <itkImage.h>
#include <mitkDataNode.h>
#include <mitkPointSet.h>
#include <mitkSurface.h>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkPolyData.h>
#include "CoreExports.h"
class Core_EXPORT DataNodeManager {
public:
    struct Error {
        Error(std::string errormsg) { errorMsg = errormsg; }
        ~Error() { std::cout << "error:" << errorMsg << std::endl; }

    private:
        std::string errorMsg;
    };

    enum DataType { DataType_Point, DataType_Line, DataType_Mesh, DataType_Volume };

    static mitk::DataNode::Pointer constructDataNode(vtkImageData *pImage, const std::string &name);

    // 由数据(vtkImageData、itkImage、vtkPolyData)构造DataNode
    template <typename T>
    static mitk::DataNode::Pointer constructDataNode(const typename T::Pointer pImage, const std::string &name);
    static mitk::DataNode::Pointer constructDataNode(vtkPolyData *pPolyData, const std::string &name);

    // 由基础类型创建DataNode
    // 创建点
    template <typename T>
    static mitk::DataNode::Pointer constructDataNode(const T *const pt, const std::string &name);
    // 创建线
    template <typename T>
    static mitk::DataNode::Pointer constructDataNode(const T *const startPt, const T *const endPt,
                                                     const std::string &name);

    static void transformDataNode(mitk::DataNode::Pointer pNode, vtkMatrix4x4 *const vmt);
    static void transformDataNode(mitk::DataNode::Pointer pNode, Eigen::Matrix4d mt);
    static void getDataNodeTransform(mitk::DataNode::Pointer pNode, Eigen::Matrix4d &mt);
    static void getDataNodeTransform(mitk::DataNode::Pointer pNode, vtkMatrix4x4 *const vmt);

    // 可见性
    static void setDataNodeVisibility(mitk::DataNode::Pointer pNode, bool v);
    static bool getDataNodeVisibility(mitk::DataNode::Pointer pNode);
    // 不透明度
    static void setDataNodeOpacity(mitk::DataNode::Pointer pNode, float opacity);
    static float getDataNodeOpacity(mitk::DataNode::Pointer pNode);

    // 颜色color range between 0 and 1
    static void setDataNodeColor(mitk::DataNode::Pointer pNode, const float *const color);
    static void setDataNodeColor(mitk::DataNode::Pointer pNode, float r, float g, float b);

    // return memory should be managed outside
    static float *getDataNodeColor(mitk::DataNode::Pointer pNode);
    static void getDataNodeColor(mitk::DataNode::Pointer pNode, float *color);

    template <DataType T>
    static mitk::DataNode::Pointer createNodeWithName(const std::string &name);

    static mitk::Surface *castToSurface(mitk::BaseData::Pointer data);
    static mitk::Image *castToImage(mitk::BaseData::Pointer data);
    static mitk::PointSet *castToPoint(mitk::BaseData::Pointer data);
    static mitk::PointSet *castToLine(mitk::BaseData::Pointer data);

        static void coverSurface(mitk::DataNode::Pointer node, vtkPolyData *vp);
    static void coverImage(mitk::DataNode::Pointer node, vtkImageData *vimg);

    static void EigenToVtkMatrix4x4(Eigen::Matrix4d mt, vtkMatrix4x4 *vmt);
    static void VtkMatrix4x4ToEigen(Eigen::Matrix4d &mt, vtkMatrix4x4 *vmt);
};