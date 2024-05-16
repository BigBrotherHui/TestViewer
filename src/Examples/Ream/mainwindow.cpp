#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QVTKOpenGLNativeWidget.h>
#include <QVBoxLayout>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkSphereSource.h>
#include <vtkRenderer.h>
#include <vtkSTLReader.h>
#include <vtkProperty.h>
#include <vtkRendererCollection.h>
#include <vtkMatrix4x4.h>
#include <vtkCamera.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTransform.h>
#include <QDebug>
#include <vtkMetaImageReader.h>
#include "NTBooleanOperationFilter.h"
#include <QThread>
#include <vtkDICOMReader.h>
#include <vtkDICOMSorter.h>
#include <vtkStringArray.h>
#include <vtkMetaImageWriter.h>
#include <vtkInformation.h>
#include <vtkMatrix3x3.h>
#include <itkImage.h>
#include <itkImageSeriesReader.h>
#include <itkGDCMImageIO.h>
#include <itkGDCMSeriesFileNames.h>
#include <itkImageToVTKImageFilter.h>
#include <vtkClipPolyData.h>
#include <vtkBox.h>
#include <vtkMarchingCubes.h>
#include <vtkDiscreteFlyingEdges3D.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkExtractVOI.h>
#include <vtkAppendPolyData.h>
//pelvis.mhd

#define VTK_CREATE(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mwidget = new QVTKOpenGLNativeWidget(ui->widget);
    QVBoxLayout *l = new QVBoxLayout(ui->widget);
    l->addWidget(mwidget);
    mwidget->setRenderWindow(rw);
    rw->AddRenderer(mrenderer);
    vtkNew<vtkSTLReader> reader;
    reader->SetFileName("D:\\__AA_raasystem__\\build\\bin\\x64\\Release\\prosthesisdata\\MeshSTL\\THA_Femur_Left.stl");
    reader->Update();
    reader->GetOutput()->GetCenter(center);
    out = reader->GetOutput();
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(reader->GetOutput());
    actor->SetMapper(mapper);
    actor->GetProperty()->SetSpecular(0.);
    actor->GetProperty()->SetAmbient(0.);
    mrenderer->AddActor(actor);

 
    //std::string path = "D:\\Images\\ct";
    //vtkNew<vtkDICOMReader> newReader;
    //vtkNew<vtkStringArray> sinleFramesImages;
    //vtkNew<vtkDICOMSorter> sorter;
    //int count = 0;
    //QDir dir(QString::fromStdString(path));
    //auto list = dir.entryInfoList(QStringList() << "*.dcm", QDir::Files);
    //for (const auto &im : list) {
    //    const auto path = im.absoluteFilePath();
    //    if (!path.isEmpty()) {
    //        sinleFramesImages->InsertValue(count++, path.toStdString());
    //    }
    //}
    //sorter->SetInputFileNames(sinleFramesImages);
    //sorter->Update();
    ////newReader->SetMemoryRowOrderToFileNative();
    //newReader->SetFileNames(sorter->GetFileNamesForSeries(0));
    ////newReader->SetDataByteOrderToLittleEndian();
    //newReader->Update();
    //vtkSmartPointer<vtkMatrix3x3> vmt = vtkSmartPointer<vtkMatrix3x3>::New();
    //for (int i = 0; i < 3; ++i) {
    //    for (int j = 0; j < 3; ++j) {
    //        vmt->SetElement(i, j, newReader->GetPatientMatrix()->GetElement(i, j));
    //    }
    //}
    //newReader->GetOutput()->SetDirectionMatrix(vmt);
    //newReader->GetOutput()->SetOrigin(newReader->GetPatientMatrix()->GetElement(0, 3),
    //                                  newReader->GetPatientMatrix()->GetElement(1, 3),
    //                                  newReader->GetPatientMatrix()->GetElement(2, 3));
    //开始读取DICOM数据序列
    typedef signed short shortPixelType;
    const unsigned int Dim = 3;  //数据的Dimension

    typedef itk::Image<shortPixelType, Dim> ShortImageType;
    typedef itk::ImageSeriesReader<ShortImageType> ReaderType;

    itk::GDCMImageIO::Pointer gdcmIO = itk::GDCMImageIO::New();
    itk::GDCMSeriesFileNames::Pointer seriesFileNames = itk::GDCMSeriesFileNames::New();

    seriesFileNames->SetDirectory("D:\\Images\\ct");
    const itk::GDCMSeriesFileNames::SeriesUIDContainerType &seriesUIDs = seriesFileNames->GetSeriesUIDs();
    const ReaderType::FileNamesContainer &filenames = seriesFileNames->GetFileNames(seriesUIDs[0]);

    typename ReaderType::Pointer itkreader = ReaderType::New();
    try {
        itkreader->SetImageIO(gdcmIO);
        itkreader->SetFileNames(filenames);
        itkreader->Update();
    } catch (itk::ExceptionObject &ex) {
        //读取过程发生错误
        std::cerr << "Error: " << ex << std::endl;
        return ;
    }
    itk::ImageToVTKImageFilter<ShortImageType>::Pointer cast = itk::ImageToVTKImageFilter<ShortImageType>::New();
    cast->SetInput(itkreader->GetOutput());
    cast->Update();
    vtkImageData *rt = cast->GetOutput();
    image = vtkSmartPointer<vtkImageData>::New();
    vtkSmartPointer<vtkInformation> in = vtkSmartPointer<vtkInformation>::New();
    image->SetScalarType(VTK_UNSIGNED_CHAR, in);
    image->SetSpacing(rt->GetSpacing());
    image->SetOrigin(rt->GetOrigin());
    image->SetExtent(rt->GetExtent());
    image->SetDimensions(rt->GetDimensions());
    image->SetNumberOfScalarComponents(1, in);
    image->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
    int *Extents = image->GetExtent();
    double *Spacing = image->GetSpacing();
    double *Origin = image->GetOrigin();
