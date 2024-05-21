#include "ResliceWidget.h"

ResliceWidget::ResliceWidget(QWidget *parent)
    : QVTKOpenGLNativeWidget(parent)
{
    m_renderwindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    setRenderWindow(m_renderwindow);
    m_renderer = vtkSmartPointer<vtkRenderer>::New();
    m_renderwindow->AddRenderer(m_renderer);
    m_actor = vtkSmartPointer<vtkImageActor>::New();
    m_renderer->AddActor(m_actor);
}

ResliceWidget::~ResliceWidget()
{
}

void ResliceWidget::setImageReslicer(const vtkSmartPointer<vtkImageResliceToColors>& reslicer)
{
    m_reslicer = reslicer;
}

void ResliceWidget::setResliceMatrix(vtkMatrix4x4* vmt)
{
    if (!m_reslicer)
       return;
    m_reslicer->SetResliceAxes(vmt);
    m_reslicer->Update();
    m_actor->SetInputData(m_reslicer->GetOutput());
    m_renderwindow->Render();
}

