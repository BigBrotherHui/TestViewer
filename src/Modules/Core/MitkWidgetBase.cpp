#include "MitkWidgetBase.h"
//vtk
// 
#include <vtkProp3D.h>
//mitk
#include <QmitkRenderWindow.h>

#include <mitkStandaloneDataStorage.h>

#include <mitkBaseData.h>
#include <mitkRenderingManager.h>
#include "./QmitkStdMultiWidget.h"
#include <vtkSTLWriter.h>
#include <mitkSurfaceVtkMapper3D.h>
#include <vtkAssembly.h>
#include <mitkNodePredicateDataType.h>
#include <mitkImage.h>
#include <mitkSurface.h>
class MitkWidgetBasePrivate {
public:
    MitkWidgetBasePrivate();
    void createDataStorage();
    bool addRenderWindow(QmitkRenderWindow *pRenderer);
    bool addMPRWidget(QmitkStdMultiWidget *widget);
    void computeBounds(bool resetCamera=true);
    void computeBounds(MitkWidgetBase::DataType type, bool resetCamera = true);

    bool isActorExist(vtkProp3D* actor);
    bool isActorExist(const std::string& name);

    //actor的操作目前仅支持单窗口
    void addActor(vtkProp3D* actor, const std::string& name,bool cover=false);
    void removeActorByName(const std::string &name);
    void removeActor(vtkProp3D* actor);
    vtkProp3D* getActor(const std::string& name);

    bool isDataNodeExistByName(const std::string& name);


    void requestUpdateRender();

    std::vector<QmitkRenderWindow *> m_renderwindows;
    std::vector<QmitkStdMultiWidget*> m_mprwidgets;
    mitk::StandaloneDataStorage::Pointer m_dataStorage{nullptr};
    std::map<std::string, vtkProp3D*> m_actorMap;

private:
    
};

MitkWidgetBasePrivate::MitkWidgetBasePrivate()
{
}

void MitkWidgetBasePrivate::createDataStorage()
{
    m_dataStorage = mitk::StandaloneDataStorage::New();
}

bool MitkWidgetBasePrivate::addRenderWindow(QmitkRenderWindow *renderwindow){
    if (!renderwindow) return false;
    if (std::find(m_renderwindows.begin(), m_renderwindows.end(), renderwindow) == m_renderwindows.end())
    {
        m_renderwindows.push_back(renderwindow);
        return true;
    }
    return false;
}

bool MitkWidgetBasePrivate::addMPRWidget(QmitkStdMultiWidget* widget)
{
    if (!widget) return false;
    if (std::find(m_mprwidgets.begin(), m_mprwidgets.end(), widget) == m_mprwidgets.end()) {
        widget->InitializeMultiWidget();
        widget->AddPlanesToDataStorage();
        widget->SetCrosshairGap(0);
        //widget->ActivateMenuWidget(0);
        //widget->SetWidgetPlanesVisibility(false, widget->GetRenderWindow4()->GetRenderer());
        /*widget->GetWidgetPlane1()->SetProperty("draw edges", mitk::BoolProperty::New(false));
        widget->GetWidgetPlane2()->SetProperty("draw edges", mitk::BoolProperty::New(false));
        widget->GetWidgetPlane3()->SetProperty("draw edges", mitk::BoolProperty::New(false));*/
        m_mprwidgets.push_back(widget);
        return true;
    }
    return false;
}

void MitkWidgetBasePrivate::computeBounds(bool resetCamera)
{
    for (int i = 0; i < m_renderwindows.size(); ++i) {
        mitk::RenderingManager::GetInstance()->InitializeViewByBoundingObjects(m_renderwindows[i]->GetVtkRenderWindow(),
                                                                               m_dataStorage,resetCamera);
    }
    for (int i = 0; i < m_mprwidgets.size();++i)
    {
        m_mprwidgets[i]->ResetView();        
    } 
}

