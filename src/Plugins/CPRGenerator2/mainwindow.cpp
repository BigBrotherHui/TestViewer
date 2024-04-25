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
#include <vtkCardinalSpline.h>
#include <vtkTransformFilter.h>
#include <vtkProbeFilter.h>
#include <vtkPointData.h>

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
        /*vtkSmartPointer<vtkMatrix4x4> sourceMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
        
        double m_axialMatrix[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
        sourceMatrix->DeepCopy(m_axialMatrix);
        vtkImageData* imgData = static_cast<mitk::Image*>(ds1->GetNamedNode("imageNode")->GetData())->GetVtkImageData();
        double center[3];
        double* origin = imgData->GetOrigin();
        int* extent = imgData->GetExtent();
        double* spacing = imgData->GetSpacing();
        center[0] = origin[0] + spacing[0] * 0.5 * (extent[0] + extent[1]);
        center[1] = origin[1] + spacing[1] * 0.5 * (extent[2] + extent[3]);
        center[2] = origin[2] + spacing[2] * 0.5 * (extent[4] + extent[5]);*/

        qint32 n = rep->GetNumberOfNodes();
        vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
        vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
        lines->InsertNextCell(n);
        for (qint32 i = 0; i < n; ++i) {
            double p[3];
            rep->GetNthNodeWorldPosition(i, p);
            /*vtkNew<vtkTransform> transform1;
            transform1->SetMatrix(sourceMatrix);
            transform1->Translate(p[0], p[1], 0);*/
            /*points->InsertNextPoint(transform1->GetMatrix()->GetElement(0, 3), transform1->GetMatrix()->GetElement(1, 3),
                transform1->GetMatrix()->GetElement(2, 3));*/
            points->InsertNextPoint(p[0],p[1],p[2]);
            /*qDebug() << transform1->GetMatrix()->GetElement(0, 3)<< transform1->GetMatrix()->GetElement(1, 3)<<
                transform1->GetMatrix()->GetElement(2, 3);*/
            lines->InsertCellPoint(i);
        }
        vtkSmartPointer<vtkPolyData> p = vtkSmartPointer<vtkPolyData>::New();
        p->SetPoints(points);
        p->SetLines(lines);

        
        auto spline = vtkCardinalSpline::New();
        spline->SetLeftConstraint(2);
        spline->SetLeftValue(0.0);
        spline->SetRightConstraint(2);
        spline->SetRightValue(0.0);

        double segLength{0.3};

        vtkNew<vtkSplineFilter> spline_filter;
        spline_filter->SetSubdivideToLength();
        spline_filter->SetLength(segLength);
        spline_filter->SetInputData(p);
        spline_filter->SetSpline(spline);
        spline_filter->Update();
        auto spline_PolyData = spline_filter->GetOutput();

        auto mitkPsetNew = mitk::PointSet::New();

        for (int i{0}; i < spline_PolyData->GetPoints()->GetNumberOfPoints(); i++) {
            mitk::Point3D a;
            a[0] = spline_PolyData->GetPoint(i)[0];
            a[1] = spline_PolyData->GetPoint(i)[1];
            a[2] = spline_PolyData->GetPoint(i)[2];

            mitkPsetNew->InsertPoint(a);
        }
        auto curveNode = mitk::DataNode::New();
        curveNode->SetData(mitkPsetNew);
        curveNode->SetName("Dental curve");
        curveNode->SetFloatProperty("point 2D size", 0.3);
        GetDataStorage()->Add(curveNode);

        vtkSmartPointer<vtkImageAppend> append = vtkSmartPointer<vtkImageAppend>::New();

        append->SetAppendAxis(2);

        int thickness{30};

        auto attemptImageNode = GetDataStorage()->GetNamedNode("imageNode");

        if (attemptImageNode == nullptr) {
            return;
        }
        auto vtkImage = GetDataStorage()->GetNamedObject<mitk::Image>("imageNode")->GetVtkImageData();

        for (int i{0}; i < 2 * thickness; i++) {
            // caculate how much length every step covers
            double stepSize = segLength * (-thickness + i);
            // expand spline using their normal,expand assume spline in the center and expand thickess
            auto expandedSpline = ExpandSpline(spline_PolyData, spline_PolyData->GetNumberOfPoints() - 1, stepSize);
            // Sweep the line to form a surface.
            double direction[3];
            direction[0] = 0.0;
            direction[1] = 0.0;
            direction[2] = 1.0;
            unsigned cols = 250;

            double distance = cols * segLength;
            // force every spline to generate a wall cross z with quads
            // then the col spacing is depends on spline sample rate,the row spacing is depends on segLength
            auto surface = SweepLine_2Sides(expandedSpline, direction, distance, cols);

            if (i == thickness) {
                if (GetDataStorage()->GetNamedNode("Probe surface") != nullptr) {
                    GetDataStorage()->Remove(GetDataStorage()->GetNamedNode("Probe surface"));
                }

                auto probeSurface = mitk::Surface::New();
                probeSurface->SetVtkPolyData(surface);
                auto probeNode = mitk::DataNode::New();
                probeNode->SetData(probeSurface);
                probeNode->SetName("Probe surface");
                float color[3]{1, 0, 0};
                probeNode->SetColor(color);
                probeNode->SetOpacity(0.5);
                GetDataStorage()->Add(probeNode);
            }

            // Apply the inverse geometry of the MITK image to the probe surface
            auto geometryMatrix = vtkMatrix4x4::New();
            geometryMatrix->DeepCopy(attemptImageNode->GetData()->GetGeometry()->GetVtkMatrix());
            auto spacing = attemptImageNode->GetData()->GetGeometry()->GetSpacing();

            for (int j{0}; j < 3; j++) {
                geometryMatrix->SetElement(j, 0, geometryMatrix->GetElement(j, 0) / spacing[0]);
                geometryMatrix->SetElement(j, 1, geometryMatrix->GetElement(j, 1) / spacing[1]);
                geometryMatrix->SetElement(j, 2, geometryMatrix->GetElement(j, 2) / spacing[2]);
            }

            geometryMatrix->Invert();  // move to the [0,0,0] origin but not change the spacing
            // if directly using identity matrix then the spacing of image will change to [1,1,1]

            vtkNew<vtkTransformFilter> tmpTransFilter;
            vtkNew<vtkTransform> tmpTransform;
            tmpTransform->SetMatrix(geometryMatrix);
            tmpTransFilter->SetTransform(tmpTransform);
            tmpTransFilter->SetInputData(surface);
            tmpTransFilter->Update();

            vtkNew<vtkProbeFilter> sampleVolume;
            sampleVolume->SetSourceData(vtkImage);
            sampleVolume->SetInputData(tmpTransFilter->GetOutput());

            sampleVolume->Update();

            auto probeData = sampleVolume->GetOutput();

            auto probePointData = probeData->GetPointData();

            auto tmpArray = probePointData->GetScalars();
            //经过vtkProbeFilter的结果其采样是曲面，下面要摆直
            auto testimageData = vtkImageData::New();
            testimageData->SetDimensions(cols + 1, spline_PolyData->GetNumberOfPoints(), 1);
            // testimageData->SetDimensions( spline_PolyData->GetNumberOfPoints(), cols + 1, 1);

            // testimageData->SetSpacing(segLength, segLength, 1);
            testimageData->SetSpacing(1, 1, 1);
            testimageData->SetOrigin(0, 0, 0);
            testimageData->AllocateScalars(VTK_INT, 1);
            testimageData->GetPointData()->SetScalars(tmpArray);

            // auto tmpImage = vtkImageData::New();
            // tmpImage->DeepCopy(testimageData);

            append->AddInputData(testimageData);
        }

        append->Update();
        auto appenedImage = append->GetOutput();
        //设置为采样的spacing
        appenedImage->SetSpacing(segLength, segLength, segLength);
        // appenedImage->SetSpacing(1, 1, 1);

        auto mitkAppendedImage = mitk::Image::New();

        mitkAppendedImage->Initialize(appenedImage);
        mitkAppendedImage->SetVolume(appenedImage->GetScalarPointer());

        // Rotate the image by -90 degree along the +z axis
        auto rotateTrans = vtkTransform::New();
        rotateTrans->PostMultiply();
        rotateTrans->SetMatrix(mitkAppendedImage->GetGeometry()->GetVtkMatrix());
        rotateTrans->RotateZ(-90);
        rotateTrans->Update();

        mitkAppendedImage->GetGeometry()->SetIndexToWorldTransformByVtkMatrix(rotateTrans->GetMatrix());

        if (GetDataStorage()->GetNamedNode("Panorama") != nullptr) {
            GetDataStorage()->Remove(GetDataStorage()->GetNamedNode("Panorama"));
        }

        auto tmpNode_ = mitk::DataNode::New();
        tmpNode_->SetData(mitkAppendedImage);
        tmpNode_->SetName("Panorama");
        GetDataStorage()->Add(tmpNode_);

        GetDataStorage()->GetNamedNode("Dental curve")->SetVisibility(true);
        //GetDataStorage()->GetNamedNode("Dental curve seeds")->SetVisibility(true);

        tmpNode_->SetProperty("volumerendering", mitk::BoolProperty::New(1));
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

}

