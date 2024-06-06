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

    //Դͼ���Mesh,��Ӧ���衢������̬
    void setSourcePolyData(vtkPolyData *src,vtkMatrix4x4 *vmt=nullptr);
    //�ʱ��뾶���ʱ�����
    void setCupRadius(unsigned int radius,double *cupCenter);
    //��ĥ���߰뾶�����ݡ���ʼλ��
    void setTool(unsigned int toolRadius,vtkPolyData *src,vtkMatrix4x4 *vmt=nullptr);
    //��ĥ������̬
    void setToolPosture(vtkMatrix4x4 *vmt);
    //��Ⱦ���ɽ��
    void renderResult();

    //��ʼˢ�½��
    void runReam();

protected:
    vtkSmartPointer<vtkImageData> polyDataToImageData(vtkPolyData *polydata);
    vtkSmartPointer<vtkImageData> generateImageData();

private:
    class Impl;
    Impl *m_impl{nullptr};
};