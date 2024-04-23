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
#include "CBCTSplineDrivenImageSlicer.h"
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

MainWindow::MainWindow(QWidget* parent) : QWidget(parent)
    , ui(new Ui::MainWindow)
{
    QmitkRegisterClasses();
    vtkOutputWindow::GlobalWarningDisplayOff();
    ui->setupUi(this);
    ds1=mitk::StandaloneDataStorage::New();
    /*ds2=mitk::StandaloneDataStorage::New();
    ds3=mitk::StandaloneDataStorage::New();*/
    w1=new QmitkStdMultiWidget(this);
    /*w2=new QmitkStdMultiWidget(this);
    rw=new QmitkRenderWindow(this);*/
    QHBoxLayout *l=new QHBoxLayout(ui->widget);
    l->addWidget(w1);
    QmitkLevelWindowWidget* level_window_widget = new QmitkLevelWindowWidget(this); 
    l->addWidget(level_window_widget);
    level_window_widget->SetDataStorage(ds1);
    /*l->addWidget(w2);
    l->addWidget(rw);*/
    l->setStretch(0,100);
    l->setStretch(1, 1);
    /*l->setStretch(2, 100);
    l->setStretch(3, 100);*/
    w1->SetDataStorage(ds1);
    //w2->SetDataStorage(ds2);
    //rw->GetRenderer()->SetDataStorage(ds3);
    //rw->GetSliceNavigationController()->SetDefaultViewDirection(mitk::AnatomicalPlane::Coronal);
    //rw->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);
    w1->InitializeMultiWidget();
    //w2->InitializeMultiWidget();
    w1->AddPlanesToDataStorage();
    //w2->AddPlanesToDataStorage();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SlotContourEndInteractionEvent(vtkObject* t_obj, unsigned long, void*, void*)
{
    qDebug() << "SlotContourEndInteractionEvent";
    auto widget = dynamic_cast<vtkContourWidget*>(t_obj);
    auto rep = dynamic_cast<vtkOrientedGlyphContourRepresentation*>(widget->GetContourRepresentation());
    if (rep) {
        vtkSmartPointer<vtkMatrix4x4> sourceMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
        //vtkMatrix4x4* curPlaneGeo = ;
        /*double pt[3]{ 100, 200, 200 };
        mitk::Point3D mpt(pt);*/
    	//w1->GetRenderWindow1()->GetSliceNavigationController()->SelectSliceByPoint(mpt);
        /*const mitk::PlaneGeometry* plane1 = w1->GetRenderWindow1()->GetSliceNavigationController()->GetCurrentPlaneGeometry();
        const mitk::PlaneGeometry* plane2 = w1->GetRenderWindow2()->GetSliceNavigationController()->GetCurrentPlaneGeometry();
        const mitk::PlaneGeometry* plane3 = w1->GetRenderWindow3()->GetSliceNavigationController()->GetCurrentPlaneGeometry();
        mitk::Line3D line;
        if ((plane1 != nullptr) && (plane2 != nullptr)
            && (plane1->IntersectionLine(plane2, line)))
        {
            mitk::Point3D point;
            if ((plane3 != nullptr) && (plane3->IntersectionPoint(line, point)))
            {
                sourceMatrix->SetElement(0, 3, point[0]);
                sourceMatrix->SetElement(1, 3, point[1]);
                sourceMatrix->SetElement(2, 3, point[2]);
            }
        }*/

        double m_axialMatrix[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
        sourceMatrix->DeepCopy(m_axialMatrix);
        vtkImageData* imgData = static_cast<mitk::Image*>(ds1->GetNamedNode("imageNode")->GetData())->GetVtkImageData();
        double center[3];
        double* origin = imgData->GetOrigin();
        int* extent = imgData->GetExtent();
        double* spacing = imgData->GetSpacing();
        center[0] = origin[0] + spacing[0] * 0.5 * (extent[0] + extent[1]);
        center[1] = origin[1] + spacing[1] * 0.5 * (extent[2] + extent[3]);
        center[2] = origin[2] + spacing[2] * 0.5 * (extent[4] + extent[5]);
        /*sourceMatrix->SetElement(0, 3, center[0]);
        sourceMatrix->SetElement(1, 3, center[1]);
        sourceMatrix->SetElement(2, 3, center[2]);
        sourceMatrix->Modified();*/
        qint32 n = rep->GetNumberOfNodes();
        vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
        vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
        lines->InsertNextCell(n);
        for (qint32 i = 0; i < n; ++i) {
            double p[3];
            rep->GetNthNodeWorldPosition(i, p);
            vtkNew<vtkTransform> transform1;
            transform1->SetMatrix(sourceMatrix);
            transform1->Translate(p[0], p[1], 0);
            points->InsertNextPoint(transform1->GetMatrix()->GetElement(0, 3), transform1->GetMatrix()->GetElement(1, 3),
                transform1->GetMatrix()->GetElement(2, 3));
            qDebug() << transform1->GetMatrix()->GetElement(0, 3)<< transform1->GetMatrix()->GetElement(1, 3)<<
                transform1->GetMatrix()->GetElement(2, 3);
            lines->InsertCellPoint(i);
        }
        vtkSmartPointer<vtkPolyData> p = vtkSmartPointer<vtkPolyData>::New();
        p->SetPoints(points);
        p->SetLines(lines);
        
        vtkNew<vtkSplineFilter> spline_filter;
        spline_filter->SetSubdivideToLength();
        spline_filter->SetLength(0.2);
        spline_filter->SetInputData(p);
        spline_filter->Update();
        vtkNew<vtkImageAppend> append;
        append->SetAppendAxis(2);
        vtkNew<CBCTSplineDrivenImageSlicer> reslicer;
        reslicer->SetInputData(imgData);
        reslicer->SetPathConnection(spline_filter->GetOutputPort());
        long long nb_points = spline_filter->GetOutput()->GetNumberOfPoints();
        qDebug() << "nb_points after splinefilter:" << nb_points;
        for (int pt_id = 0; pt_id < nb_points; pt_id++) {
            reslicer->Setoffset_point_(pt_id);
            reslicer->Update();
            vtkNew<vtkImageData> tempSlice;
            tempSlice->DeepCopy(reslicer->GetOutput());
            append->AddInputData(tempSlice);
        }
        append->Update();
        qDebug() << "range1 append:" << append->GetOutput()->GetScalarRange()[1];

        vtkNew<vtkImagePermute> permute_filter;
        permute_filter->SetInputData(append->GetOutput());
        permute_filter->SetFilteredAxes(2, 0, 1);
        permute_filter->Update();
        vtkNew<vtkImageFlip> flip_filter;
        flip_filter->SetInputData(permute_filter->GetOutput());
        flip_filter->SetFilteredAxes(1);
        flip_filter->Update();
        /*vtkNew<vtkMetaImageWriter> wr;
        wr->SetInputData(flip_filter->GetOutput());
        wr->SetFileName("cpr.mhd");
        wr->SetRAWFileName("cpr.raw");
        wr->Write();*/
        qDebug()<<"range1:"<<flip_filter->GetOutput()->GetScalarRange()[1];
    	mitk::DataNode::Pointer imgNode = mitk::DataNode::New();
        mitk::Image::Pointer img = mitk::Image::New();
        img->Initialize(flip_filter->GetOutput());
        img->SetImportVolume(flip_filter->GetOutput()->GetScalarPointer());
        imgNode->SetData(img);
        ds1->Add(imgNode);
        imgNode->SetProperty("volumerendering", mitk::BoolProperty::New(1));
        w1->ResetView();
        w1->RequestUpdateAll();
    }
}

//std::string GenerateNameFromDICOMProperties(const mitk::IPropertyProvider* provider)
//{
//    std::string nodeName = mitk::DataNode::NO_NAME_VALUE();
//
//    auto studyProp = provider->GetConstProperty(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x1030).c_str());
//    if (studyProp.IsNotNull())
//    {
//        nodeName = studyProp->GetValueAsString();
//    }
//
//    auto seriesProp = provider->GetConstProperty(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x103E).c_str());
//
//    if (seriesProp.IsNotNull())
//    {
//        if (studyProp.IsNotNull())
//        {
//            nodeName += " / ";
//        }
//        else
//        {
//            nodeName = "";
//
//        }
//        nodeName += seriesProp->GetValueAsString();
//    }
//
//    return nodeName;
//};

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
        imgNode->SetVisibility(0, w1->GetRenderWindow4()->GetRenderer());
        //imgNode->SetProperty("volumerendering", mitk::BoolProperty::New(1));
        ds1->Add(imgNode);
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return;
    }
    //mitk::IOUtil::Load(dicomSeriesPath,*ds1);
    //mitk::IOUtil::Load("D:/Images/Pelvis_Right.stl", *ds2);
    /*auto subset=ds1->GetSubset(mitk::NodePredicateDataType::New("Image"));
    for(auto iter : *subset){
        iter->SetProperty("volumerendering",mitk::BoolProperty::New(1));
    }
    w1->ResetCrosshair();
    ds1->GetNamedNode("stdmulti.widget0.plane")->GetData()->Print(std::cout);
    w1->RequestUpdateAll();*/

    /*mitk::StringList relevantFiles = mitk::GetDICOMFilesInSameDirectory(dicomSeriesPath);
    mitk::DICOMITKSeriesGDCMReader::Pointer reader = mitk::DICOMITKSeriesGDCMReader::New();
    std::vector< std::string > m_ReadFiles;
    const unsigned int ntotalfiles = relevantFiles.size();
    for (unsigned int i = 0; i < ntotalfiles; i++)
    {
        m_ReadFiles.push_back(relevantFiles.at(i));
    }
    reader->SetAdditionalTagsOfInterest(mitk::GetCurrentDICOMTagsOfInterest());
    reader->SetTagLookupTableToPropertyFunctor(mitk::GetDICOMPropertyForDICOMValuesFunctor);
    reader->SetInputFiles(relevantFiles);

    mitk::DICOMDCMTKTagScanner::Pointer scanner = mitk::DICOMDCMTKTagScanner::New();
    scanner->AddTagPaths(reader->GetTagsOfInterest());
    scanner->SetInputFiles(relevantFiles);
    scanner->Scan();

    reader->SetTagCache(scanner->GetScanCache());
    reader->AnalyzeInputFiles();
    reader->LoadImages();
    std::vector<mitk::BaseData::Pointer> result;
    for (unsigned int i = 0; i < reader->GetNumberOfOutputs(); ++i)
    {
        const mitk::DICOMImageBlockDescriptor& desc = reader->GetOutput(i);
        mitk::BaseData::Pointer data = desc.GetMitkImage().GetPointer();

        std::string nodeName = GenerateNameFromDICOMProperties(&desc);

        mitk::StringProperty::Pointer nameProp = mitk::StringProperty::New(nodeName);
        data->SetProperty("name", nameProp);
        data->GetGeometry()->Print(std::cout);
        data->SetProperty(mitk::PropertyKeyPathToPropertyName(
            mitk::DICOMIOMetaInformationPropertyConstants::READER_CONFIGURATION()), mitk::StringProperty::New(reader->GetConfigurationLabel()));
        result.push_back(data);
    }
    for(int i=0;i<result.size();++i)
    {
        mitk::DataNode::Pointer dt = mitk::DataNode::New();
        dt->SetData(result[i]);
        ds1->Add(dt);
    }*/
    mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(ds1);
    w1->ResetCrosshair();
}


