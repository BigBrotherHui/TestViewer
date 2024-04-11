#include "ct_image.h"
#include <vtkDICOMImageReader.h>
#include <vtkCommand.h>
#include <vtkImageCast.h>
#include <vtkMatrix4x4.h>
#include <itkImage.h>
#include <itkImageSeriesReader.h>
#include <itkGDCMSeriesFileNames.h>
#include <itkImageToVTKImageFilter.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkImageStencil.h>
#include <vtkConeSource.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkImageBlend.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataToImageStencil.h>
#include <itkCommand.h>
#include <QDebug>
#include <vtkImageFlip.h>
#include <itkMinimumMaximumImageCalculator.h>
#include <itkVTKImageToImageFilter.h>
#include <itkImageSeriesWriter.h>
#include <itkNumericSeriesFileNames.h>
#include "uiHelper.h"
#include <vtkPolyDataMapper.h>
#include <itkJPEGImageIO.h>
#include <itkIntensityWindowingImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkShiftScaleImageFilter.h>
#include <itkThresholdImageFilter.h>
#include <itkCastImageFilter.h>
#include <QFileDialog>

CT_Image::CT_Image()
{
}


CT_Image::~CT_Image()
{
    this->ctImage = nullptr;
}

class CommandProgressUpdate : public itk::Command
{
public:
    typedef  CommandProgressUpdate   Self;
    typedef  itk::Command            Superclass;
    typedef  itk::SmartPointer<Self> Pointer;
    itkNewMacro(Self);
protected:
    CommandProgressUpdate()
    {
    };
public:
    void Execute(itk::Object *caller, const itk::EventObject & event)
    {
        Execute((const itk::Object *)caller, event);
    }
    void Execute(const itk::Object * object, const itk::EventObject & event)
    {
        const itk::ProcessObject * filter = dynamic_cast<const itk::ProcessObject *>(object);
        if (!itk::ProgressEvent().CheckEvent(&event)) {
            return;
        }
        this->dialog->setValue(static_cast<int>(filter->GetProgress() * 100));
    }
    void setProgressDialog(QProgressDialog* dialog)
    {
        this->dialog = dialog;
    }
private:
    QProgressDialog* dialog;
};

bool CT_Image::loadDicomFromDirectory(QString path, QProgressDialog* dialog)
{
    isNifti = false;
    // load meta data
    this->path = path;
    using ImageType = itk::Image<PixelType, 3>;
    using ImageIOType = itk::GDCMImageIO;
    auto dicomIO = ImageIOType::New();

    // define file names loader
    using NamesGeneratorType = itk::GDCMSeriesFileNames;
    auto nameGenerator = NamesGeneratorType::New();
    nameGenerator->SetInputDirectory(this->path.toStdString().c_str());
    nameGenerator->SetGlobalWarningDisplay(false);
    using FileNamesContainer = std::vector<std::string>;
    FileNamesContainer fileNames = nameGenerator->GetInputFileNames();
    mFileNames = fileNames;

    // check if the directory has DICOM files
    try {
        using SeriesIdContainer = std::vector<std::string>;
        const SeriesIdContainer& seriesUID = nameGenerator->GetSeriesUIDs();
        auto seriesItr = seriesUID.begin();
        auto seriesEnd = seriesUID.end();
        if (seriesItr == seriesEnd) {
            std::cout << "No Dicom Files Found!" << endl;
            this->load_succeed = false;
            return false;
        }
    }
    catch (const itk::ExceptionObject & ex) {
        // if no DICOM file is found
        // terminate the read process and set the load_succeed to false
        this->load_succeed = false;
        std::cout << ex << std::endl;
        return false;
    }

    // create a reader for inputs
    using ReaderType = itk::ImageSeriesReader<ImageType>;
    auto reader = ReaderType::New();
    reader->SetImageIO(dicomIO);
    reader->SetFileNames(fileNames);
    reader->ForceOrthogonalDirectionOff();
    CommandProgressUpdate::Pointer observer = CommandProgressUpdate::New();
    observer->setProgressDialog(dialog);
    reader->AddObserver(itk::ProgressEvent(), observer);
    try {
        reader->Update();
    }
    catch (const itk::ExceptionObject & ex) {
        std::cout << ex << std::endl;
        this->load_succeed = false;
        return false;
    }

    using ImageCalculatorFilterType = itk::MinimumMaximumImageCalculator<ImageType>;
    ImageCalculatorFilterType::Pointer imageCalculatorFilter = ImageCalculatorFilterType::New();
    imageCalculatorFilter->SetImage(reader->GetOutput());
    imageCalculatorFilter->Compute();
    this->min_pixel_val = imageCalculatorFilter->GetMinimum();
    this->max_pixel_val = imageCalculatorFilter->GetMaximum();

    // create the dictionary of DICOM meta tags
    createMetaInfo(dicomIO);

    // convert itk data to vtk data
    using FilterType = itk::ImageToVTKImageFilter<ImageType>;
    auto filter = FilterType::New();
    filter->SetInput(reader->GetOutput());
    try {
        filter->Update();
    }
    catch (const itk::ExceptionObject & error) {
        std::cerr << "Error: " << error << std::endl;
        this->load_succeed = false;
        return false;
    }
    mImageItk = reader->GetOutput();
    // flip the image since ITK and VTK has different coordinate system
    vtkNew<vtkImageFlip> imageFlip;
    imageFlip->SetInputData(filter->GetOutput());
    imageFlip->SetFilteredAxes(1);
    imageFlip->Update();

    vtkNew<vtkImageFlip> imageFlip1;
    imageFlip1->SetInputConnection(imageFlip->GetOutputPort());
    imageFlip1->SetFilteredAxes(0);
    imageFlip1->Update();

    // convert scalar type
    vtkNew<vtkImageCast> imageCast;
    imageCast->SetInputConnection(imageFlip1->GetOutputPort());
    imageCast->SetOutputScalarTypeToShort();
    imageCast->Update();

    this->ctImage = imageCast->GetOutput();

    this->load_succeed = true;
    return true;
}

