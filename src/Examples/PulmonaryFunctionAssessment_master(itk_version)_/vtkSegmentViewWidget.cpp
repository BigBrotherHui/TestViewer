#include "vtkSegmentViewWidget.h"
#include <vtkSliderWidget.h>
#include <vtkSliderRepresentation.h>
#include "vtkCustomImageViewer2.h"
#include <vtkLookupTable.h>
#include <vtkImageActor.h>
#include <vtkSliderRepresentation2D.h>
#include <vtkImageProperty.h>
#include <vtkProperty2D.h>
#include <vtkTextProperty.h>
#include <vtkImageData.h>
#include <vtkPointPicker.h>
#include <QDebug>
#include <vtkDICOMImageReader.h>
#include <itkVTKImageToImageFilter.h>
#include <itkImage.h>
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryMorphologicalClosingImageFilter.h>
#include <itkImageToVTKImageFilter.h>
#include <vtkImageMapToColors.h>
#include <itkBinaryFillholeImageFilter.h>
#include <itkSubtractImageFilter.h>
#include <vtkMatrix3x3.h>
#include "itkConnectedComponentImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkLabelShapeKeepNObjectsImageFilter.h"
#include <itkRescaleIntensityImageFilter.h>
#include <itkImageFileWriter.h>
#include <itkNiftiImageIO.h>
#include "uStatus.h"
#include <vtkPolyDataMapper.h>
#include <QTimer>
#include <vtkPlaneSource.h>
#include <vtkProperty.h>
#include <vtkCamera.h>
vtkSegmentViewWidget::vtkSegmentViewWidget(QWidget* parent) : QVTKOpenGLNativeWidget(parent)
{
	/*viewer2 = vtkSmartPointer<vtkCustomImageViewer2>::New();
	viewer2->OffScreenRenderingOn();*/
    renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    //renderer = viewer2->GetRenderer();
	renderer = vtkSmartPointer<vtkRenderer>::New();
	renderer->GetActiveCamera()->ParallelProjectionOn();
    renderWindow->AddRenderer(renderer);
    setRenderWindow(renderWindow);
	//viewer2->SetRenderWindow(renderWindow);

	actorForeground = vtkSmartPointer<vtkImageActor>::New();
	renderer->AddActor(actorForeground);
	actorBackground = vtkSmartPointer<vtkImageActor>::New();
	renderer->AddActor(actorBackground);
	actorForeground->GetProperty()->SetDiffuse(0.0);
	actorForeground->SetOpacity(.5);
	actorForeground->SetPickable(false);
	
	style = vtkSmartPointer<LineInteractorStyle>::New();
	//vtkSmartPointer<vtkPointPicker> picker = vtkSmartPointer<vtkPointPicker>::New();
	//picker->SetTolerance(1e-10);
	//GetInteractor()->SetPicker(picker);
	GetInteractor()->SetInteractorStyle(style);


	vtkSmartPointer<vtkPlaneSource> plane = vtkSmartPointer<vtkPlaneSource>::New();
	plane->SetCenter(0, 0, 0);
	plane->SetNormal(0.0, 0, 1);
	plane->Update();
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(plane->GetOutputPort());
	actorBrush = vtkSmartPointer<vtkActor>::New();
	style->actorBrush = actorBrush;
	actorBrush->SetMapper(mapper);
	actorBrush->GetProperty()->SetColor(1.0, 0, 0);
	double* space = style->connectedThresholdResult->GetSpacing();
	actorBrush->SetPickable(false);
	actorBrush->GetProperty()->SetRepresentationToWireframe();
	actorBrush->GetProperty()->SetEdgeColor(1.0, 1.0, 0.0);
	actorBrush->GetProperty()->SetEdgeVisibility(true);
	actorBrush->GetProperty()->SetLineWidth(2.0);
	actorBrush->GetProperty()->SetRenderLinesAsTubes(true);
	actorBrush->SetVisibility(0);
	renderer->AddActor(actorBrush);
}

vtkSegmentViewWidget::~vtkSegmentViewWidget()
{

}

void vtkSegmentViewWidget::showImage(vtkSmartPointer<vtkImageData> backimage)
{
	style->setVtkImage(backimage);
	style->foregroundImageActor = actorForeground;
	style->backgroundImageActor = actorBackground;
	actorBackground->SetInputData(backimage);

	//vtkCamera* camera = renderer->GetActiveCamera();
	renderer->ResetCamera();
	renderWindow->Render();
	//viewer2->Render();
	/*vtkCamera* camera = renderer->GetActiveCamera();
	renderer->ResetCameraClippingRange();
	renderWindow->Render();*/
}

void vtkSegmentViewWidget::setConnectedThresholdMinMax(int min, int max)
{
	style->min = min;
	style->max = max;
}

void vtkSegmentViewWidget::FillHole()
{
	fillHole(style->connectedThresholdResult);
}

void vtkSegmentViewWidget::ExtractLargestRegion()
{
	extractLargestRegion(style->connectedThresholdResult);
}