mitk::DataStorage::Pointer MainWindow::GetDataStorage()
{
    return w1->GetDataStorage();
}

#include <vtkSCurveSpline.h>
#include <vtkParametricSpline.h>
#include <vtkParametricFunctionSource.h>
#include <vtkPlane.h>
#include <vtkPolyDataMapper.h>
#include <vtkCutter.h>
#include <vtkImageShiftScale.h>
#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkVolumeProperty.h>
#include <vtkPiecewiseFunction.h>

vtkSmartPointer<vtkPolyData> MainWindow::ExpandSpline(vtkPolyData* line, int divisionNum, double stepSize)
{
    vtkNew<vtkPoints> points;
    for (int i{0}; i < line->GetNumberOfPoints(); i++) {
        Eigen::Vector3d currentPoint;
        currentPoint[0] = line->GetPoint(i)[0];
        currentPoint[1] = line->GetPoint(i)[1];
        currentPoint[2] = line->GetPoint(i)[2];

        Eigen::Vector3d z_axis;
        z_axis[0] = 0;
        z_axis[1] = 0;
        z_axis[2] = 1;

        Eigen::Vector3d ptpVector;
        // caculate normal direction
        if (i == (line->GetNumberOfPoints() - 1)) {
            ptpVector[0] = -line->GetPoint(i - 1)[0] + currentPoint[0];
            ptpVector[1] = -line->GetPoint(i - 1)[1] + currentPoint[1];
            ptpVector[2] = -line->GetPoint(i - 1)[2] + currentPoint[2];
        }
        else {
            ptpVector[0] = line->GetPoint(i + 1)[0] - currentPoint[0];
            ptpVector[1] = line->GetPoint(i + 1)[1] - currentPoint[1];
            ptpVector[2] = line->GetPoint(i + 1)[2] - currentPoint[2];
        }

        Eigen::Vector3d tmpVector;

        tmpVector = z_axis.cross(ptpVector);

        tmpVector.normalize();

        points->InsertNextPoint(currentPoint[0] + tmpVector[0] * stepSize, currentPoint[1] + tmpVector[1] * stepSize,
                                currentPoint[2] + tmpVector[2] * stepSize);
    }

    // vtkCellArrays
    vtkNew<vtkCellArray> lines;
    lines->InsertNextCell(points->GetNumberOfPoints());
    for (unsigned int i = 0; i < points->GetNumberOfPoints(); ++i) {
        lines->InsertCellPoint(i);
    }

    // vtkPolyData
    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetLines(lines);

    auto spline = vtkCardinalSpline::New();
    spline->SetLeftConstraint(2);
    spline->SetLeftValue(0.0);
    spline->SetRightConstraint(2);
    spline->SetRightValue(0.0);

    vtkNew<vtkSplineFilter> splineFilter;
    splineFilter->SetInputData(polyData);
    splineFilter->SetSubdivideToSpecified();
    splineFilter->SetNumberOfSubdivisions(divisionNum);
    splineFilter->SetSpline(spline);
    splineFilter->Update();

    auto spline_PolyData = splineFilter->GetOutput();

    return spline_PolyData;
}

