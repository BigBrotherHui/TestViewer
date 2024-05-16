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
#include <QDebug>
#include <vtkPolyDataMapper.h>
#include <vtkCylinderSource.h>
#include <mitkNodePredicateDataType.h>
#include "PointSetDataInteractorScrew.h"
#include "SegmentationWidget.h"
#include "ToolsWidget.h"
#include <usModuleRegistry.h>
#include <mitkAffineBaseDataInteractor3D.h>
#include "PlanningWidget.h"
#include "BoneSegmentationWidget.h"
MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindow)
{
    QmitkRegisterClasses();
    vtkOutputWindow::GlobalWarningDisplayOff();
    ui->setupUi(this);
    ds1=mitk::StandaloneDataStorage::New();

    w1=new QmitkStdMultiWidget(this);

    QHBoxLayout *l=new QHBoxLayout(ui->widget);
    l->addWidget(w1);
    QmitkLevelWindowWidget* level_window_widget = new QmitkLevelWindowWidget(this); 
    l->addWidget(level_window_widget);
    level_window_widget->SetDataStorage(ds1);

    l->setStretch(0,100);
    l->setStretch(1, 1);

    w1->SetDataStorage(ds1);
    w1->InitializeMultiWidget();
    w1->AddPlanesToDataStorage();

    /*PointSetDataInteractorScrew::Pointer mPointSetDataInteractorScrew = PointSetDataInteractorScrew::New();
    mPointSetDataInteractorScrew->LoadStateMachine("PointSet.xml");
    mPointSetDataInteractorScrew->SetEventConfig("PointSetConfig.xml");
    mPointSetDataInteractorScrew->SetMaxPoints(2);*/
    /*itk::ReceptorMemberCommand<MainWindow>::Pointer command = itk::ReceptorMemberCommand<MainWindow>::New();
    command->SetCallbackFunction(this, &MainWindow::ModifyObserved);
    w1->GetRenderWindow1()->GetSliceNavigationController()->AddObserver(mitk::SliceNavigationController::GeometryUpdateEvent(nullptr,0),command);*/
    /*w1->GetRenderWindow2()->GetSliceNavigationController()->AddObserver(
        mitk::SliceNavigationController::GeometryUpdateEvent(nullptr, 0), command);
    w1->GetRenderWindow3()->GetSliceNavigationController()->AddObserver(
        mitk::SliceNavigationController::GeometryUpdateEvent(nullptr, 0), command);*/

    mitk::DataNode::Pointer node = mitk::DataNode::New();
    mitk::Surface::Pointer sur = mitk::Surface::New();
    node->SetData(sur);
    node->SetName("cylinder");
    vtkSmartPointer<vtkCylinderSource> source = vtkSmartPointer<vtkCylinderSource>::New();
    source->SetResolution(30);
    source->SetHeight(100);
    source->SetCapping(1);
    source->SetRadius(30);
    source->Update();
    sur->SetVtkPolyData(source->GetOutput());
    ds1->Add(node);
    mitk::PointSet::Pointer ps = mitk::PointSet::New();
    mitk::DataNode::Pointer psnode = mitk::DataNode::New();
    psnode->SetData(ps);
    ds1->Add(psnode);
    //mPointSetDataInteractorScrew->SetDataNode(node);
    //mPointSetDataInteractorScrew->SetScrew(node);
    //mPointSetDataInteractorScrew->SetScrewDiameter(30);
    //mPointSetDataInteractorScrew->SetScrewLength(100);
    w1->InitializeViews(ds1->ComputeBoundingGeometry3D(ds1->GetAll()), false);
    w1->RequestUpdateAll();


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ModifyObserved(const itk::EventObject& geometryUpdateEvent)
{
    qDebug() << "modified";
    const auto* updateEvent =
        dynamic_cast<const mitk::SliceNavigationController::GeometryUpdateEvent*>(&geometryUpdateEvent);

    if (nullptr == updateEvent) {
        return;
    }
    vtkMatrix4x4* vmt = updateEvent->GetTimeGeometry()->GetGeometryForTimePoint(0)->GetVtkMatrix();
    vmt->Print(std::cout);
    mitk::NodePredicateDataType::Pointer type = mitk::NodePredicateDataType::New("Image");
    auto image = ds1->GetNode(type);
    if (!image) return;
    auto img = static_cast<mitk::Image*>(image->GetData())->GetVtkImageData();
    auto spacing = static_cast<mitk::Image*>(image->GetData())->GetVtkImageData()->GetSpacing();
    qDebug() << "((((((((" << spacing[0] << spacing[1] << spacing[2];
    /*for (int j{0}; j < 3; j++) {
        vmt->SetElement(j, 0, vmt->GetElement(j, 0) / spacing[0]);
        vmt->SetElement(j, 1, vmt->GetElement(j, 1) / spacing[1]);
        vmt->SetElement(j, 2, vmt->GetElement(j, 2) / spacing[2]);
    }*/
    double* origin = img->GetOrigin();
    int* extent = img->GetExtent();
    double center[4]{0,0,0,1};
    center[0] = origin[0] + spacing[0] * 0.5 * (extent[0] + extent[1]);
    center[1] = origin[1] + spacing[1] * 0.5 * (extent[2] + extent[3]);
    center[2] = origin[2] + spacing[2] * 0.5 * (extent[4] + extent[5]);
    //auto mt1=static_cast<mitk::Image*>(image->GetData())->GetGeometry()->GetVtkMatrix();
    //double *c=mt1->MultiplyDoublePoint(center);
    w1->SetSelectedPosition(mitk::Point3D(center), "");
    /*for (int i = 0; i < w1->GetNumberOfRenderWindowWidgets(); ++i) {
        w1->GetRenderWindow(i)->GetSliceNavigationController()->AdjustSliceStepperRange();
    }*/
    ds1->GetNamedNode("cylinder")->GetData()->GetGeometry()->SetIndexToWorldTransformByVtkMatrix(vmt);
    /*auto mt = vtkSmartPointer<vtkMatrix4x4>::New();
    mt->SetElement(0, 3, w1->GetSelectedPosition("")[0]);
    mt->SetElement(1, 3, w1->GetSelectedPosition("")[1]);
    mt->SetElement(2, 3, w1->GetSelectedPosition("")[2]);
    ds1->GetNamedNode("cylinder")->GetData()->GetGeometry()->SetIndexToWorldTransformByVtkMatrix(mt);*/
    w1->RequestUpdateAll();
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
        imgNode->SetName("image");
        imgNode->SetVisibility(0, w1->GetRenderWindow4()->GetRenderer());
        //imgNode->SetProperty("volumerendering", mitk::BoolProperty::New(1));
        ds1->Add(imgNode);
        /*SegmentationWidget* w = new SegmentationWidget();
        w->show();
        ToolsWidget* w1 = new ToolsWidget;
        w1->show();*/
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return;
    }
    auto* image = ds1->GetNamedObject<mitk::Image>("image");
    auto center = image->GetGeometry()->GetCenter();
    mitk::Vector3D translate(center.GetVnlVector());

    auto node = ds1->GetNamedNode("cylinder");
    auto* data = node->GetData();
    data->GetGeometry()->Translate(translate);

    node->SetBoolProperty("pickable", true);

    auto affineDataInteractor = mitk::AffineBaseDataInteractor3D::New();
    affineDataInteractor->LoadStateMachine("AffineInteraction3D.xml",
                                           us::ModuleRegistry::GetModule("MitkDataTypesExt"));
    affineDataInteractor->SetEventConfig("AffineMouseConfig.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
    affineDataInteractor->SetDataNode(node);


    BoneSegmentationWidget* w2 = new BoneSegmentationWidget;
    w2->show();
    mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(ds1);
    w1->ResetCrosshair();

    /*vtkPolyData* source =
        static_cast<mitk::Surface*>(ds1->GetNamedNode("cylinder")->GetData())->GetVtkPolyData();
    w1->SetSelectedPosition(mitk::Point3D(source->GetCenter()), "");
    w1->RequestUpdateAll();

    vtkNew<vtkTransform> trans;
    vtkNew<vtkMatrix4x4> vmt;
    vmt->SetElement(0, 1, 0);
    vmt->SetElement(1, 1, 0);
    vmt->SetElement(2, 1, -1);
    vmt->SetElement(0, 2, 0);
    vmt->SetElement(1, 2, 1);
    vmt->SetElement(2, 2, 0);
    vmt->SetElement(0, 3, source->GetCenter()[0]);
    vmt->SetElement(1, 3, source->GetCenter()[1]);
    vmt->SetElement(2, 3, source->GetCenter()[2]);
    
    trans->SetMatrix(vmt);
    trans->RotateX(30);
    auto mt=trans->GetMatrix();
    mitk::Vector3D normal;
    normal.SetElement(0, mt->GetElement(0,1));
    normal.SetElement(1, mt->GetElement(1, 1));
    normal.SetElement(2, mt->GetElement(2, 1));
    w1->GetRenderWindow1()->GetSliceNavigationController()->ReorientSlices(mitk::Point3D(source->GetCenter()), normal);
    w1->RequestUpdateAll();*/
}