void MitkWidgetBasePrivate::computeBounds(MitkWidgetBase::DataType type, bool resetCamera)
{
    mitk::NodePredicateBase::Pointer predicate;
    if (type==MitkWidgetBase::Mesh)
        predicate= mitk::NodePredicateDataType::New(mitk::Surface::GetStaticNameOfClass());
    else if (type==MitkWidgetBase::Image)
        predicate = mitk::NodePredicateDataType::New(mitk::Image::GetStaticNameOfClass());
    mitk::DataStorage::SetOfObjects::ConstPointer filteredDataNodes = m_dataStorage->GetSubset(predicate);
    for (int i = 0; i < m_renderwindows.size(); ++i) {
        mitk::BaseRenderer* renderer = m_renderwindows[i]->GetRenderer();
        if (renderer->GetMapperID() == mitk::BaseRenderer::Standard2D)
            continue;
        auto bounds = m_dataStorage->ComputeBoundingGeometry3D(filteredDataNodes, "visible", renderer);
        mitk::RenderingManager::GetInstance()->InitializeView(renderer->GetRenderWindow(), bounds,resetCamera);
    }
    for (int i = 0; i < m_mprwidgets.size(); ++i) {
        mitk::BaseRenderer* renderer = m_mprwidgets[i]->GetRenderWindow4()->GetRenderer();
        auto bounds = m_dataStorage->ComputeBoundingGeometry3D(filteredDataNodes, "visible", renderer);
        //m_mprwidgets[i]->ResetView(bounds,true);
        for (int i = 0; i < m_mprwidgets[i]->GetNumberOfRenderWindowWidgets();++i)
            mitk::RenderingManager::GetInstance()->InitializeView(m_mprwidgets[i]->GetRenderWindow(i)->GetVtkRenderWindow(),bounds,resetCamera);
    }
}

bool MitkWidgetBasePrivate::isActorExist(vtkProp3D* actor)
{
    if (!actor) return false;
    for (auto pair:m_actorMap)
    {
        if (pair.second == actor) return true;
    }
    return false;
}

bool MitkWidgetBasePrivate::isActorExist(const std::string& name)
{
    for (auto pair : m_actorMap) {
        if (pair.first == name) {
            vtkProp* actor = pair.second;
            if (!actor) return false;
            return true;
        }
    }
    return false;
}

vtkProp3D* MitkWidgetBasePrivate::getActor(const std::string& name)
{
    for (auto pair : m_actorMap) {
        if (pair.first == name) {
            vtkProp3D* actor = pair.second;
            return actor;
        }
    }
    return nullptr;

}

void MitkWidgetBasePrivate::addActor(vtkProp3D* actor, const std::string& name, bool cover)
{
    if (isActorExist(actor) && !cover) {
        DataNodeManager::Error("actor is existed");
        return;
    }
    if (cover && isActorExist(name)) {
        removeActorByName(name);
    }
    m_actorMap.emplace(name, actor);
    for (int i = 0; i < m_renderwindows.size(); ++i) {
        QmitkRenderWindow* rw = m_renderwindows[i];
        rw->GetRenderer()->GetVtkRenderer()->AddActor(actor);
    }
    for (int i = 0; i < m_mprwidgets.size();++i)
    {
        m_mprwidgets[i]->GetRenderWindow4()->GetRenderer()->GetVtkRenderer()->AddActor(actor);
    } 
}

void MitkWidgetBasePrivate::removeActorByName(const std::string& name)
{
    if (!isActorExist(name)) {
        DataNodeManager::Error("actor is not existed");
        return;
    }
    for (auto it = m_actorMap.begin(); it != m_actorMap.end();) {
        if (it->first == name) {
            m_actorMap.erase(it++);
            for (int i = 0; i < m_renderwindows.size(); ++i) {
                m_renderwindows[i]->GetRenderer()->GetVtkRenderer()->RemoveActor(it->second);
            }
            break;
        }   
        else {
            ++it;
        }   
    }
}

void MitkWidgetBasePrivate::removeActor(vtkProp3D* actor)
{
    if (!isActorExist(actor)) {
        DataNodeManager::Error("actor is not existed");
        return;
    }
    for (auto it = m_actorMap.begin(); it != m_actorMap.end();)
    {
        if (it->second == actor) {
            m_actorMap.erase(it++);
        }
        else
        { 
            ++it;
        }
    }
    for (int i = 0; i < m_renderwindows.size(); ++i) {
        m_renderwindows[i]->GetRenderer()->GetVtkRenderer()->RemoveActor(actor);
    }
}

