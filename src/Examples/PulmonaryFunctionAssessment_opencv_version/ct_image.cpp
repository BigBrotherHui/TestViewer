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
#include <itkExtractImageFilter.h>
#include "uStatus.h"
#include "itkFlipImageFilter.h"
CT_Image::CT_Image()
{
}


CT_Image::~CT_Image()
{
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
std::map<int, float>& CT_Image::getAverageMap() {
    return mAverageMap;
}

template<class TItkImageType>
bool CT_Image::AddItkImage(TItkImageType* input)
{
    mImageDimension = TItkImageType::ImageDimension;
    typedef itk::ImageToVTKImageFilter <TItkImageType> ConverterType;
    typename ConverterType::Pointer converter = ConverterType::New();
    converter->SetInput(input);
    converter->Update();
    vtkSmartPointer<vtkImageData> img = vtkSmartPointer<vtkImageData>::New();
    img->DeepCopy(converter->GetOutput());
    mVtkImages.push_back(img);

    // Account for direction in transform. The offset is already accounted for
    // in the VTK image coordinates, no need to put it in the transform.
    vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
    matrix->Identity();
    for (unsigned int i = 0; i < input->GetImageDimension(); i++) {
        for (unsigned int j = 0; j < input->GetImageDimension(); j++) {
            matrix->SetElement(i, j, input->GetDirection()[i][j]);
            // Direction is used around the image origin in ITK
            matrix->SetElement(i, 3, matrix->GetElement(i, 3) - matrix->GetElement(i, j) * input->GetOrigin()[j]);
        }
        matrix->SetElement(i, 3, matrix->GetElement(i, 3)+ input->GetOrigin()[i]);
    }

    // GetDirection provides the forward transform, vtkImageReslice wants the inverse
    matrix->Invert();

    mTransform.push_back(vtkSmartPointer<vtkTransform>::New());
    mTransform.back()->SetMatrix(matrix);
    return true;
}

bool CT_Image::loadNiftiFromFilePath(QString path, QProgressDialog* dialog)
{
    typedef itk::NiftiImageIO ImageIOType;
    ImageIOType::Pointer NiftiImageIO = ImageIOType::New();
    itk::ImageFileReader<InputImageType>::Pointer reader = itk::ImageFileReader<InputImageType>::New();
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
    mImage = reader->GetOutput();
    createMetaInfo(NiftiImageIO);
    dialog->setValue(10);
    this->path = path;

    typedef itk::Image< PixelType, 3> OutImageType;
    typedef itk::ExtractImageFilter<InputImageType, OutImageType> ExtractImageFilterType;

    typename InputImageType::RegionType inputRegion = reader->GetOutput()->GetLargestPossibleRegion();
    typename InputImageType::SizeType inputSize = inputRegion.GetSize();
    typename InputImageType::IndexType start = inputRegion.GetIndex();
    typename InputImageType::SizeType extractedRegionSize = inputSize;
    typename InputImageType::RegionType extractedRegion;
    extractedRegionSize[Dimension - 1] = 0;
    extractedRegion.SetSize(extractedRegionSize);
    for (unsigned int i = 0; i < inputSize[Dimension - 1]; i++) {
        start[Dimension - 1] = i;
        extractedRegion.SetIndex(start);

        typename ExtractImageFilterType::Pointer filter = ExtractImageFilterType::New();
        filter->SetExtractionRegion(extractedRegion);
        filter->SetInput(reader->GetOutput());
        filter->SetDirectionCollapseToSubmatrix();
        try
        {
            filter->Update();
        }
        catch (itk::ExceptionObject& e)
        {
            std::cout << "failed to ExtractImage:" <<e.what()<< std::endl;
            return false;
        }
        if (!AddItkImage<OutImageType>(filter->GetOutput()))
           return false;
        if (dialog->value() < 100)
            dialog->setValue(dialog->value()+1);
    }

    //writeSeries("./out", NiftiImageIO->GetMetaDataDictionary());
    if (dialog->value() < 100)
        dialog->setValue(100);
    this->load_succeed = true;
    return true;
}

vtkSmartPointer<vtkImageData> CT_Image::getCTImageDataVtk(int index)
{
    if (index<0 || index > mVtkImages.size() - 1)
        return nullptr;
    else
        return mVtkImages[index];
}

vtkSmartPointer<vtkImageData> CT_Image::getCTImageDataVtk_Registered(int index)
{
    if (index<0 || index > mVtkImages_registered.size() - 1)
        return nullptr;
    else
        return mVtkImages_registered[index];
}

vtkSmartPointer<vtkImageData> CT_Image::getCTImageDataVtk_Segmented(int index)
{
    if (index<0 || index > mVtkImages_segmented.size() - 1)
        return nullptr;
    else
		return mVtkImages_segmented[index];
}

vtkSmartPointer<vtkImageData> CT_Image::getCTImageDataVtk_Segmented_Src(int index)
{
    if (index<0 || index > mVtkImages_segmented_src.size() - 1)
        return nullptr;
    else
        return mVtkImages_segmented_src[index];
}

int CT_Image::getNumberOfImages()
{
    return mVtkImages.size();
}

QMap<QString, QString> CT_Image::getMetaInfo()
{
    return this->metaInfo;
}

bool CT_Image::checkLoadSuccess()
{
    return this->load_succeed;
}

void CT_Image::setRegisteredImages(std::vector<vtkSmartPointer<vtkImageData>> images)
{
    mVtkImages_registered = images;
    mVtkImages_segmented.clear();
    mVtkImages_segmented_src.clear();
    for(int i=0;i<mVtkImages_registered.size();++i)
    {
        vtkSmartPointer<vtkImageData> ig = vtkSmartPointer<vtkImageData>::New();
        ig->DeepCopy(mVtkImages_registered[i]);
        mVtkImages_segmented.push_back(ig);
        vtkSmartPointer<vtkImageData> ig2 = vtkSmartPointer<vtkImageData>::New();
        ig2->DeepCopy(mVtkImages_registered[i]);
        mVtkImages_segmented_src.push_back(ig2);
    }
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

void CT_Image::writeSeries(QString path, itk::MetaDataDictionary dict)
{
    std::string outpath = path.toStdString();
    itksys::SystemTools::RemoveADirectory(outpath);
    itksys::SystemTools::MakeDirectory(outpath);

    typedef itk::Image<unsigned char, Dimension> CharImageType;
    using IntensityWindowingImageFilterType = itk::IntensityWindowingImageFilter<InputImageType, InputImageType>;
    IntensityWindowingImageFilterType::Pointer intensityFilter = IntensityWindowingImageFilterType::New();
    intensityFilter->SetInput(mImage);
    intensityFilter->SetWindowMinimum(0);
    intensityFilter->SetWindowMaximum(4096);
    intensityFilter->SetOutputMinimum(0);
    intensityFilter->SetOutputMaximum(255);
    intensityFilter->Update();

    using Filp = itk::FlipImageFilter<InputImageType>;
    Filp::Pointer flip = Filp::New();
    flip->SetInput(intensityFilter->GetOutput());
    Filp::FlipAxesArrayType axis;
    axis[0] = 0;
    axis[1] = 1;
    flip->SetFlipAxes(axis);
    flip->Update();

    using ImageCastType = itk::CastImageFilter<InputImageType, CharImageType>;
    ImageCastType::Pointer imageCast = ImageCastType::New();
    imageCast->SetInput(flip->GetOutput());
    imageCast->Update();

    //设置输出图像格式
    constexpr unsigned int OutputDimension = 2;
    using Image2DType = itk::Image<unsigned char, OutputDimension>;
    using SeriesWriterType = itk::ImageSeriesWriter<CharImageType, Image2DType>;
    SeriesWriterType::Pointer seriesWriter = SeriesWriterType::New();

    typedef itk::NumericSeriesFileNames OutputGeneratorType;
    OutputGeneratorType::Pointer outputGenerator = OutputGeneratorType::New();
    InputImageType::RegionType region = imageCast->GetOutput()->GetLargestPossibleRegion();
    InputImageType::IndexType start = region.GetIndex();
    InputImageType::SizeType  outputSize = region.GetSize();
    std::string format = outpath;
    format += "/image%03d.jpg";
    outputGenerator->SetSeriesFormat(format.c_str());
    outputGenerator->SetStartIndex(start[Dimension-1]);
    outputGenerator->SetEndIndex(start[Dimension - 1] + outputSize[Dimension-1] - 1);
    outputGenerator->SetIncrementIndex(1);
    seriesWriter->SetFileNames(outputGenerator->GetFileNames());
    seriesWriter->SetInput(imageCast->GetOutput());
    
    itk::JPEGImageIO::Pointer jpegIO = itk::JPEGImageIO::New();
    seriesWriter->SetImageIO(jpegIO);
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
    imageFlip->SetInputData(nullptr);
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

itk::Image<CT_Image::PixelType, CT_Image::Dimension>::Pointer CT_Image::getCTImageDataItk()
{
    return mImage;
}