void MainWindow::on_pushButton_addpoint_clicked()
{
    if(!m_ps)
    {
        mitk::DataNode::Pointer dt = mitk::DataNode::New();
        m_ps = mitk::PointSet::New();
        dt->SetData(m_ps);
        //mitk::SplineVtkMapper3D::Pointer mapper = mitk::SplineVtkMapper3D::New();
        //dt->SetMapper(mitk::BaseRenderer::Standard3D, mapper);
        dt->SetProperty("pointsize", mitk::FloatProperty::New(10.));
        dt->SetName("pointNode");
        mitk::PointSetDataInteractor::Pointer inter = mitk::PointSetDataInteractor::New();
        inter->SetMaxPoints(10);
        inter->LoadStateMachine("PointSet.xml");
        inter->SetEventConfig("PointSetConfig.xml");
        inter->SetDataNode(dt);
        dt->SetVisibility(0, w1->GetRenderWindow4()->GetRenderer());

        ds1->Add(dt);
    }
    static bool f{ false };
    f = !f;
    if(!f)
    {
        m_ps->Clear();
    }
}

void MainWindow::on_pushButton_cpr_clicked()
{
    if(!m_contour_widget_)
    {
        m_contour_widget_ = vtkContourWidget::New();
        m_contour_widget_->SetInteractor(w1->GetRenderWindow1()->GetInteractor());
        m_contour_widget_->CreateDefaultRepresentation();
        m_contour_widget_->On();
    }
    if (!m_scrollConnection)
    {
        m_scrollConnection = vtkSmartPointer<vtkEventQtSlotConnect>::New();
        m_scrollConnection->Connect(
            m_contour_widget_, vtkCommand::EndInteractionEvent, this, SLOT(
                SlotContourEndInteractionEvent(
                    vtkObject*, unsigned long, void*, void*))
            , nullptr, 0.0, Qt::UniqueConnection);
    }
    
}