vtkSmartPointer<vtkPolyData> MainWindow::SweepLine_2Sides(vtkPolyData* line, double direction[3], double distance,
    unsigned cols)
{
    unsigned int rows = line->GetNumberOfPoints();
    double spacing = distance / cols;
    vtkNew<vtkPolyData> surface;

    // Generate the points.
    cols++;
    unsigned int numberOfPoints = rows * cols;
    unsigned int numberOfPolys = (rows - 1) * (cols - 1);
    vtkNew<vtkPoints> points;
    points->Allocate(numberOfPoints);
    vtkNew<vtkCellArray> polys;
    polys->Allocate(numberOfPolys * 4);

    double x[3];
    unsigned int cnt = 0;
    for (unsigned int row = 0; row < rows; row++) {
        for (unsigned int col = 0; col < cols; col++) {
            double p[3];
            line->GetPoint(row, p);
            x[0] = p[0] - distance * direction[0] / 2 + direction[0] * col * spacing;
            x[1] = p[1] - distance * direction[1] / 2 + direction[1] * col * spacing;
            x[2] = p[2] - distance * direction[2] / 2 + direction[2] * col * spacing;
            points->InsertPoint(cnt++, x);
        }
    }
    // Generate the quads.
    vtkIdType pts[4];
    for (unsigned int row = 0; row < rows - 1; row++) {
        for (unsigned int col = 0; col < cols - 1; col++) {
            pts[0] = col + row * (cols);
            pts[1] = pts[0] + 1;
            pts[2] = pts[0] + cols + 1;
            pts[3] = pts[0] + cols;
            polys->InsertNextCell(4, pts);
        }
    }
    surface->SetPoints(points);
    surface->SetPolys(polys);

    return surface;
}
