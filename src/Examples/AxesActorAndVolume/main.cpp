

#include <vtkAxesActor.h>
#include <vtkColorTransferFunction.h>
#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPiecewiseFunction.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkStructuredPoints.h>
#include <vtkStructuredPointsReader.h>
#include <vtkVolumeProperty.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkDicomImageReader.h>
#include <vtkSmartPointer.h>
#include <vtkAutoInit.h>
#include <vtkAnnotatedCubeActor.h>
#include <vtkImageFlip.h>
#include <vtkDICOMReader.h>
#include <iostream>


#include <itkImageFileReader.h>
#include <itkImageSeriesReader.h>
#include <itkGDCMImageIO.h>
#include <itkGDCMSeriesFileNames.h>
#include <itkImageToVTKImageFilter.h>

#include <QDebug>
#include <vtkDICOMSorter.h>
#include <vtkStringArray.h>
#include <QDir>

VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);
int main(int argc, char* argv[])
{
    /* vtkNew<vtkDICOMImageReader> vtkreader;
     vtkreader->SetFileName(path.c_str());
     vtkreader->Update();*/
    std::string path = "D:\\Images\\THAIMAGE";
    vtkNew<vtkDICOMReader> newReader;
    vtkNew<vtkStringArray> sinleFramesImages;
    vtkNew<vtkDICOMSorter> sorter;
    int count = 0;
    QDir dir(QString::fromStdString(path));
    auto list = dir.entryInfoList(QStringList() << "*.dcm", QDir::Files);
    for (const auto& image : list) {
        const auto path = image.absoluteFilePath();
        if (!path.isEmpty()) {
            sinleFramesImages->InsertValue(count++, path.toStdString());
        }
    }   
    sorter->SetInputFileNames(sinleFramesImages);
    sorter->Update();
    qDebug() << "number of series:"<< sorter->GetNumberOfSeries();
    newReader->SetFileNames(sorter->GetFileNamesForSeries(0));
    newReader->SetDataByteOrderToLittleEndian();
    newReader->Update();
    vtkNew<vtkImageFlip> flipy;
    flipy->SetInputData(newReader->GetOutput());
    flipy->SetFilteredAxis(1);
    flipy->Update();
    vtkNew<vtkImageFlip> flipz;
    flipz->SetInputData(flipy->GetOutput());
    flipz->SetFilteredAxis(2);
    flipz->Update();
    //开始读取DICOM数据序列
    typedef signed short shortPixelType;   
    const unsigned int  Dim = 3;       //数据的Dimension
  
    typedef itk::Image<shortPixelType, Dim> ShortImageType;
    typedef itk::ImageSeriesReader<ShortImageType> ReaderType;

    itk::GDCMImageIO::Pointer gdcmIO = itk::GDCMImageIO::New();
    itk::GDCMSeriesFileNames::Pointer seriesFileNames= itk::GDCMSeriesFileNames::New();

    seriesFileNames->SetDirectory(path);
    const itk::GDCMSeriesFileNames::SeriesUIDContainerType& seriesUIDs = seriesFileNames->GetSeriesUIDs();
	const ReaderType::FileNamesContainer& filenames = seriesFileNames->GetFileNames(seriesUIDs[0]);

    typename ReaderType::Pointer reader = ReaderType::New();
    try
    {
	reader->SetImageIO(gdcmIO);
	reader->SetFileNames(filenames);
	reader->Update();
    }
    catch (itk::ExceptionObject& ex)
    {
       //读取过程发生错误
        std::cerr << "Error: " << ex << std::endl;
        return false;
    }
    itk::ImageToVTKImageFilter<ShortImageType>::Pointer cast = itk::ImageToVTKImageFilter<ShortImageType>::New();
    cast->SetInput(reader->GetOutput());
    cast->Update();
    vtkSmartPointer<vtkSmartVolumeMapper> volumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
    flipz->GetOutput()->Print(std::cout);
    qDebug() << "==========================";
    cast->GetOutput()->Print(std::cout);
    volumeMapper->SetInputData(flipz->GetOutput());

    //设置光线采样距离
    // volumeMapper->SetSampleDistance(volumeMapper->GetSampleDistance()*4);
    //设置图像采样步长
    // volumeMapper->SetAutoAdjustSampleDistances(0);
    // volumeMapper->SetImageSampleDistance(4);
    /*************************************************************************/
    vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
    volumeProperty->SetInterpolationTypeToLinear();
    volumeProperty->ShadeOn();  //打开或者关闭阴影测试
    volumeProperty->SetAmbient(0.4);
    volumeProperty->SetDiffuse(0.6);   //漫反射
    volumeProperty->SetSpecular(0.2);  //镜面反射
    //设置不透明度
    vtkSmartPointer<vtkPiecewiseFunction> compositeOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
    compositeOpacity->AddPoint(70, 0.00);
    compositeOpacity->AddPoint(90, 0.40);
    compositeOpacity->AddPoint(180, 0.60);
    volumeProperty->SetScalarOpacity(compositeOpacity);  //设置不透明度传输函数
    // compositeOpacity->AddPoint(120,  0.00);//测试隐藏部分数据,对比不同的设置
    // compositeOpacity->AddPoint(180,  0.60);
    // volumeProperty->SetScalarOpacity(compositeOpacity);
    //设置梯度不透明属性
    vtkSmartPointer<vtkPiecewiseFunction> volumeGradientOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
    volumeGradientOpacity->AddPoint(10, 0.0);
    volumeGradientOpacity->AddPoint(90, 0.5);
    volumeGradientOpacity->AddPoint(100, 1.0);
    volumeProperty->SetGradientOpacity(volumeGradientOpacity);  //设置梯度不透明度效果对比
    //设置颜色属性
    vtkSmartPointer<vtkColorTransferFunction> color = vtkSmartPointer<vtkColorTransferFunction>::New();
    color->AddRGBPoint(0.000, 0.00, 0.00, 0.00);
    color->AddRGBPoint(64.00, 1.00, 0.52, 0.30);
    color->AddRGBPoint(190.0, 1.00, 1.00, 1.00);
    color->AddRGBPoint(220.0, 0.20, 0.20, 0.20);
    volumeProperty->SetColor(color);
    /********************************************************************************/
    vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();
    volume->SetMapper(volumeMapper);
    volume->SetProperty(volumeProperty);

    vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();
    ren->SetBackground(0, 1, 0);
    ren->AddVolume(volume);

    vtkSmartPointer<vtkRenderWindow> rw = vtkSmartPointer<vtkRenderWindow>::New();
    rw->AddRenderer(ren);
    rw->SetSize(640, 480);
    /*rw->SetMultiSamples(1);
    rw->SetPointSmoothing(0);
    rw->SetLineSmoothing(0);
    rw->SetPolygonSmoothing(0);*/
    rw->Render();
    rw->SetWindowName("VolumeRendering PipeLine");

    vtkSmartPointer<vtkRenderWindowInteractor> rwi = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    rwi->SetRenderWindow(rw);

    ren->ResetCamera();
    rw->Render();
     vtkNew<vtkAnnotatedCubeActor> m_axes;                          // 坐标方向箭头
     vtkNew<vtkOrientationMarkerWidget> m_markwidget;               // 坐标方向指示器
    m_axes->SetXPlusFaceText("L");
    m_axes->SetXMinusFaceText("R");
    m_axes->SetYMinusFaceText("A");
    m_axes->SetYPlusFaceText("P");
    m_axes->SetZMinusFaceText("I");
    m_axes->SetZPlusFaceText("S");
    m_markwidget->SetOutlineColor(1, 1, 1);
    m_markwidget->SetOrientationMarker(m_axes);
    m_markwidget->SetInteractor(rwi);
    m_markwidget->SetViewport(0.9, 0.05, 1.0, 0.15);
    m_markwidget->SetEnabled(1);
    m_markwidget->InteractiveOn();
    rwi->Start();

    return 0;
}