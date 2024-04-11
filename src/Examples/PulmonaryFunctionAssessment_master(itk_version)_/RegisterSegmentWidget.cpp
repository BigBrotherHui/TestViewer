#include "RegisterSegmentWidget.h"
#include "ct_image.h"
#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkBinaryErodeImageFilter.h"
#include "itkBinaryFillholeImageFilter.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkSliceBySliceImageFilter.h"
#include "itkLabelShapeKeepNObjectsImageFilter.h"
#include "itkLabelImageToShapeLabelMapFilter.h"
#include <itkImageFileWriter.h>
#include "itkExtractImageFilter.h"
#include <itkConnectedThresholdImageFilter.h>
#include <itkShiftScaleImageFilter.h>
#include <itkBinaryMorphologicalClosingImageFilter.h>
#include <itkNiftiImageIO.h>
#include <itkImageFileReader.h>
#include <itkImageToVTKImageFilter.h>
#include <QDebug>

vtkSmartPointer<vtkImageData> itkToVtk(itk::Image<signed short,3>::Pointer image)
{
    typedef itk::ImageToVTKImageFilter<itk::Image<signed short, 3>> itkTovtkFilterType;
    itkTovtkFilterType::Pointer itkTovtkImageFilter = itkTovtkFilterType::New();
    itkTovtkImageFilter->SetInput(image);
    try
    {
        itkTovtkImageFilter->Update();
    }
    catch (itk::ExceptionObject &e)
    {
        std::cout << "itk exception:" << e.what() << std::endl;
        return nullptr;
    }
    return itkTovtkImageFilter->GetOutput();
}

RegisterSegmentWidget::RegisterSegmentWidget(QWidget *parent)
    : QWidget(parent),ui(new Ui::RegisterSegmentWidget)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    ui->lineEdit_thresholdMax->setText(QString::number(max));
    ui->lineEdit_thresholdMin->setText(QString::number(min));
    on_pushButton_apply_clicked();
}

RegisterSegmentWidget::~RegisterSegmentWidget()
{

}

void RegisterSegmentWidget::setImageItk(itk::Image<CT_Image::PixelType, 3>::Pointer img)
{
    mImageItk = img;
    vtkSmartPointer<vtkImageData> t = itkToVtk(mImageItk);
    ui->widget_preview->showImage(t);
}