bool MitkWidgetBasePrivate::isDataNodeExistByName(const std::string& name)
{
    return m_dataStorage->GetNamedNode(name);
}

void MitkWidgetBasePrivate::requestUpdateRender()
{
    for (int i = 0; i < m_renderwindows.size(); ++i) {
        mitk::RenderingManager::GetInstance()->RequestUpdate(m_renderwindows[i]->GetVtkRenderWindow());
    }
    for (int i = 0; i < m_mprwidgets.size(); ++i) {
        m_mprwidgets[i]->RequestUpdateAll();
    }
}

MitkWidgetBase::MitkWidgetBase() : m_d(new MitkWidgetBasePrivate)
{

}

MitkWidgetBase::~MitkWidgetBase()
{
    if (m_d)
    {
        delete m_d;
        m_d = nullptr;
    } 
}

void MitkWidgetBase::configRenderWindow(QmitkRenderWindow *renderwindow)
{
    if (!m_d->m_dataStorage) m_d->createDataStorage();
    if (!renderwindow) return;
    bool ret = m_d->addRenderWindow(renderwindow);
    if (ret) renderwindow->GetRenderer()->SetDataStorage(m_d->m_dataStorage);
}

void MitkWidgetBase::configMPRWidget(QmitkStdMultiWidget* widget)
{
    if (!m_d->m_dataStorage) m_d->createDataStorage();
    if (!widget) return;
    widget->SetDataStorage(m_d->m_dataStorage);
    m_d->addMPRWidget(widget);
}

void MitkWidgetBase::setWidgetPlaneVisibility(int index, bool v)
{
    if (m_d->m_mprwidgets.size() == 0) return;
    for (int i = 0; i < m_d->m_mprwidgets.size();++i)
    {
        m_d->m_mprwidgets[i]->SetWidgetPlanesVisibility(v, m_d->m_mprwidgets[i]->GetRenderWindow(i)->GetRenderer());      
    }
    requestUpdateRender();
}

void MitkWidgetBase::setSurfaceNodeDistance(mitk::DataNode::Pointer node, double distance)
{
    if (!node || !isDataNodeExist(node))
        return;
    for (int i = 0; i < m_d->m_renderwindows.size(); ++i) {
        QmitkRenderWindow* rw = m_d->m_renderwindows[i];
        if (rw->GetRenderer()->GetMapperID()!=mitk::BaseRenderer::Standard3D) continue;
        auto mapper = dynamic_cast<mitk::VtkMapper*>(node->GetMapper(mitk::BaseRenderer::Standard3D));
        if (!mapper) continue;
        vtkActor* actor = dynamic_cast<vtkActor*>(mapper->GetVtkProp(rw->GetRenderer()));
        if (actor) {
            auto v_mapper = dynamic_cast<vtkMapper*>(actor->GetMapper());
            if (!v_mapper) continue;
            v_mapper->SetResolveCoincidentTopologyToPolygonOffset();
            v_mapper->SetRelativeCoincidentTopologyLineOffsetParameters(0, distance);
            v_mapper->SetRelativeCoincidentTopologyPolygonOffsetParameters(0, distance);
            v_mapper->SetRelativeCoincidentTopologyPointOffsetParameter(distance);
        }
    }
    for (int i = 0; i < m_d->m_mprwidgets.size(); ++i) {
        auto mapper = dynamic_cast<mitk::VtkMapper*>(node->GetMapper(mitk::BaseRenderer::Standard3D));
        if (!mapper)
            continue;
        vtkActor* actor =
            dynamic_cast<vtkActor*>(mapper->GetVtkProp(m_d->m_mprwidgets[i]->GetRenderWindow4()->GetRenderer()));
        if (actor)
        {
            auto v_mapper = dynamic_cast<vtkMapper*>(actor->GetMapper());
            if (!v_mapper)
                continue;
            v_mapper->SetResolveCoincidentTopologyToPolygonOffset();
            v_mapper->SetRelativeCoincidentTopologyLineOffsetParameters(0, distance);
            v_mapper->SetRelativeCoincidentTopologyPolygonOffsetParameters(0, distance);
            v_mapper->SetRelativeCoincidentTopologyPointOffsetParameter(distance);
        }
        else
        {
            vtkAssembly* assembly =
                dynamic_cast<vtkAssembly*>(mapper->GetVtkProp(m_d->m_mprwidgets[i]->GetRenderWindow4()->GetRenderer()));
            if (!assembly)
                continue;
            vtkSmartPointer<vtkPropCollection> collection = vtkSmartPointer<vtkPropCollection>::New();
            assembly->GetActors(collection);
            collection->InitTraversal();
            for (vtkIdType i = 0; i < collection->GetNumberOfItems(); i++) {
                vtkActor *actor=dynamic_cast<vtkActor*>(collection->GetNextProp());
                if (!actor) continue;
                auto v_mapper = dynamic_cast<vtkMapper*>(actor->GetMapper());
                if (!v_mapper) continue;
                v_mapper->SetResolveCoincidentTopologyToPolygonOffset();
                v_mapper->SetRelativeCoincidentTopologyLineOffsetParameters(0, distance);
                v_mapper->SetRelativeCoincidentTopologyPolygonOffsetParameters(0, distance);
                v_mapper->SetRelativeCoincidentTopologyPointOffsetParameter(distance);
            }
        }
    }
}

