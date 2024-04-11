#pragma once

#include <QWidget>
#include "ui_registersegmentwidget.h"
#include <itkImage.h>
#include "ct_image.h"

class RegisterSegmentWidget : public QWidget
{
    Q_OBJECT

public:
    RegisterSegmentWidget(QWidget *parent = nullptr);
    ~RegisterSegmentWidget();
    void setImageItk(itk::Image<CT_Image::PixelType, 3>::Pointer img);
    void lungSegment();
    vtkSmartPointer<vtkImageData> getSegmentResult();
signals:
    void signal_volumeRenderingForegroundImage();
protected slots:
    void on_pushButton_apply_clicked();
    void on_pushButton_fillhole_clicked();
    void on_pushButton_largestRegion_clicked();
    void on_pushButton_volumeRendering_clicked();
    void on_pushButton_brush_clicked();
private:
    Ui::RegisterSegmentWidget* ui;
    itk::Image<CT_Image::PixelType, 3>::Pointer mImageItk{nullptr};
    int min{ 30 }, max{3000};
};