#pragma omp parallel for
    for (int i = Extents[0]; i <= Extents[1]; i++) {
        for (int j = Extents[2]; j <= Extents[3]; j++) {
            for (int k = Extents[4]; k <= Extents[5]; k++) {
                double pixelPos[3] = {i * Spacing[0] + Spacing[0] / 2 + Origin[0],
                                      j * Spacing[1] + Spacing[1] / 2 + Origin[1],
                                      k * Spacing[2] + Spacing[2] / 2 + Origin[2]};
                auto *pixel = static_cast<short *>(rt->GetScalarPointer(i, j, k));
                if (*pixel > 0) {
                    *static_cast<uchar *>(image->GetScalarPointer(i, j, k)) = 1;
                }
                else
                    *static_cast<uchar *>(image->GetScalarPointer(i, j, k)) = 0;
            }
        }
    }
    image->Modified();
    //vtkNew<vtkMetaImageWriter> writer;
    //writer->SetFileName("22222222222222.mhd");
    //writer->SetInputData(image);
    //writer->Write();
    connect(&watcher, &QFutureWatcher<void>::finished, this, [&] { rw->Render(); });
}

void MainWindow::on_pushButton_move_clicked()
{
    //t->start();
    double c[3];    
    c[0] = center[0]-10;
    c[1] = center[1];
    c[2] = center[2];
    sp->SetCenter(c);
    sp->SetRadius(20);
    sp->SetPhiResolution(50);
    sp->SetThetaResolution(50);
    sp->Update();
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(sp->GetOutput());
    actorsphere->SetMapper(mapper);
    actorsphere->GetProperty()->SetColor(1, 0, 0);
    if (!mrenderer->HasViewProp(actorsphere))
        mrenderer->AddActor(actorsphere);

    vtkNew<vtkClipPolyData> clipper;
    clipper->SetInputData(out);
    vtkNew<vtkBox> box;
    box->SetBounds(sp->GetOutput()->GetBounds());
    clipper->SetClipFunction(box);
    clipper->Update();

    static_cast<vtkPolyDataMapper *>(actor->GetMapper())->SetInputData(clipper->GetOutput());

    double *Center = sp->GetOutput()->GetCenter();
    double Radius = 20;
    double *Origin = image->GetOrigin();
    double *Spacing = image->GetSpacing();
    int *CutExtents=image->GetExtent();
    int Extents[6];
    Extents[0] = std::max(static_cast<int>(floor((Center[0] - Origin[0] - Radius) / Spacing[0])), CutExtents[0]);
    Extents[1] = std::min(static_cast<int>(ceil((Center[0] - Origin[0] + Radius) / Spacing[0])), CutExtents[1]);
    Extents[2] = std::max(static_cast<int>(floor((Center[1] - Origin[1] - Radius) / Spacing[1])), CutExtents[2]);
    Extents[3] = std::min(static_cast<int>(ceil((Center[1] - Origin[1] + Radius) / Spacing[1])), CutExtents[3]);
    Extents[4] = std::max(static_cast<int>(floor((Center[2] - Origin[2] - Radius) / Spacing[2])), CutExtents[4]);
    Extents[5] = std::min(static_cast<int>(ceil((Center[2] - Origin[2] + Radius) / Spacing[2])), CutExtents[5]);

#pragma omp parallel for
    for (int i = Extents[0]; i <= Extents[1]; i++) {
        for (int j = Extents[2]; j <= Extents[3]; j++) {
            for (int k = Extents[4]; k <= Extents[5]; k++) {
                double pixelPos[3] = {i * Spacing[0] + Spacing[0] / 2 + Origin[0],
                                      j * Spacing[1] + Spacing[1] / 2 + Origin[1],
                                      k * Spacing[2] + Spacing[2] / 2 + Origin[2]};
                const double dis = vtkMath::Distance2BetweenPoints(pixelPos, Center);
                auto *pixel = static_cast<uchar *>(image->GetScalarPointer(i, j, k));
                if (dis < Radius * Radius) {
                    *pixel = 0;
                }
            }
        }
    }
    image->Modified();

    vtkNew<vtkExtractVOI> extract;
    extract->SetVOI(Extents);
    extract->SetInputData(image);
    extract->Update();

    vtkNew<vtkMarchingCubes> marchingcube;
    marchingcube->SetInputData(extract->GetOutput());
    //marchingcube->SetComputeGradients(false);
    //marchingcube->SetComputeNormals(false);
    //marchingcube->SetComputeScalars(true);
    marchingcube->SetNumberOfContours(1);
    marchingcube->SetValue(0, 1);
    marchingcube->Update();

    vtkNew<vtkAppendPolyData> append;
    append->AddInputData(marchingcube->GetOutput());
    append->AddInputData(clipper->GetOutput());
    append->Update();
    static_cast<vtkPolyDataMapper *>(actor->GetMapper())->SetInputData(append->GetOutput());

     //vtkNew<vtkMetaImageWriter> writer;
     //writer->SetFileName("33333333333333.mhd");
     //writer->SetInputData(image);
     //writer->Write();
    rw->Render();
}

void MainWindow::on_pushButton_hide_clicked()
{
    actorsphere->SetVisibility(0);
    rw->Render();
}
