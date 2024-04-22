#pragma once
#include <QWidget>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
namespace Ui {
class Widget;
}
class WidgetPrivate;
class Widget : public QWidget {
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    void makeLayout(int row, int col);
    void setInputImage(vtkImageData *imagedata);
    void renderImage();

protected:
    QScopedPointer<WidgetPrivate> d_ptr;

private:
    Ui::Widget* ui;
    Q_DISABLE_COPY(Widget)
    Q_DECLARE_PRIVATE(Widget)
    Q_D(Widget);
};