bool CT_Image::loadNiftiFromFilePath(QString path, QProgressDialog* dialog)
{
    isNifti = 1;
    typedef itk::NiftiImageIO   ImageIOType;//GDCMImageIO读DICOM
    ImageIOType::Pointer NiftiImageIO = ImageIOType::New();
    const int Dimension = 3;
    typedef itk::Image<PixelType, Dimension> ImageType;
    itk::ImageFileReader<ImageType>::Pointer reader = itk::ImageFileReader<ImageType>::New();
    reader->SetFileName(path.toStdString());
    reader->SetImageIO(ImageIOType::New());
    try
    {
        reader->Update();
    }
    catch (const itk::ExceptionObject& ex) {
        std::cout << ex << std::endl;
        this->load_succeed = false;
        return false;
    }
    createMetaInfo(NiftiImageIO);

    this->path = QFileInfo(path).path();
    this->fileName = QFileInfo(path).fileName();
    //mFileNames = fileNames;
    using FilterType = itk::ImageToVTKImageFilter<ImageType>;
    auto filter = FilterType::New();
    filter->SetInput(reader->GetOutput());
    try {
        filter->Update();
    }
    catch (const itk::ExceptionObject& error) {
        std::cerr << "Error: " << error << std::endl;
        this->load_succeed = false;
        return false;
    }
    mImageItk = reader->GetOutput();
    writeSeries("./out", reader->GetOutput(), NiftiImageIO->GetMetaDataDictionary(), "jpg");
    //ImageType::DirectionType dir=reader->GetOutput()->GetDirection();
    //for (int i = 0; i < 3; ++i) {
    //    std::cout << dir[i][0] << " " << dir[i][1] << " " << dir[i][2] << std::endl;
    //}
    // flip the image since ITK and VTK has different coordinate system
    vtkNew<vtkImageFlip> imageFlip;
    imageFlip->SetInputData(filter->GetOutput());
    imageFlip->SetFilteredAxes(1);
    imageFlip->Update();

    vtkNew<vtkImageFlip> imageFlip1;
    imageFlip1->SetInputConnection(imageFlip->GetOutputPort());
    imageFlip1->SetFilteredAxes(0);
    imageFlip1->Update();

    // convert scalar type
    vtkNew<vtkImageCast> imageCast;
    imageCast->SetInputConnection(imageFlip1->GetOutputPort());
    imageCast->SetOutputScalarTypeToShort();
    imageCast->Update();

    this->ctImage = imageCast->GetOutput();

    this->load_succeed = true;
    return true;
}

