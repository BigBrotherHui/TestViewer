#include "vtkHelper.h"
#include <vtkNew.h>
#include <vtkNamedColors.h>
#include <vtkTextProperty.h>
#include <vtkCornerAnnotation.h>
#include <QString>
#include <itkImage.h>
#include <itkGDCMImageIO.h>
#include <itkImageFileReader.h>
#include <itkIntensityWindowingImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkShiftScaleImageFilter.h>
#include <itkThresholdImageFilter.h>
#include <itkCastImageFilter.h>
#include <itkImageFileWriter.h>
#include <itkJPEGImageIO.h>
#include <QDir>
QString DICOMToJPEG(char* imgpath)
{
	using CharPixelType = unsigned char;
	const unsigned int Dimension = 2;
	using ImageIOType = itk::GDCMImageIO;
	using CharImageType = itk::Image<CharPixelType, Dimension>;
	using IntPixelType = signed int;
	using IntImageType = itk::Image<IntPixelType, Dimension>;

	using ReaderType = itk::ImageFileReader<IntImageType>;

	//获取不带后缀的文件名
	std::string path = imgpath;
	int ps = path.find_last_of("/");
	int pe = path.find_last_of(".");
	std::string name = path.substr(ps + 1, pe - ps - 1);

	//ITK获取DICOM图像
	ImageIOType::Pointer gdcmImageIO = ImageIOType::New();
	ReaderType::Pointer reader = ReaderType::New();
	reader->SetFileName(imgpath);
	reader->SetImageIO(gdcmImageIO);
	try
	{
		reader->Update();
	}
	catch (itk::ExceptionObject& e)
	{
		std::cout << "error:" << e.what() << std::endl;
		return "";
	}
	//像素灰度值反转
	IntImageType::Pointer image = reader->GetOutput();
	/*ImageType::SizeType size = image->GetLargestPossibleRegion().GetSize();
	ImageType::IndexType index;
	for (int x = 0; x < size[0]; x++)
		for (int y = 0; y < size[1]; y++)
		{
			index[0] = x;
			index[1] = y;
			ImageType::PixelType value = image->GetPixel(index);
			unsigned short test = 4096 - value;
			image->SetPixel(index, test);
		}
	image->Update();*/

	//设置窗位和窗宽
	using IntensityWindowingImageFilterType = itk::IntensityWindowingImageFilter <IntImageType, IntImageType>;
	IntensityWindowingImageFilterType::Pointer intensityFilter = IntensityWindowingImageFilterType::New();
	intensityFilter->SetInput(image);
	intensityFilter->SetWindowMinimum(0);
	intensityFilter->SetWindowMaximum(4096);
	intensityFilter->SetOutputMinimum(0);
	intensityFilter->SetOutputMaximum(255);
	intensityFilter->Update();

	//dcm转JPG
	using RescaleFilterType = itk::RescaleIntensityImageFilter<IntImageType, IntImageType>;
	RescaleFilterType::Pointer rescaler = RescaleFilterType::New();
	rescaler->SetOutputMinimum(0);
	rescaler->SetOutputMaximum(255);
	rescaler->SetInput(intensityFilter->GetOutput());
	rescaler->Update();

	using ShiftScaleFilterType = itk::ShiftScaleImageFilter<IntImageType, IntImageType>;
	ShiftScaleFilterType::Pointer shiftFilter = ShiftScaleFilterType::New();
	shiftFilter->SetInput(rescaler->GetOutput());
	//改变参数调节显示灰度 
	shiftFilter->SetScale(3);
	shiftFilter->Update();

	using ThresholdFilterType = itk::ThresholdImageFilter<IntImageType>;
	ThresholdFilterType::Pointer thresholdfilter = ThresholdFilterType::New();
	thresholdfilter->SetOutsideValue(255);
	thresholdfilter->ThresholdAbove(255);//上变下不变
	thresholdfilter->SetInput(shiftFilter->GetOutput());
	thresholdfilter->Update();

	using ImageCastType = itk::CastImageFilter<IntImageType, CharImageType>;
	ImageCastType::Pointer imageCast = ImageCastType::New();
	imageCast->SetInput(thresholdfilter->GetOutput());
	imageCast->Update();

	//将JPG格式的图像存储在本地
	using Writer1Type = itk::ImageFileWriter<CharImageType>;
	Writer1Type::Pointer writer1 = Writer1Type::New();

	QDir().rmdir("./out");
	QDir().mkdir("./out");

	std::string writepath = "./out/" + name + ".jpg";
	using jpgType = itk::JPEGImageIO;
	jpgType::Pointer jpgIO = jpgType::New();
	writer1->SetImageIO(jpgIO);
	writer1->SetFileName(writepath);
	writer1->SetInput(imageCast->GetOutput());
	try
	{
		writer1->Update();
	}
	catch (itk::ExceptionObject& e)
	{
		std::cout << "error write:" << e.what() << std::endl;
		return "";
	}

	QString jpgimgpath = QString::fromStdString(writepath);
	return jpgimgpath;
}

void setHeader(vtkRenderer * ren, int axis)
{
    vtkNew<vtkNamedColors> colors;
    vtkNew<vtkCornerAnnotation> cornerAnnotation;
    cornerAnnotation->GetTextProperty()->SetFontFamilyToArial();
    cornerAnnotation->GetTextProperty()->BoldOn();
    cornerAnnotation->GetTextProperty()->SetFontSize(20);
    cornerAnnotation->GetTextProperty()->SetColor(colors->GetColor3d("Azure").GetData());
    cornerAnnotation->SetLinearFontScaleFactor(2);
    cornerAnnotation->SetNonlinearFontScaleFactor(1);
    cornerAnnotation->SetMaximumFontSize(20);
    switch (axis) {
    case 0:
        cornerAnnotation->SetText(2, "Sagittal");
        break;
    case 1:
        cornerAnnotation->SetText(2, "Coronal");
        break;
    case 2:
        cornerAnnotation->SetText(2, "Axial");
        break;
    case 3:
        cornerAnnotation->SetText(2, "3D View");
        break;
    }
    ren->AddViewProp(cornerAnnotation);
}