void MainWindow::on_pushButton_sharedTo3_clicked()
{
    auto ps=static_cast<mitk::PointSet*>(ds1->GetNamedNode("pointNode")->GetData());
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    for(int i=0;i<ps->GetSize();++i)
    {
        points->InsertNextPoint(ps->GetPoint(i).data());
    }
    auto actor=doCurvePlanReformation(points);
    w1->GetRenderWindow4()->GetRenderer()->GetVtkRenderer()->AddActor(actor);
    w1->GetRenderWindow4()->GetVtkRenderWindow()->Render();
}
#include <vtkSCurveSpline.h>
#include <vtkParametricSpline.h>
#include <vtkParametricFunctionSource.h>
#include <vtkSplineDrivenImageSlicer.h>
#include <vtkPlane.h>
#include <vtkPolyDataMapper.h>
#include <vtkCutter.h>
#include <vtkImageShiftScale.h>
#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkVolumeProperty.h>
#include <vtkPiecewiseFunction.h>
vtkActor* MainWindow::doCurvePlanReformation(vtkPoints* route) {

    vtkSCurveSpline* xSpline = vtkSCurveSpline::New();
    vtkSCurveSpline* ySpline = vtkSCurveSpline::New();
    vtkSCurveSpline* zSpline = vtkSCurveSpline::New();
    vtkParametricSpline* spline = vtkParametricSpline::New();
    vtkParametricFunctionSource* functionSource = vtkParametricFunctionSource::New();

    vtkSplineDrivenImageSlicer* reslicer = vtkSplineDrivenImageSlicer::New();
    vtkImageAppend* append = vtkImageAppend::New();

    //! cpr slice
    vtkPolyDataMapper* cprmapper = vtkPolyDataMapper::New();
    vtkActor* cpractor = vtkActor::New();

    vtkPlane* pPlane = vtkPlane::New();
    vtkCutter* pCut = vtkCutter::New();
    vtkImageShiftScale* m_pShift = vtkImageShiftScale::New();

    //! cpr volume
    vtkFixedPointVolumeRayCastMapper* cprVolumeMapper = vtkFixedPointVolumeRayCastMapper::New();
    vtkVolume* cprvolume = vtkVolume::New();
    vtkColorTransferFunction* cprcolorTranFun = vtkColorTransferFunction::New();
    vtkVolumeProperty* cprVolumeproperty = vtkVolumeProperty::New();
    vtkPiecewiseFunction* cprPieceFun = vtkPiecewiseFunction::New();

    spline->SetXSpline(xSpline);
    spline->SetYSpline(ySpline);
    spline->SetZSpline(zSpline);
    spline->SetPoints(route);
    functionSource->SetParametricFunction(spline);
    functionSource->SetUResolution(200);
    functionSource->SetVResolution(200);
    functionSource->SetWResolution(200);
    functionSource->Update();
    vtkImageData* imgData = static_cast<mitk::Image*>(ds1->GetNamedNode("imageNode")->GetData())->GetVtkImageData();
    reslicer->SetInputData(imgData);
    reslicer->SetPathConnection(functionSource->GetOutputPort());
    reslicer->SetSliceSpacing(0.2, 0.1);
    reslicer->SetSliceThickness(0.8);
    reslicer->SetSliceExtent(200, 200);
    reslicer->SetOffsetPoint(30);

    long long nbPoints = functionSource->GetOutput()->GetNumberOfPoints();
    for (int ptId = 0; ptId < nbPoints; ptId++) {
        reslicer->SetOffsetPoint(ptId);
        reslicer->Update();
        vtkImageData* tempSlice = vtkImageData::New();
        tempSlice->DeepCopy(reslicer->GetOutput(0));
        append->AddInputData(tempSlice);
    }
    append->SetAppendAxis(2);
    append->Update();

    //    vtkSmartPointer<vtkMetaImageWriter> writer =
    //         vtkSmartPointer<vtkMetaImageWriter>::New();
    //      writer->SetInputConnection(append->GetOutputPort());
    //      writer->SetFileName("C:\\Users\\cheng\\Desktop\\hehe\\hehe.mhd");
    //      writer->SetRAWFileName("C:\\Users\\cheng\\Desktop\\hehe\\hehe.raw");
    //      writer->Write();

        //! TODO append->GetOutput()
    cprVolumeMapper->SetInputConnection(append->GetOutputPort());
    cprcolorTranFun->AddRGBSegment(0, 1, 1, 1, 255, 1, 1, 1);
    cprPieceFun->AddSegment(0, 0, 3000, 1);
    cprPieceFun->AddPoint(20, 0.2);
    cprPieceFun->AddPoint(80, 0.5);
    cprPieceFun->AddPoint(120, 0.7);
    cprPieceFun->AddPoint(200, 0.9);
    cprPieceFun->ClampingOff();
    cprVolumeMapper->SetBlendModeToMaximumIntensity();
    cprVolumeproperty->SetColor(cprcolorTranFun);
    cprVolumeproperty->SetScalarOpacity(cprPieceFun);
    cprVolumeproperty->SetInterpolationTypeToLinear();
    cprVolumeproperty->ShadeOff();
    cprvolume->SetProperty(cprVolumeproperty);
    cprvolume->SetMapper(cprVolumeMapper);

    double range[2];
    imgData->GetScalarRange(range);
    m_pShift->SetShift(-1.0 * range[0]);
    m_pShift->SetScale(255.0 / (range[1] - range[0]));
    m_pShift->SetOutputScalarTypeToUnsignedChar();
    m_pShift->SetInputConnection(append->GetOutputPort());
    m_pShift->ReleaseDataFlagOff();
    m_pShift->Update();

    pPlane->SetOrigin(cprvolume->GetCenter());
    pPlane->SetNormal(1, 1, 0);
    pCut->SetCutFunction(pPlane);
    pCut->SetInputConnection(m_pShift->GetOutputPort());
    pCut->Update();

    cprmapper->SetInputData(pCut->GetOutput());
    cpractor->SetMapper(cprmapper);
    return cpractor;
}