void MitkWidgetBase::computeBounds(bool resetCamera)
{
    return m_d->computeBounds(resetCamera);
}

void MitkWidgetBase::computeBounds(DataType type, bool resetCamera)
{
    return m_d->computeBounds(type, true);
}

void MitkWidgetBase::requestUpdateRender()
{
    m_d->requestUpdateRender();
}

void MitkWidgetBase::init()
{
    m_d->createDataStorage();
}

void MitkWidgetBase::addDataNode(mitk::DataNode::Pointer pNode, bool computeBounds)
{
    if (!pNode) {
        Error("node is empty");
        return;
    }
    if (isDataNodeExist(pNode))
    {
        Error("node is exist");
        return;
    }
    m_d->m_dataStorage->Add(pNode);
    if (computeBounds)
    {
        m_d->computeBounds();
    } 
}

void MitkWidgetBase::removeDataNode(mitk::DataNode::Pointer pNode, bool computeBounds)
{
    if (!isDataNodeExist(pNode)) {
        Error("node is not exist");
        return;
    }
    m_d->m_dataStorage->Remove(pNode);
    if (computeBounds) {
        m_d->computeBounds();
    }
}

void MitkWidgetBase::removeDataNodeByName(const std::string& name, bool computeBounds)
{
    if (!isDataNodeExistByName(name)) {
        Error("node is not exist");
        return;
    }
    m_d->m_dataStorage->Remove(getDataNodeByName(name));
    if (computeBounds) {
        m_d->computeBounds();
    }
}

bool MitkWidgetBase::isDataNodeExist(mitk::DataNode::Pointer pNode)
{
    return m_d->m_dataStorage->Exists(pNode);
}

bool MitkWidgetBase::isDataNodeExistByName(const std::string& name)
{
    return m_d->isDataNodeExistByName(name);
}

mitk::DataNode::Pointer MitkWidgetBase::getDataNodeByName(const std::string& pName)
{
    return m_d->m_dataStorage->GetNamedNode(pName);
}

void MitkWidgetBase::addActor(vtkProp3D* actor, const std::string& name, bool cover)
{
    m_d->addActor(actor, name);
}

void MitkWidgetBase::removeActor(vtkProp3D* actor)
{
    m_d->removeActor(actor);
}

void MitkWidgetBase::removeActorByName(const std::string& name)
{
    m_d->removeActorByName(name);
}

bool MitkWidgetBase::isActorExist(vtkProp3D* actor)
{
    return m_d->isActorExist(actor);
}

bool MitkWidgetBase::isActorExist(const std::string& name)
{
    return m_d->isActorExist(name);
}

vtkProp3D* MitkWidgetBase::getActor(const std::string& name)
{
    return m_d->getActor(name);
}

void MitkWidgetBase::transformDataNodeByName(const std::string& name, vtkMatrix4x4* const vmt)
{
    if (!isDataNodeExistByName(name))
    {
        return;
    }
    transformDataNode(getDataNodeByName(name),vmt);
}