vtkSmartPointer<vtkImageData> CT_Image::getCTImageDataVtk()
{
    return this->ctImage;
}

QMap<QString, QString> CT_Image::getMetaInfo()
{
    return this->metaInfo;
}

bool CT_Image::checkLoadSuccess()
{
    return this->load_succeed;
}

void CT_Image::createMetaInfo(itk::SmartPointer<itk::GDCMImageIO> dicomIO)
{
    QMap<QString, QString> metaInfo;
    using DictionaryType = itk::MetaDataDictionary;
    const DictionaryType & dictionary = dicomIO->GetMetaDataDictionary();
    using MetaDataStringType = itk::MetaDataObject<std::string>;

    auto itr = dictionary.Begin();
    auto end = dictionary.End();

    std::string entryId[19] = {"0010|0010", "0008|0012", "0008|0032", "0008|0060", "0008|0080",
        "0008|0070", "0008|1030", "0008|103e", "0008|1050", "0008|1060",
        "0008|1070", "0008|1090", "0010|0020", "0010|0030", "0010|0040",
        "0010|1010", "0018|0050", "0018|0060", "0018|0088"};

    std::string entryName[19] = {"Patient Name", "Instance Creation Date", "Acquisition Time", "Modality", "Institution Name",
        "Manufacturer", "Study Description", "Series Description", "Performing Physician's Name", "Name of Physician(s) Reading Study",
        "Operators' Name", "Manufacturer's Model Name", "Patient ID", "Patient's Birth Date", "Patient's Sex",
        "Patient's Age", "Slice Thickness", "KVP", "Spacing Between Slices"};

    for (int i = 0; i < 19; i++) {
        auto tagItr = dictionary.Find(entryId[i]);
        if (tagItr != end) {
            MetaDataStringType::ConstPointer entryvalue = dynamic_cast<const MetaDataStringType *>(tagItr->second.GetPointer());
            if (entryvalue) {
                std::string tagvalue = entryvalue->GetMetaDataObjectValue();
                metaInfo[QString::fromStdString(entryName[i])] = QString::fromStdString(tagvalue);
            }
        }
    }
    this->metaInfo = metaInfo;
}

void CT_Image::createMetaInfo(itk::SmartPointer<itk::NiftiImageIO> dicomIO)
{
    QMap<QString, QString> metaInfo;
    using DictionaryType = itk::MetaDataDictionary;
    const DictionaryType& dictionary = dicomIO->GetMetaDataDictionary();
    using MetaDataStringType = itk::MetaDataObject<std::string>;

    auto itr = dictionary.Begin();
    auto end = dictionary.End();

    std::string entryId[19] = { "0010|0010", "0008|0012", "0008|0032", "0008|0060", "0008|0080",
        "0008|0070", "0008|1030", "0008|103e", "0008|1050", "0008|1060",
        "0008|1070", "0008|1090", "0010|0020", "0010|0030", "0010|0040",
        "0010|1010", "0018|0050", "0018|0060", "0018|0088" };

    std::string entryName[19] = { "Patient Name", "Instance Creation Date", "Acquisition Time", "Modality", "Institution Name",
        "Manufacturer", "Study Description", "Series Description", "Performing Physician's Name", "Name of Physician(s) Reading Study",
        "Operators' Name", "Manufacturer's Model Name", "Patient ID", "Patient's Birth Date", "Patient's Sex",
        "Patient's Age", "Slice Thickness", "KVP", "Spacing Between Slices" };

    for (int i = 0; i < 19; i++) {
        auto tagItr = dictionary.Find(entryId[i]);
        if (tagItr != end) {
            MetaDataStringType::ConstPointer entryvalue = dynamic_cast<const MetaDataStringType*>(tagItr->second.GetPointer());
            if (entryvalue) {
                std::string tagvalue = entryvalue->GetMetaDataObjectValue();
                metaInfo[QString::fromStdString(entryName[i])] = QString::fromStdString(tagvalue);
            }
        }
    }
    this->metaInfo = metaInfo;
}

