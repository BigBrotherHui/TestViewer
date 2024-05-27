
#pragma once

#include <vtkProp3d.h>
#include "DataNodeManager.h"
#include "LayoutManager.h"
#include "IOHelper.h"
#include "CoreExports.h"
class QmitkRenderWindow;
class MitkWidgetBasePrivate;

class Core_EXPORT MitkWidgetBase : public DataNodeManager, public LayoutManager, public IOHelper {
public:
    enum DataType
    {
        Image,Mesh
    };
    MitkWidgetBase();
    virtual ~MitkWidgetBase();
    // 初始化
    void init();
    // 配置单渲染窗口
    void configRenderWindow(QmitkRenderWindow *renderwindow);
    void configMPRWidget(QmitkStdMultiWidget *widget);

    void setWidgetPlaneVisibility(int index,bool v);
    void setSurfaceNodeDistance(mitk::DataNode::Pointer node,double distance);
    void computeBounds(bool resetCamera=true);
    void computeBounds(DataType type, bool resetCamera = true);
    // 渲染
    void requestUpdateRender();

    // 添加、获取、移除、判断DataNode是否存在
    void addDataNode(mitk::DataNode::Pointer pNode, bool computeBounds = false);
    void removeDataNode(mitk::DataNode::Pointer pNode, bool computeBounds = false);
    void removeDataNodeByName(const std::string &name, bool computeBounds = false);
    bool isDataNodeExist(mitk::DataNode::Pointer pNode);
    bool isDataNodeExistByName(const std::string &name);
    mitk::DataNode::Pointer getDataNodeByName(const std::string &pName);

    // 兼容vtkActor:添加、删除、获取、判断是否存在
    void addActor(vtkProp3D *actor, const std::string &name,bool cover=false);
    void removeActor(vtkProp3D *actor);
    void removeActorByName(const std::string &name);
    bool isActorExist(vtkProp3D *actor);
    bool isActorExist(const std::string &name);
    vtkProp3D *getActor(const std::string &name);

    // 变换、获取数据空间位置
    void transformDataNodeByName(const std::string &name, vtkMatrix4x4 *const vmt);
    void transformDataNodeByName(const std::string &name, Eigen::Matrix4d mt);
    void transformActorByName(std::string &name, Eigen::Matrix4d &mt);
    void transformActorByName(std::string &name, vtkMatrix4x4 *vmt);
    void getDataNodeTransformByName(const std::string &name, Eigen::Matrix4d &mt);
    void getDataNodeTransformByName(const std::string &name, vtkMatrix4x4 *const vmt);

    // 通用属性设置
    void setDataNodeVisibilityByName(const std::string &name, bool v);
    bool getDataNodeVisibilityByName(const std::string &name);

    void setDataNodeOpacityByName(const std::string &name, float opacity);
    float getDataNodeOpacityByName(const std::string &name);

    void setDataNodeColorByName(const std::string &name, const float *const color);
    void setDataNodeColorByName(const std::string &name, float r, float g, float b);

    float *getDataNodeColorByName(const std::string &name);
    void getDataNodeColorByName(const std::string &name, float *color);

    
    //这个接口临时使用，后续会删除，外部不要调用
    mitk::DataStorage::Pointer getDataStorage() const;

private:
    MitkWidgetBasePrivate *m_d{nullptr};
};