void vtkSegmentViewWidget::fillHole(vtkSmartPointer<vtkImageData> image)
{
	typedef LineInteractorStyle::ImageType ImageType;
	itk::VTKImageToImageFilter<ImageType>::Pointer filter = itk::VTKImageToImageFilter<ImageType>::New();
	filter->SetInput(image);
	try
	{
		filter->Update();
	}
	catch (itk::ExceptionObject &e)
	{
		std::cout << "to itk image failed:" << e.what() << std::endl;
		return;
	}
	
	ImageType::Pointer itkimage=filter->GetOutput();
	//������̬ѧ��ȥ��һ���ķβ���С����
	/*typedef itk::BinaryBallStructuringElement<LineInteractorStyle::PixelType, 3> BSEType;
	using BinaryMorphologicalClosingImageFilterType = itk::BinaryMorphologicalClosingImageFilter<ImageType, ImageType, BSEType>;
	BinaryMorphologicalClosingImageFilterType::Pointer bm = BinaryMorphologicalClosingImageFilterType::New();
	BSEType ballStrEle;
	ballStrEle.SetRadius(5);
	ballStrEle.CreateStructuringElement();
	bm->SetInput(itkimage);
	bm->SetKernel(ballStrEle);
	try
	{
		bm->Update();
	}
	catch (itk::ExceptionObject& e)
	{
		std::cout << "BinaryMorphologicalClosingImageFilter failed:" << e.what() << std::endl;
		return;
	}*/
	//typedef typename itk::BinaryFillholeImageFilter<ImageType> FillHoleFilterType;
	//typename FillHoleFilterType::Pointer fillHoleFilter = FillHoleFilterType::New();
	//fillHoleFilter->SetInput(itkimage);
	////fillHoleFilter->SetForegroundValue(1);
	//fillHoleFilter->UpdateLargestPossibleRegion();
	//try
	//{
	//	fillHoleFilter->Update();
	//}
	//catch (itk::ExceptionObject& e)
	//{
	//	std::cout << "fillHoleFilter failed:" << e.what() << std::endl;
	//	return;
	//}
	typedef itk::BinaryBallStructuringElement< LineInteractorStyle::PixelType, 3  > StructuringElementType;
	typedef itk::BinaryDilateImageFilter <
		ImageType,
		ImageType,
		StructuringElementType >  DilateFilterType;
	StructuringElementType  structuringElement;
	structuringElement.SetRadius(1);   //�����СΪ3*3
	structuringElement.CreateStructuringElement();
	DilateFilterType::Pointer binaryDilate = DilateFilterType::New();
	binaryDilate->SetInput(itkimage);
	binaryDilate->SetKernel(structuringElement);
	binaryDilate->SetDilateValue(1);
	binaryDilate->Update();

	StructuringElementType  structuringElement2;
	structuringElement2.SetRadius(1);   //�����СΪ3*3
	structuringElement2.CreateStructuringElement();
	typedef itk::BinaryErodeImageFilter <
		ImageType,
		ImageType,
		StructuringElementType >  ErodeFilterType;
	ErodeFilterType::Pointer binaryErode = ErodeFilterType::New();
	binaryErode->SetInput(binaryDilate->GetOutput());
	binaryErode->SetKernel(structuringElement2);
	binaryErode->SetErodeValue(1);
	binaryErode->Update();

	itk::ImageToVTKImageFilter<ImageType>::Pointer ff = itk::ImageToVTKImageFilter<ImageType>::New();
	ff->SetInput(binaryErode->GetOutput());
	try
	{
		ff->Update();
	}
	catch (itk::ExceptionObject& e)
	{
		std::cout << "to vtk image failed:" << e.what() << std::endl;
		return;
	}

	vtkSmartPointer<vtkLookupTable> pColorTable = vtkSmartPointer<vtkLookupTable>::New();
	pColorTable->SetNumberOfColors(2);
	pColorTable->SetTableRange(0, 1);
	pColorTable->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
	pColorTable->SetTableValue(1, 0, 1, 0, 1);
	pColorTable->Build();

	vtkSmartPointer<vtkImageMapToColors> colorMap =
		vtkSmartPointer<vtkImageMapToColors>::New();
	colorMap->SetInputData(ff->GetOutput());
	colorMap->SetLookupTable(pColorTable);
	try
	{
		colorMap->Update();
	}
	catch (itk::ExceptionObject& e)
	{
		std::cout << "colorMap failed:" << e.what() << std::endl;
		return;
	}
	qDebug() << "fillHole ...";
	//MRI need reverse z(1->-1)
	vtkMatrix3x3* vmt = colorMap->GetOutput()->GetDirectionMatrix();
	if(uStatus::g_ctimage.isNifti)
		vmt->SetElement(1, 1, -1);
	actorForeground->SetInputData(colorMap->GetOutput());
	style->connectedThresholdResult->DeepCopy(ff->GetOutput());
	renderWindow->Render();
	qDebug() << "fillHole finished";
}

