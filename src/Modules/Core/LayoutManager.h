
#pragma once

#include <QmitkRenderWindow.h>
#include "CoreExports.h"
class QmitkStdMultiWidget;

class Core_EXPORT LayoutManager {
public:
    enum WidgetType { WidgetType_3D, WidgetType_Axial, WidgetType_Sagittal, WidgetType_Coronal };

    // 创建MPR窗口
    static QmitkStdMultiWidget *createMPRWidget(QWidget *parent = nullptr);

    // 创建一个axial/sagittal/coronal/3d窗口
    static QmitkRenderWindow *createSingleWidget(QWidget *parent = nullptr);

    // 设置窗口类型
    static void setWidgetType(QmitkRenderWindow *widget, WidgetType type);
};