void MitkWidgetBase::transformDataNodeByName(const std::string &name, Eigen::Matrix4d mt)
{
    if (!isDataNodeExistByName(name)) {
        std::cout << name << "is not exist" << std::endl;
        return;
    }
    transformDataNode(getDataNodeByName(name), mt);
}

void MitkWidgetBase::transformActorByName(std::string& name, Eigen::Matrix4d& mt)
{
    // 构建vtk变换矩阵,获取渲染对象当前变换矩阵
    vtkSmartPointer<vtkTransform> vtkTrans = vtkSmartPointer<vtkTransform>::New();
    vtkTrans->PostMultiply();

    // 计算传入的Eigen矩阵
    double element[16] = {mt(0, 0), mt(0, 1), mt(0, 2), mt(0, 3), mt(1, 0), mt(1, 1), mt(1, 2), mt(1, 3),
                          mt(2, 0), mt(2, 1), mt(2, 2), mt(2, 3), mt(3, 0), mt(3, 1), mt(3, 2), mt(3, 3)};
    vtkTrans->Concatenate(element);

    auto actor3d = this->getActor(name);
    if (!actor3d) {
        std::cout << name << " data is not exist!" << std::endl;
    }

    actor3d->SetUserTransform(vtkTrans);
    requestUpdateRender();
}

void MitkWidgetBase::transformActorByName(std::string& name, vtkMatrix4x4* vmt)
{
    Eigen::Matrix4d mt;
    VtkMatrix4x4ToEigen(mt, vmt);
    transformActorByName(name, mt);
}

void MitkWidgetBase::getDataNodeTransformByName(const std::string& name, Eigen::Matrix4d& mt)
{
    if (!isDataNodeExistByName(name)) {
        return;
    }
    getDataNodeTransform(getDataNodeByName(name), mt);
}

void MitkWidgetBase::getDataNodeTransformByName(const std::string& name, vtkMatrix4x4* const vmt)
{
    if (!isDataNodeExistByName(name)) {
        return;
    }
    getDataNodeTransform(getDataNodeByName(name), vmt);
}

void MitkWidgetBase::setDataNodeVisibilityByName(const std::string& name, bool v)
{
    if (!isDataNodeExistByName(name)) {
        return;
    }
    setDataNodeVisibility(getDataNodeByName(name), v);
}

bool MitkWidgetBase::getDataNodeVisibilityByName(const std::string& name)
{
    if (!isDataNodeExistByName(name)) {
        return 0;
    }
    return getDataNodeVisibility(getDataNodeByName(name));
}

void MitkWidgetBase::setDataNodeOpacityByName(const std::string& name, float opacity)
{
    if (!isDataNodeExistByName(name)) {
        return;
    }
    setDataNodeOpacity(getDataNodeByName(name), opacity);
}

float MitkWidgetBase::getDataNodeOpacityByName(const std::string& name)
{
    if (!isDataNodeExistByName(name)) {
        return 0.0;
    }
    return getDataNodeOpacity(getDataNodeByName(name));
}

void MitkWidgetBase::setDataNodeColorByName(const std::string &name, const float* const color)
{
    if (!isDataNodeExistByName(name)) {
        return;
    }
    setDataNodeColor(getDataNodeByName(name), color);
}

void MitkWidgetBase::setDataNodeColorByName(const std::string& name, float r, float g, float b)
{
    if (!isDataNodeExistByName(name)) {
        return;
    }
    setDataNodeColor(getDataNodeByName(name), r,g,b);
}

float* MitkWidgetBase::getDataNodeColorByName(const std::string& name)
{
    if (!isDataNodeExistByName(name)) {
        return nullptr;
    }
    return getDataNodeColor(getDataNodeByName(name));
}

void MitkWidgetBase::getDataNodeColorByName(const std::string& name, float* color)
{
    if (!isDataNodeExistByName(name)) {
        return;
    }
    return getDataNodeColor(getDataNodeByName(name),color);
}


mitk::DataStorage::Pointer MitkWidgetBase::getDataStorage() const
{
    return m_d->m_dataStorage;
}