void vtkSegmentViewWidget::extractLargestRegion(vtkSmartPointer<vtkImageData> image)
{
	typedef LineInteractorStyle::ImageType ImageType;
	itk::VTKImageToImageFilter<ImageType>::Pointer filter = itk::VTKImageToImageFilter<ImageType>::New();
	filter->SetInput(image);
	try
	{
		filter->Update();
	}
	catch (itk::ExceptionObject& e)
	{
		std::cout << "to itk image failed:" << e.what() << std::endl;
		return;
	}

	ImageType::Pointer itkimage = filter->GetOutput();

	using ConnectedComponentFilter = itk::ConnectedComponentImageFilter<ImageType, ImageType>;
	auto connnectedComponentFilter = ConnectedComponentFilter::New();
	connnectedComponentFilter->SetInput(itkimage);
	connnectedComponentFilter->Update();

	using LabelShapeKeepNObjectsFilter = itk::LabelShapeKeepNObjectsImageFilter<ImageType>;
	auto labelShapeKeepNObjectsFilter = LabelShapeKeepNObjectsFilter::New();
	labelShapeKeepNObjectsFilter->SetInput(connnectedComponentFilter->GetOutput());
	labelShapeKeepNObjectsFilter->SetBackgroundValue(0);
	labelShapeKeepNObjectsFilter->SetNumberOfObjects(1);
	labelShapeKeepNObjectsFilter->SetAttribute(
		LabelShapeKeepNObjectsFilter::LabelObjectType::NUMBER_OF_PIXELS);
	labelShapeKeepNObjectsFilter->Update();

	typedef itk::RescaleIntensityImageFilter<ImageType, ImageType> RescaleFilterType;
	RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
	rescaleFilter->SetOutputMinimum(0);
	rescaleFilter->SetOutputMaximum(1);
	rescaleFilter->SetInput(labelShapeKeepNObjectsFilter->GetOutput());
	rescaleFilter->Update();

	ImageType::Pointer img = rescaleFilter->GetOutput();
	//auto size = img->GetLargestPossibleRegion().GetSize();
	//for (int i = 0; i < size[2]; ++i) {
	//	for (int j = 0; j < size[1]; ++j) {
	//		for (int k = 0; k < size[0]; ++k) {
	//			ImageType::IndexType index;
	//			index[0] = k;
	//			index[1] = j;
	//			index[2] = i;
	//			signed short v = img->GetPixel(index);
	//			if (v!= 0) {
	//				img->SetPixel(index, -32768);
	//			}
	//			else {
	//				img->SetPixel(index, 1);
	//			}
	//		}
	//	}
	//}
	//img->Modified();

	//const char* out_path = "1111output.nii"; // ������������·��
	//using WriterFilter = itk::ImageFileWriter<ImageType>;
	//auto writer = WriterFilter::New();
	//itk::NiftiImageIO::Pointer io = itk::NiftiImageIO::New();
	//writer->SetFileName(out_path);
	//writer->SetImageIO(io);
	//writer->SetInput(img);
	//writer->Update();

	itk::ImageToVTKImageFilter<ImageType>::Pointer ff = itk::ImageToVTKImageFilter<ImageType>::New();
	ff->SetInput(img);
	try
	{
		ff->Update();
	}
	catch (itk::ExceptionObject& e)
	{
		std::cout << "to vtk image failed:" << e.what() << std::endl;
		return;
	}

	vtkSmartPointer<vtkLookupTable> pColorTable = vtkSmartPointer<vtkLookupTable>::New();
	pColorTable->SetNumberOfColors(2);
	pColorTable->SetTableRange(0, 1);
	pColorTable->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
	pColorTable->SetTableValue(1, 0, 1, 0, 1);
	pColorTable->Build();

	vtkSmartPointer<vtkImageMapToColors> colorMap =
		vtkSmartPointer<vtkImageMapToColors>::New();
	colorMap->SetInputData(ff->GetOutput());
	colorMap->SetLookupTable(pColorTable);
	try
	{
		colorMap->Update();
	}
	catch (itk::ExceptionObject& e)
	{
		std::cout << "colorMap failed:" << e.what() << std::endl;
		return;
	}
	//MRI need reverse z(1->-1)
	vtkMatrix3x3* vmt = colorMap->GetOutput()->GetDirectionMatrix();
	if(uStatus::g_ctimage.isNifti)
		vmt->SetElement(1, 1, -1);
	actorForeground->SetInputData(colorMap->GetOutput());
	style->connectedThresholdResult->DeepCopy(ff->GetOutput());
	renderWindow->Render();
}

vtkSmartPointer<vtkImageData> vtkSegmentViewWidget::getForegroundImageData()
{
	return style->connectedThresholdResult;
}

void vtkSegmentViewWidget::addBrush()
{
	//if (actorBrush)
	//	return;
	style->openBrush = 1;
	actorBrush->SetVisibility(1);
	renderWindow->Render();
	style->mm = !style->mm;
}
