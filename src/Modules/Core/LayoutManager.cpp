
#include "LayoutManager.h"
#include "./QmitkStdMultiWidget.h"


QmitkStdMultiWidget* LayoutManager::createMPRWidget(QWidget* parent)
{
    return new QmitkStdMultiWidget(parent);
}

QmitkRenderWindow* LayoutManager::createSingleWidget(QWidget* parent)
{
    return new QmitkRenderWindow(parent);
}

void LayoutManager::setWidgetType(QmitkRenderWindow* widget, WidgetType type)
{
    if (!widget) return;
    if (WidgetType_3D == type) {
        widget->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);
    }
    else {
        widget->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard2D);
        switch (type) {
            case WidgetType_Axial: {
                widget->GetSliceNavigationController()->SetDefaultViewDirection(mitk::AnatomicalPlane::Axial);
            } break;
            case WidgetType_Sagittal: {
                widget->GetSliceNavigationController()->SetDefaultViewDirection(mitk::AnatomicalPlane::Sagittal);
            } break;
            case WidgetType_Coronal: {
                widget->GetSliceNavigationController()->SetDefaultViewDirection(mitk::AnatomicalPlane::Coronal);
            } break;
        }
    }
}
