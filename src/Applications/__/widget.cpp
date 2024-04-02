#include "widget.h"
#include "ui_widget.h"
#include <QDir>
#include <QDebug>
#include <iostream>
#include <QGridLayout>
#include <QVTKOpenGLNativeWidget.h>
class WidgetPrivate {
public:
    WidgetPrivate(Widget *w) : q_ptr(w) { 
        m_layout = new QGridLayout(w);
        m_image = vtkSmartPointer<vtkImageData>::New();
    }
private:
    Widget *const q_ptr;
    Q_DECLARE_PUBLIC(Widget)

    QGridLayout *m_layout;
    std::vector<std::shared_ptr<QVTKOpenGLNativeWidget>> m_widgets;
    vtkSmartPointer<vtkImageData> m_image;
};
Widget::Widget(QWidget *parent)
    : QWidget(parent),d_ptr(QScopedPointer<WidgetPrivate>(new WidgetPrivate(this)))

{
    makeLayout(2, 2);
}


Widget::~Widget()
{
}

void Widget::makeLayout(int row, int col)
{
    for (int i = 0; i < d->m_widgets.size(); ++i) {
        d->m_layout->removeWidget(d->m_widgets[i].get());
        d->m_widgets[i]->deleteLater();
    }
    d->m_widgets.clear();
    for (int i = 0; i < row;++i)
    {
        for (int j=0;j<col;++j)
        {
            auto ww = std::make_shared<QVTKOpenGLNativeWidget>(this);
            d->m_widgets.push_back(ww);
            d->m_layout->addWidget(ww.get(),i,j);
        }
    } 
}

void Widget::setInputImage(vtkImageData* imagedata)
{
    d->m_image->DeepCopy(imagedata);
}

void Widget::renderImage()
{

}
