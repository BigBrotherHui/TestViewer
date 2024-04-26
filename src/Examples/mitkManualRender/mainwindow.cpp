#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QHBoxLayout>
#include <vtkOutputWindow.h>
#include <QmitkStdMultiWidget.h>
#include <mitkIOUtil.h>
#include <QmitkRenderWindow.h>
#include <QmitkRenderWindowWidget.h>
#include <mitkDataNode.h>
#include <mitkPointSetDataInteractor.h>
#include <mitkNodePredicateDataType.h>
#include <mitkImage.h>
#include <QmitkRegisterClasses.h>
#include <QmitkLevelWindowWidget.h>
//#include <mitkDICOMEnums.h>
//#include <mitkDICOMFilesHelper.h>
//#include <mitkDICOMITKSeriesGDCMReader.h>
//#include <mitkDICOMTagsOfInterestHelper.h>
//#include <mitkDICOMProperty.h>
//#include <mitkDICOMDCMTKTagScanner.h>
//#include <mitkPropertyNameHelper.h>
//#include <mitkPropertyKeyPath.h>
//#include <mitkDICOMIOMetaInformationPropertyConstants.h>
//#include <mitkSplineVtkMapper3D.h>

#include <itkImageSeriesReader.h>
#include <itkGDCMImageIO.h>
#include <itkGDCMSeriesFileNames.h>
#include <mitkITKImageImport.h>
#include <vtkOrientedGlyphContourRepresentation.h>
#include <vtkImageAppend.h>
#include <vtkSplineFilter.h>
#include <vtkImagePermute.h>
#include <vtkImageFlip.h>
#include <vtkLineSource.h>
#include <vtkLine.h>
#include <vtkColorTransferFunction.h>
#include <vtkMetaImageWriter.h>
#include <vtkImageWriter.h>
#include <mitkLine.h>
#include <mitkSliceNavigationHelper.h>
#include <mitkTimeGeometry.h>
#include <QDebug>

MainWindow::MainWindow(QWidget* parent) : QWidget(parent)
    , ui(new Ui::MainWindow)
{
    QmitkRegisterClasses();
    vtkOutputWindow::GlobalWarningDisplayOff();
    ui->setupUi(this);
    ds1=mitk::StandaloneDataStorage::New();
    /*ds2=mitk::StandaloneDataStorage::New();
    ds3=mitk::StandaloneDataStorage::New();*/
    w1=new QmitkRenderWindow(this);
    /*w2=new QmitkStdMultiWidget(this);
    rw=new QmitkRenderWindow(this);*/
    QHBoxLayout *l=new QHBoxLayout(ui->widget);
    l->addWidget(w1);
    /*l->addWidget(w2);
    l->addWidget(rw);*/
    l->setStretch(0,100);
    l->setStretch(1, 1);
    /*l->setStretch(2, 100);
    l->setStretch(3, 100);*/
    w1->GetRenderer()->SetDataStorage(ds1);
    w1->GetSliceNavigationController()->SetDefaultViewDirection(mitk::AnatomicalPlane::Axial);
    w1->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard2D);
    timer = new QTimer(this);
    connect(timer,&QTimer::timeout,this,&MainWindow::slot_timeout);
    timer->setInterval(1000 / 30);
    timer->start();
    mitk::RenderingManager::GetInstance()->RemoveRenderWindow(w1->GetVtkRenderWindow());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_loaddata_clicked()
{
    std::string dicomSeriesPath="D:/Images/SLC";
    using TPixel = signed short;
    const unsigned int DIM3 = 3;
    using TImage = itk::Image<TPixel, DIM3>;
    using TImagePtr = TImage::Pointer;
    using TPoint = TImage::IndexType;
    using ImageType = TImage;
    using ReaderType = itk::ImageSeriesReader<ImageType>;
    using ImageIOType = itk::GDCMImageIO;
    using SeriesFileNamesType = itk::GDCMSeriesFileNames;
    using FileNamesContainer = std::vector<std::string>;
    using SeriesIdContainer = std::vector<std::string>;
    using SeriesIdContainer = std::vector<std::string>;

    ImageIOType::Pointer m_gdcmImageIO;
    SeriesFileNamesType::Pointer m_gdcmSeriesFileNames = SeriesFileNamesType::New();
    ReaderType::Pointer m_gdcmReader;
    m_gdcmReader = ReaderType::New();
    m_gdcmImageIO = ImageIOType::New();

    m_gdcmSeriesFileNames->SetUseSeriesDetails(true);
    m_gdcmSeriesFileNames->SetDirectory(dicomSeriesPath.c_str());
    const SeriesIdContainer& seriesUID = m_gdcmSeriesFileNames->GetSeriesUIDs();
    FileNamesContainer fileNames = m_gdcmSeriesFileNames->GetFileNames(seriesUID[0]);
    m_gdcmReader->SetImageIO(m_gdcmImageIO);
    m_gdcmReader->SetFileNames(fileNames);
    m_gdcmReader->ForceOrthogonalDirectionOff();
    try {
        // Read the files
        m_gdcmReader->Update();
        // Store the itk::Image pointer
        mitk::Image::Pointer mitkImage = mitk::Image::New();
        mitk::GrabItkImageMemory(m_gdcmReader->GetOutput(), mitkImage);
        mitk::DataNode::Pointer imgNode = mitk::DataNode::New();
        imgNode->SetData(mitkImage);
        imgNode->SetName("imageNode");
        ds1->Add(imgNode);
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return;
    }
    mitk::RenderingManager::GetInstance()->InitializeViewByBoundingObjects(w1->GetVtkRenderWindow(),ds1);
    w1->GetRenderer()->PrepareRender();
    w1->renderWindow()->Render();
}

void MainWindow::slot_timeout()
{
    on_pushButton_addpoint_clicked();
    }

void MainWindow::on_pushButton_addpoint_clicked()
{
    
    static int i = 0;
    if (i==0) w1->GetRenderer()->GetSliceNavigationController()->GetSlice()->SetPos(100);
    //w1->GetSliceNavigationController()->GetSlice()->SetPos(i++);
    mitk::Point3D pt3d;
    pt3d[0] = i;
    pt3d[1] = i;
    pt3d[2] = i;
    i++;
    //会闪现一下第5个切层
    //w1->GetRenderer()->GetSliceNavigationController()->SelectSliceByPoint(pt3d);
    qDebug() << w1->GetRenderer()->GetSliceNavigationController()->GetSlice()->GetPos();
    mitk::Point2D pt2d;
    w1->GetRenderer()->GetCurrentWorldPlaneGeometry()->Map(pt3d, pt2d);
    w1->GetRenderer()->GetCameraController()->MoveCameraToPoint(pt2d);
    w1->GetRenderer()->PrepareRender();
    w1->renderWindow()->Render();
}