void CT_Image::writeSeries(QString path, itk::Image<PixelType, 3>::Pointer image, itk::MetaDataDictionary dict, QString suffix)
{
    std::string outpath = path.toStdString();
    itksys::SystemTools::RemoveADirectory(outpath);
    itksys::SystemTools::MakeDirectory(outpath);

    typedef itk::Image<PixelType, 3> ImageType;
    typedef itk::Image<unsigned char, 3> CharImageType;
    using IntensityWindowingImageFilterType = itk::IntensityWindowingImageFilter<ImageType, ImageType>;
    IntensityWindowingImageFilterType::Pointer intensityFilter = IntensityWindowingImageFilterType::New();
    intensityFilter->SetInput(image);
    intensityFilter->SetWindowMinimum(0);
    intensityFilter->SetWindowMaximum(4096);
    intensityFilter->SetOutputMinimum(0);
    intensityFilter->SetOutputMaximum(255);
    intensityFilter->Update();
    //dcm转JPG
    using RescaleFilterType = itk::RescaleIntensityImageFilter<ImageType, ImageType>;
    RescaleFilterType::Pointer rescaler = RescaleFilterType::New();
    rescaler->SetOutputMinimum(0);
    rescaler->SetOutputMaximum(255);
    rescaler->SetInput(intensityFilter->GetOutput());
    rescaler->Update();

    using ShiftScaleFilterType = itk::ShiftScaleImageFilter<ImageType, ImageType>;
    ShiftScaleFilterType::Pointer shiftFilter = ShiftScaleFilterType::New();
    shiftFilter->SetInput(rescaler->GetOutput());
    //改变参数调节显示灰度 
    shiftFilter->SetScale(3);
    shiftFilter->Update();

    using ThresholdFilterType = itk::ThresholdImageFilter<ImageType>;
    ThresholdFilterType::Pointer thresholdfilter = ThresholdFilterType::New();
    thresholdfilter->SetOutsideValue(255);
    thresholdfilter->ThresholdAbove(255);//上变下不变
    thresholdfilter->SetInput(shiftFilter->GetOutput());
    thresholdfilter->Update();

    using ImageCastType = itk::CastImageFilter<ImageType, CharImageType>;
    ImageCastType::Pointer imageCast = ImageCastType::New();
    imageCast->SetInput(thresholdfilter->GetOutput());
    imageCast->Update();

    //设置输出图像格式
    constexpr unsigned int OutputDimension = 2;
    using Image2DType = itk::Image<unsigned char, OutputDimension>;
    using SeriesWriterType = itk::ImageSeriesWriter<CharImageType, Image2DType>;
    SeriesWriterType::Pointer seriesWriter = SeriesWriterType::New();

    //生成输出序列文件名
    try
    {
	    
    }
    catch (...)
    {
    }
    typedef itk::NumericSeriesFileNames OutputGeneratorType;
    OutputGeneratorType::Pointer outputGenerator = OutputGeneratorType::New();
    ImageType::RegionType region = imageCast->GetOutput()->GetLargestPossibleRegion();
    ImageType::IndexType start = region.GetIndex();
    ImageType::SizeType  outputSize = region.GetSize();
    std::string format = outpath;
    format += "/image%03d."+ suffix.toStdString();
    outputGenerator->SetSeriesFormat(format.c_str());
    outputGenerator->SetStartIndex(start[2]);
    outputGenerator->SetEndIndex(start[2] + outputSize[2] - 1);
    outputGenerator->SetIncrementIndex(1);
    seriesWriter->SetFileNames(outputGenerator->GetFileNames());
    seriesWriter->SetInput(imageCast->GetOutput());
    if(suffix=="dcm")
    {
        itk::GDCMImageIO::Pointer gdcmIO = itk::GDCMImageIO::New();
        seriesWriter->SetImageIO(gdcmIO);
    }
    else if(suffix=="jpg")
    {
        itk::JPEGImageIO::Pointer jpegIO = itk::JPEGImageIO::New();
        seriesWriter->SetImageIO(jpegIO);
    }
    seriesWriter->SetMetaDataDictionary(dict);
    try
    {
        seriesWriter->Update();
    }
    catch (const itk::ExceptionObject& excp)
    {
        std::cerr << "Exception thrown while writing the series " << std::endl;
        std::cerr << excp << std::endl;
        return;
    }
}