void RegisterSegmentWidget::lungSegment()
{
    if (!mImageItk)
        return;
    const unsigned int Dimension = 3;
    using PixelType = signed short;
    typedef itk::Image<unsigned char, Dimension> MaskImageType;
    typedef itk::Image<PixelType, 3> ImageType;
    itk::ImageFileReader<ImageType>::Pointer r = itk::ImageFileReader<ImageType>::New();
    r->SetFileName("D:/image/lung.nii");
    r->SetImageIO(itk::NiftiImageIO::New());
    r->Update();
    mImageItk = r->GetOutput();

    //using StructuringElementType = itk::BinaryBallStructuringElement<PixelType, Dimension>;
    //using ErodeFilterType = itk::BinaryErodeImageFilter<MaskImageType, MaskImageType, StructuringElementType>;
    //using DilateFilterType = itk::BinaryDilateImageFilter<MaskImageType, MaskImageType, StructuringElementType>;
    //typedef itk::ConnectedComponentImageFilter<MaskImageType, ImageType> ConnectedComponentImageFilterType;
    //typedef itk::DiscreteGaussianImageFilter<ImageType, ImageType> gaussianFilterType;
    //using BinaryThresholdFilterType = itk::BinaryThresholdImageFilter<ImageType, MaskImageType>;
    //BinaryThresholdFilterType::Pointer binaryThresholdfilter = BinaryThresholdFilterType::New();
    //binaryThresholdfilter->SetInput(mImageItk);
    //const unsigned char outsidevalue = 0;
    //const unsigned char  insidevalue = 1;
    //binaryThresholdfilter->SetOutsideValue(outsidevalue);
    //binaryThresholdfilter->SetInsideValue(insidevalue);
    //const unsigned char lowerThreshold = -300;
    //const unsigned char upperThreshold = -300;
    //binaryThresholdfilter->SetUpperThreshold(upperThreshold);
    //binaryThresholdfilter->SetLowerThreshold(lowerThreshold);
    //try
    //{
    //    binaryThresholdfilter->Update();// Running Filter;
    //}
    //catch (itk::ExceptionObject& e)
    //{
    //    cout << "Caught Error:" << e.what() << endl;
    //    return;
    //}

    //itk::ConnectedThresholdImageFilter<MaskImageType, MaskImageType>::Pointer connectedThresholdFilter = itk::ConnectedThresholdImageFilter<MaskImageType, MaskImageType>::New();
    //connectedThresholdFilter->SetInput(binaryThresholdfilter->GetOutput());
    //connectedThresholdFilter->SetLower(0);
    //connectedThresholdFilter->SetUpper(0);
    //connectedThresholdFilter->SetReplaceValue(1); // 设置替换的像素值为1，表示被连接的区域
    //auto size = mImageItk->GetLargestPossibleRegion().GetSize();
    //MaskImageType::IndexType seed0{ 0,0,0 }, seed1{ size[0] - 1,size[1] - 1,0 };
    //connectedThresholdFilter->AddSeed(seed0);
    //connectedThresholdFilter->AddSeed(seed1);
    //try
    //{
    //    connectedThresholdFilter->Update();
    //}
    //catch (itk::ExceptionObject& ex)
    //{
    //    std::cerr << "Error running ConnectedThresholdImageFilter: " << ex << std::endl;
    //    return;
    //}

    // 利用形态学来去掉一定的肺部的小区域
    //typedef itk::BinaryBallStructuringElement<PixelType, Dimension> BSEType;
    //using BinaryMorphologicalClosingImageFilterType = itk::BinaryMorphologicalClosingImageFilter<MaskImageType, MaskImageType, BSEType>;
    //BinaryMorphologicalClosingImageFilterType::Pointer bm = BinaryMorphologicalClosingImageFilterType::New();
    //BSEType ballStrEle;
    //ballStrEle.SetRadius(2);
    //ballStrEle.CreateStructuringElement();
    //bm->SetInput(temp);
    //bm->SetKernel(ballStrEle);
    //bm->Update();

    //itk::ImageFileWriter<MaskImageType>::Pointer ww = itk::ImageFileWriter<MaskImageType>::New();
    //ww->SetFileName("222222.nii");
    //ww->SetImageIO(itk::NiftiImageIO::New());
    //ww->SetInput(connectedThresholdFilter->GetOutput());
    //try
    //{
    //    ww->Update();
    //}
    //catch (itk::ExceptionObject& e)
    //{
    //    std::cout << "error:" << e.what() << std::endl;
    //}
}

vtkSmartPointer<vtkImageData> RegisterSegmentWidget::getSegmentResult()
{
    return ui->widget_preview->getForegroundImageData();
}

void RegisterSegmentWidget::on_pushButton_apply_clicked()
{
    int min = ui->lineEdit_thresholdMin->text().toInt();
    int max = ui->lineEdit_thresholdMax->text().toInt();
    ui->widget_preview->setConnectedThresholdMinMax(min,max);
}

void RegisterSegmentWidget::on_pushButton_fillhole_clicked()
{
    ui->widget_preview->FillHole();
}

void RegisterSegmentWidget::on_pushButton_largestRegion_clicked()
{
    ui->widget_preview->ExtractLargestRegion();
}

void RegisterSegmentWidget::on_pushButton_volumeRendering_clicked()
{
    emit signal_volumeRenderingForegroundImage();
}

void RegisterSegmentWidget::on_pushButton_brush_clicked()
{
    ui->widget_preview->addBrush();
}
