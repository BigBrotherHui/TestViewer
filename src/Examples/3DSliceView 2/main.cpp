
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
#include <QApplication>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkCamera.h>
#include <vtkCutter.h>
#include <vtkPlane.h>
#include <vtkProperty.h>
#include <vtkCallbackCommand.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkPointData.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>
#include <vtkPlaneCollection.h>
#include <vtkPlane.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);

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
    
    vtkSmartPointer<vtkImageData> rstImg = vtkSmartPointer<vtkImageData>::New();
    
    cropImageByPolyData(cast->GetOutput(), rstImg, stlreader->GetOutput(),false);
    //rstImg->Print(std::cout);
    int extent[6];
    double spacing[3];
    double origin[3];
    double center[3];

    rstImg->GetExtent(extent);
    rstImg->GetSpacing(spacing);
    rstImg->GetOrigin(origin);

    center[0] = origin[0] + spacing[0] * 0.5 * (extent[0] + extent[1]);
    center[1] = origin[1] + spacing[1] * 0.5 * (extent[2] + extent[3]);
    center[2] = origin[2] + spacing[2] * 0.5 * (extent[4] + extent[5]);
    double cornalElements[16] = {1, 0, 0, 0, 0, 0, 1, 0, 0, -1, 0, 0, 0, 0, 0, 1};
    auto resliceAxes = vtkSmartPointer<vtkMatrix4x4>::New();
    resliceAxes->DeepCopy(cornalElements);
    resliceAxes->SetElement(0, 3, center[0]);
    resliceAxes->SetElement(1, 3, center[1]);
    resliceAxes->SetElement(2, 3, center[2]);
    auto ImageReslice = vtkSmartPointer<vtkImageReslice>::New();
    ImageReslice->SetInputData(rstImg);
    ImageReslice->SetOutputDimensionality(2);
    ImageReslice->SetResliceAxes(resliceAxes);
    ImageReslice->SetInterpolationModeToLinear();
    ImageReslice->Update();

    vtkNew<vtkImageMapToWindowLevelColors> lookupTable;
    int level = 743;
    int window = 1692;
    lookupTable->SetLevel(level);
    lookupTable->SetWindow(window);
    lookupTable->SetInputData(ImageReslice->GetOutput());
    lookupTable->Update();
    //lookupTable->GetOutput()->Print(std::cout);
    auto scalars = lookupTable->GetOutput()->GetPointData()->GetArray("ImageScalars");  // GetArray("DICOMImage")
    auto img = lookupTable->GetOutput();
    int dims[3];
    img->GetDimensions(dims);

    for (int k = 0; k < dims[2]; ++k) {
        for (int j = 0; j < dims[1]; ++j) {
            for (int i = 0; i < dims[0]; ++i) {
                vtkIdType index = k * dims[0] * dims[1] + j * dims[0] + i;
                double *tuple = scalars->GetTuple4(index);
                ushort v = *((ushort *)ImageReslice->GetOutput()->GetScalarPointer(i, j, k));
                //if (v < level - window / 2 || v > level + window / 2)
                if (v==0)
                    scalars->SetComponent(index, 3, 0);
                else
                    scalars->SetComponent(index, 3, 255);
                /*qDebug() << "scale value:" << v << "   "
                         << "color:" << tuple[0] << tuple[1] << tuple[2] << tuple[3];*/
            }
        }
    }


    vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();
    ren->SetBackground(0, 1, 0);
    vtkNew<vtkActor> stlactor;
    vtkNew<vtkPolyDataMapper> mapper;
    auto planes = vtkSmartPointer<vtkPlaneCollection>::New();
    auto plane = vtkSmartPointer<vtkPlane>::New();
    plane->SetOrigin(center[0],center[1],center[2]);
    plane->SetNormal(0, 1, 0);
    planes->AddItem(plane);
    mapper->SetClippingPlanes(planes);
    stlactor->SetMapper(mapper);
    stlactor->GetProperty()->SetAmbient(0);
    mapper->SetInputData(stlreader->GetOutput());
    ren->AddActor(stlactor);

    vtkNew<vtkImageActor> maskActor;
    maskActor->GetMapper()->SetInputData(img);
    /*maskActor->SetPosition(center[0], center[1], center[2]);
    maskActor->RotateX(-90);*/
    maskActor->SetUserMatrix(resliceAxes);

    ren->AddActor(maskActor);
    /*ren->UseDepthPeelingOn();
    ren->UseDepthPeelingForVolumesOn();*/
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> rw = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    rw->AddRenderer(ren);
    rw->SetSize(640, 480);
    /*rw->SetAlphaBitPlanes(1);
    rw->SetMultiSamples(0);*/
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
    //ren->GetActiveCamera()->AddObserver(vtkCommand::ModifiedEvent, cb);

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