QString CT_Image::getFilePath()
{
    return this->path;
}

QString CT_Image::getFileName()
{
    return this->fileName;
}

void CT_Image::saveImageData(QString diretoryPath, QProgressDialog* dialog)
{
    dialog->setValue(0);
    dialog->setLabelText("Exporting Image...");

    using PixelType = signed short;
    constexpr unsigned int Dimension = 3;
    using ImageType = itk::Image<PixelType, Dimension>;
    
    // convert VTK image to ITK image
    // before conversion, flip the image
    vtkNew<vtkImageFlip> imageFlip;
    imageFlip->SetInputData(this->ctImage);
    imageFlip->SetFilteredAxes(0);
    imageFlip->Update();

    vtkNew<vtkImageFlip> imageFlip1;
    imageFlip1->SetInputConnection(imageFlip->GetOutputPort());
    imageFlip1->SetFilteredAxes(1);
    imageFlip1->Update();

    // convert to itk image
    using FilterType = itk::VTKImageToImageFilter<ImageType>;
    auto filter = FilterType::New();
    filter->SetInput(imageFlip1->GetOutput());
    try {
        filter->Update();
    }
    catch (const itk::ExceptionObject & error) {
        std::cerr << "Error: " << error << std::endl;
        return;
    }

    using ImageIOType = itk::GDCMImageIO;
    auto dicomIO = ImageIOType::New();
    constexpr unsigned int InputDimension = 3;
    constexpr unsigned int OutputDimension = 2;
    using InputImageType = itk::Image<PixelType, InputDimension>;
    using OutputImageType = itk::Image<PixelType, OutputDimension>;
    using SeriesWriterType = itk::ImageSeriesWriter<InputImageType, OutputImageType>;
    using NamesGeneratorType = itk::GDCMSeriesFileNames;
    using ReaderType = itk::ImageSeriesReader<ImageType>;
    auto nameGenerator = NamesGeneratorType::New();
    nameGenerator->SetInputDirectory(this->path.toStdString().c_str());
    auto reader = ReaderType::New();
    reader->SetImageIO(dicomIO);
    using FileNamesContainer = std::vector<std::string>;
    FileNamesContainer fileNames = nameGenerator->GetInputFileNames();
    reader->SetFileNames(fileNames);
    reader->ForceOrthogonalDirectionOff();
    reader->Update();
    using OutputNamesGeneratorType = itk::NumericSeriesFileNames;
    auto outputNames = OutputNamesGeneratorType::New();
    std::string seriesFormat(diretoryPath.toStdString());
    seriesFormat = seriesFormat + "/" + "IM%d.dcm";
    outputNames->SetSeriesFormat(seriesFormat.c_str());
    outputNames->SetStartIndex(1);
    outputNames->SetEndIndex(fileNames.size());
    
    auto seriesWriter = SeriesWriterType::New();
    seriesWriter->SetInput(filter->GetOutput());
    seriesWriter->SetImageIO(dicomIO);
    seriesWriter->SetFileNames(outputNames->GetFileNames());
    seriesWriter->SetMetaDataDictionaryArray(reader->GetMetaDataDictionaryArray());
    CommandProgressUpdate::Pointer observer = CommandProgressUpdate::New();
    observer->setProgressDialog(dialog);
    seriesWriter->AddObserver(itk::ProgressEvent(), observer);
    try {
        seriesWriter->Update();
    }
    catch (const itk::ExceptionObject & excp) {
        std::cerr << "Exception thrown while writing the series " << std::endl;
        std::cerr << excp << std::endl;
    }
}

std::vector<std::string> CT_Image::getFileNames()
{
    return mFileNames;
}

itk::Image<CT_Image::PixelType, 3>::Pointer CT_Image::getCTImageDataItk()
{
    return mImageItk;
}
