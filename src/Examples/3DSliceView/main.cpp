
#include "vtkAlgorithm.h"
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
#include <vtkMatrix3x3.h>
#include <vtkMatrix4x4.h>
#include <QDir>
#include <vtkSTLReader.h>
#include <vtkPolyDataMapper.h>
#include <ctkVTKVolumePropertyWidget.h>
#include <QApplication>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkCamera.h>
#include <vtkCutter.h>
#include <vtkPlane.h>
#include <vtkProperty.h>
#include <vtkCallbackCommand.h>
#include <vtkGPUVolumeRayCastMapper.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);
void PropertyCallback(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData)
{
    // std::cout << "PropertyCallback"<<std::endl;
    QVTKOpenGLNativeWidget* wp = (QVTKOpenGLNativeWidget*)(clientData);
    wp->GetRenderWindow()->Render();
}
class CamerCallback : public vtkCommand {
public:
    vtkRenderWindow *m_widget = nullptr;

    static CamerCallback *New() { return new CamerCallback; }

    virtual void Execute(vtkObject *caller, unsigned long, void *)
    {
        static bool createOutline{false};
        if (!createOutline) {
            createOutline = true;
            outlineactor = vtkSmartPointer<vtkActor>::New();
            vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
            outlineactor->SetMapper(mapper);
            outlineactor->GetProperty()->SetColor(1,0,0);
            renderer->AddActor(outlineactor);
        }
        // set one camera;
        vtkCamera *camera = static_cast<vtkCamera *>(caller);
        double cr[2];
        double cameracentre[3];
        double camerafocalpoint[3];
        camera->GetPosition(cameracentre);
        camera->GetFocalPoint(camerafocalpoint);

        double dist = sqrt(vtkMath::Distance2BetweenPoints(cameracentre, camerafocalpoint));
        camera->GetClippingRange(cr);

        camera->SetClippingRange(dist, cr[1]);

        vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();
        plane->SetOrigin(camerafocalpoint);
        Eigen::Vector3d v1(cameracentre);
        Eigen::Vector3d v2(camerafocalpoint);
        plane->SetNormal(Eigen::Vector3d(v1-v2).normalized().data());
        vtkSmartPointer<vtkCutter> cutter = vtkSmartPointer<vtkCutter>::New();
        cutter->SetInputData(source);
        cutter->SetCutFunction(plane);
        cutter->Update();
        vtkPolyDataMapper::SafeDownCast(outlineactor->GetMapper())->SetInputData(cutter->GetOutput());
    }
    vtkSmartPointer<vtkActor> outlineactor;
    vtkPolyData *source;
    vtkRenderer *renderer;
};
int main(int argc, char* argv[])
{
    /* vtkNew<vtkDICOMImageReader> vtkreader;
     vtkreader->SetFileName(path.c_str());
     vtkreader->Update();*/
    QApplication ac(argc, argv);
    QVTKOpenGLNativeWidget w;
    std::string path = "D:\\Images\\ct";
    typedef signed short shortPixelType;
    const unsigned int Dim = 3;  //数据的Dimension

    typedef itk::Image<shortPixelType, Dim> ShortImageType;
    typedef itk::ImageSeriesReader<ShortImageType> ReaderType;

    itk::GDCMImageIO::Pointer gdcmIO = itk::GDCMImageIO::New();
    itk::GDCMSeriesFileNames::Pointer seriesFileNames = itk::GDCMSeriesFileNames::New();

    seriesFileNames->SetDirectory(path);
    const itk::GDCMSeriesFileNames::SeriesUIDContainerType& seriesUIDs = seriesFileNames->GetSeriesUIDs();
    const ReaderType::FileNamesContainer& filenames = seriesFileNames->GetFileNames(seriesUIDs[0]);

    typename ReaderType::Pointer reader = ReaderType::New();
    try {
        reader->SetImageIO(gdcmIO);
        reader->SetFileNames(filenames);
        reader->Update();
    } catch (itk::ExceptionObject& ex) {
        //读取过程发生错误
        std::cerr << "Error: " << ex << std::endl;
        return false;
    }
    itk::ImageToVTKImageFilter<ShortImageType>::Pointer cast = itk::ImageToVTKImageFilter<ShortImageType>::New();
    cast->SetInput(reader->GetOutput());
    cast->Update();


    vtkNew<vtkSTLReader> stlreader;
    stlreader->SetFileName("D:\\__AA_raasystem__\\build\\bin\\x64\\Release\\prosthesisdata\\MeshSTL\\THA_Pelvis.stl");
    stlreader->Update();
    vtkNew<vtkActor> stlactor;
    vtkNew<vtkPolyDataMapper> mapper;
    stlactor->SetMapper(mapper);
    stlactor->GetProperty()->SetAmbient(0);
    mapper->SetInputData(stlreader->GetOutput());
    vtkSmartPointer<vtkImageData> rstImg = vtkSmartPointer<vtkImageData>::New();
    
    cropImageByPolyData(cast->GetOutput(), rstImg, stlreader->GetOutput());
    vtkSmartPointer<vtkSmartVolumeMapper> volumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
    volumeMapper->SetInputData(rstImg);
    volumeMapper->SetSampleDistance(0);
    volumeMapper->SetAutoAdjustSampleDistances(false);
    volumeMapper->InteractiveAdjustSampleDistancesOff();
    //volumeMapper->setInputData(cast->GetOutput());
    //设置光线采样距离
     //volumeMapper->SetSampleDistance(volumeMapper->GetSampleDistance()*4);
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
    compositeOpacity->AddPoint(151.35, 0.00);
    compositeOpacity->AddPoint(158.28, 0.44);
    compositeOpacity->AddPoint(190.11, 0.58);
    compositeOpacity->AddPoint(200.87, 0.73);
    compositeOpacity->AddPoint(3652.86, 0.74);

    volumeProperty->SetScalarOpacity(compositeOpacity);  //设置不透明度传输函数
    // compositeOpacity->AddPoint(120,  0.00);//测试隐藏部分数据,对比不同的设置
    // compositeOpacity->AddPoint(180,  0.60);
     //volumeProperty->SetScalarOpacity(compositeOpacity);
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
    //volumeProperty->SetInterpolationTypeToLinear();
    /********************************************************************************/
    vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();
    volume->SetMapper(volumeMapper);
    volume->SetProperty(volumeProperty);
    ctkVTKVolumePropertyWidget propertyWidget;
    propertyWidget.setVolumeProperty(volumeProperty);
    propertyWidget.show();
    vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();
    ren->SetBackground(0, 1, 0);
    ren->AddVolume(volume);
    ren->AddActor(stlactor);
    ren->UseDepthPeelingOn();
    ren->UseDepthPeelingForVolumesOn();
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> rw = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    rw->AddRenderer(ren);
    rw->SetSize(640, 480);
    rw->SetAlphaBitPlanes(1);
    rw->SetMultiSamples(0);
    /*rw->SetMultiSamples(1);
    rw->SetPointSmoothing(0);
    rw->SetLineSmoothing(0);
    rw->SetPolygonSmoothing(0);*/
    //rw->Render();
    rw->SetWindowName("VolumeRendering PipeLine");
    w.setRenderWindow(rw);
    //vtkSmartPointer<vtkRenderWindowInteractor> rwi = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    //rwi->SetRenderWindow(rw);

    ren->ResetCamera();
    vtkSmartPointer<CamerCallback> cb = vtkSmartPointer<CamerCallback>::New();
    cb->renderer = ren;
    cb->source = stlreader->GetOutput();
    ren->GetActiveCamera()->AddObserver(vtkCommand::ModifiedEvent, cb);
    vtkSmartPointer<vtkCallbackCommand> propertycallback = vtkSmartPointer<vtkCallbackCommand>::New();

    propertycallback->SetCallback(PropertyCallback);
    propertycallback->SetClientData(&w);

    volumeProperty->AddObserver(vtkCommand::AnyEvent, propertycallback);

    volumeProperty->GetRGBTransferFunction()->AddObserver(vtkCommand::AnyEvent, propertycallback);
    volumeProperty->GetGradientOpacity(0)->AddObserver(vtkCommand::AnyEvent, propertycallback);
    //rw->Render();
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
    m_markwidget->SetInteractor(w.interactor());
    m_markwidget->SetViewport(0.9, 0.05, 1.0, 0.15);
    m_markwidget->SetEnabled(1);
    m_markwidget->InteractiveOn();
    //rwi->Start();
    w.show();

    return ac.exec();
}