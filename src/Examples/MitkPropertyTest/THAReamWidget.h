#pragma once 
#include <QWidget>
#include "MitkWidgetBase.h"
#include <vtkImageData.h>
#include <vtkPolyData.h>
class THAReamWidget : public QWidget,public MitkWidgetBase
{
    Q_OBJECT
public:
    THAReamWidget(QWidget *parent=nullptr);
    ~THAReamWidget();

    //源图像的Mesh,对应骨盆、骨盆姿态
    void setSourcePolyData(vtkPolyData *src,vtkMatrix4x4 *vmt=nullptr);
    //臼杯半径、臼杯中心
    void setCupRadius(unsigned int radius,double *cupCenter);
    //打磨工具半径、数据、初始位置
    void setTool(unsigned int toolRadius,vtkPolyData *src,vtkMatrix4x4 *vmt=nullptr);
    //打磨工具姿态
    void setToolPosture(vtkMatrix4x4 *vmt);
    //渲染生成结果
    void renderResult();

    //开始刷新结果
    void runReam();

protected:
    vtkSmartPointer<vtkImageData> polyDataToImageData(vtkPolyData *polydata);
    vtkSmartPointer<vtkImageData> generateImageData();

private:
    class Impl;
    Impl *m_impl{nullptr};
};