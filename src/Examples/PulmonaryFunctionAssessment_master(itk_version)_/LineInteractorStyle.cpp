
#include "LineInteractorStyle.h"
#include <vtkImageActor.h>
#include <vtkImageMapToColors.h>
#include <vtkLookupTable.h>
#include <vtkImageCast.h>
#include <vtkImageShiftScale.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCamera.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <itkImageToVTKImageFilter.h>
#include <itkVTKImageToImageFilter.h>
#include <vtkRendererCollection.h>
#include <vtkPointPicker.h>
#include <vtkImageData.h>
#include <itkConnectedThresholdImageFilter.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkImageMapper3D.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkInformation.h>
#include <vtkTransform.h>
#include <vtkImageFlip.h>
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryDilateImageFilter.h>
#include <itkSubtractImageFilter.h>
#include <itkBinaryFillholeImageFilter.h>
#include <itkBinaryErodeImageFilter.h>
#include <vtkMatrix3x3.h>
#include "uStatus.h"
#include <QDebug>
#include <vtkPolyDataMapper.h>
#include <vtkPolyData.h>
#include <vtkPlaneSource.h>
#include <vtkSphereSource.h>
#include <vtkRegularPolygonSource.h>
#include <vtkProperty.h>
vtkStandardNewMacro(LineInteractorStyle);

vtkSmartPointer<vtkImageData> LineInteractorStyle::itkToVtk(ImageType::Pointer image)
{
	typedef itk::ImageToVTKImageFilter<ImageType> itkTovtkFilterType;
	itkTovtkFilterType::Pointer itkTovtkImageFilter = itkTovtkFilterType::New();
	itkTovtkImageFilter->SetInput(image);//����ͼ�����ݴ�ITKת��VTK
	itkTovtkImageFilter->Update();

	return itkTovtkImageFilter->GetOutput();
}

itk::Image<short, 3>::Pointer LineInteractorStyle::VtkToItk(vtkSmartPointer<vtkImageData> image)
{
	typedef itk::VTKImageToImageFilter<ImageType> itkTovtkFilterType;
	itkTovtkFilterType::Pointer itkTovtkImageFilter = itkTovtkFilterType::New();
	itkTovtkImageFilter->SetInput(image);
	itkTovtkImageFilter->Update();

	return itkTovtkImageFilter->GetOutput();
}

itk::Image<short,3>::Pointer LineInteractorStyle::connectedThreshold(int range[2], ImageType::Pointer itkImage, ImageType::IndexType seed)
{
	typedef itk::ConnectedThresholdImageFilter< ImageType, ImageType > ConnectedFilterType;
	ConnectedFilterType::Pointer connectedThres = ConnectedFilterType::New();
	connectedThres->SetInput(itkImage);
	connectedThres->SetLower(range[0]);
	connectedThres->SetUpper(range[1]);
	connectedThres->SetReplaceValue(1);
	connectedThres->AddSeed(seed);
	try
	{
		connectedThres->Update();
	}
	catch (itk::ExceptionObject &e)
	{
		std::cout << "connectedThreshold exception:" << e.what() << std::endl;
		return nullptr;
	}

	return connectedThres->GetOutput();
}

LineInteractorStyle::LineInteractorStyle()
{
	connectedThresholdResult = vtkSmartPointer<vtkImageData>::New();
	picker = vtkSmartPointer<vtkCellPicker>::New();
	picker2 = vtkSmartPointer<vtkCellPicker>::New();
	picker->SetTolerance(0.001);
	picker2->SetTolerance(0.001);
}

LineInteractorStyle::~LineInteractorStyle()
{
}
void LineInteractorStyle::OnLeftButtonDown()
{
	if (this->Interactor->GetControlKey()) {
		int* pixelPos = this->Interactor->GetEventPosition();
		picker->Pick(
			pixelPos[0], pixelPos[1], 0,
			this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()
		);
		double picked[3];
		picker->GetPickPosition(picked);

		auto mImageData = vtkImage;
		double ijk[3];
		if (/*pointPicker->GetPointId() != -1 &&*/ nullptr != mImageData)
		{
			//double* point = mImageData->GetPoint(pointPicker->GetPointId());
			mImageData->TransformPhysicalPointToContinuousIndex(picked, ijk);
			//std::cout << "ijk:" << ijk[0] << " " << ijk[1] << " " << ijk[2] << std::endl;
			int* dim = mImageData->GetDimensions();
			bool isWidthInBound = ijk[0] > 4 && ijk[0] < dim[0] - 4;
			bool isHeightInBound = ijk[1] > 4 && ijk[1] < dim[1] - 4;
			if (!isWidthInBound || !isHeightInBound)
				return;
		}else
		{
			std::cout << "--------point id == -1" << std::endl;
			return;
		}

		//int range[2] = { 226,3071 };
		int range[2] = { min,max };
		ImageType::IndexType pixelIndex;
		pixelIndex[0] = static_cast<int>(ijk[0]);
		pixelIndex[1] = static_cast<int>(ijk[1]);
		pixelIndex[2] = static_cast<int>(ijk[2]);
		ImageType::Pointer image1 = connectedThreshold(range, itkImage, pixelIndex);
		connectedThresholdResult->DeepCopy(itkToVtk(image1));

		//short v = *(short*)mImageData->GetScalarPointer(ijk[0], ijk[1], ijk[2]);
		//std::cout << v << std::endl;
		
		//vtkSmartPointer<vtkImageCast> imgCast =
		//	vtkSmartPointer<vtkImageCast>::New();
		//imgCast->SetInputData(image2);
		//imgCast->SetOutputScalarTypeToUnsignedChar();
		//imgCast->Update();
		//vtkSmartPointer<vtkImageShiftScale> ShiftScale =
		//	vtkSmartPointer<vtkImageShiftScale>::New();
		//ShiftScale->SetInputData(imgCast->GetOutput());
		//ShiftScale->SetOutputScalarTypeToUnsignedChar();
		//ShiftScale->SetShift(0);
		//ShiftScale->SetScale(255);
		//ShiftScale->Update();

		vtkSmartPointer<vtkLookupTable> pColorTable = vtkSmartPointer<vtkLookupTable>::New();
		pColorTable->SetNumberOfColors(2);
		pColorTable->SetTableRange(0, 1);
		pColorTable->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
		//static bool flag = true;
		//if (flag)
		pColorTable->SetTableValue(1, 0, 1, 0, 1);
		/*else
			pColorTable->SetTableValue(1, 0, 0, 1, 1.0);*/
		//flag = !flag;
		pColorTable->Build();
		
		vtkSmartPointer<vtkImageMapToColors> colorMap =
			vtkSmartPointer<vtkImageMapToColors>::New();
		colorMap->SetInputData(connectedThresholdResult);
		colorMap->SetLookupTable(pColorTable);
		colorMap->Update();
		//MRI need reverse z(1->-1)
		vtkMatrix3x3* vmt = colorMap->GetOutput()->GetDirectionMatrix();
		if(uStatus::g_ctimage.isNifti)
			vmt->SetElement(1, 1, -1);
		if(foregroundImageActor)
		{
			foregroundImageActor->SetInputData(colorMap->GetOutput());
			this->Interactor->Render();
		}
	}
	else {
		vtkInteractorStyleImage::OnLeftButtonDown();
	}
}

void LineInteractorStyle::OnRightButtonDown()
{
	vtkInteractorStyleImage::OnRightButtonDown();
}

void LineInteractorStyle::OnMouseMove()
{
	if (!openBrush)
		return vtkInteractorStyleImage::OnMouseMove();
	auto mImageData = vtkImage;
	vtkRenderer* renderer = this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
	if(!mImageData)
		return vtkInteractorStyleImage::OnMouseMove();
	int* pixelPos = this->Interactor->GetEventPosition();
	vtkCamera* camera = renderer->GetActiveCamera();
	renderer->SetWorldPoint(camera->GetFocalPoint());
	renderer->WorldToDisplay();
	double cameraDisp[3];
	renderer->GetDisplayPoint(cameraDisp);
	picker2->Pick(pixelPos[0], pixelPos[1], cameraDisp[2],renderer);
	picker->Pick(pixelPos[0], pixelPos[1], cameraDisp[2],renderer);
	double picked[3],picked2[3];
	picker->GetPickPosition(picked);
	picker2->GetPickPosition(picked2);
	double ijk[3];
	if (picker->GetPointId() != -1 && nullptr != mImageData)
	{
		mImageData->TransformPhysicalPointToContinuousIndex(picked, ijk);
		int *dim=mImageData->GetDimensions();
		int size = 10;
		for (int i = 0; i < 2*size+1; ++i)
		{
			for (int j = 0; j < 2 * size + 1; ++j)
			{
				auto img = foregroundImageActor->GetInput();
				if (!img)
					return;
				int m = ijk[0] - size + j;
				int n = ijk[1] - size + i;
				if(m<0 || m>dim[0]-1 || n<0 || n>dim[1]-1)
					continue;
				short* pixel = (short *)(img->GetScalarPointer(ijk[0] - size + j, ijk[1] - size + i, ijk[2]));
				if(mm)
				{
					pixel[0] = -256;
					pixel[1] = -256;
					pixel[2] = -256;
				}
				else
				{
					pixel[0] = 0;
					pixel[1] = 0;
					pixel[2] = 0;
				}
				
			}
		}
		foregroundImageActor->GetInput()->Modified();
		connectedThresholdResult->Modified();
		double* space = mImageData->GetSpacing();
		//vtkSmartPointer<vtkPlaneSource> plane = vtkSmartPointer<vtkPlaneSource>::New();
		//double *origin=mImageData->GetOrigin();
		//plane->SetCenter(picked2[0], picked2[1], 0);
		//plane->SetNormal(0.0, 0, 1);
		//plane->Update();

		//plane->SetPoint1(picked2[0]+5*space[0], picked2[1]-5*space[1],0);
		//plane->SetPoint2(picked2[0] - 5 * space[0], picked2[1] + 5 * space[1], 0);
		//plane->SetOrigin(picked2[0] + 5 * space[0], picked2[1] + 5 * space[1], 0);
		//plane->Update();
		//static_cast<vtkPolyDataMapper*>(actorBrush->GetMapper())->SetInputData(plane->GetOutput());
		//vtkCamera* camera = renderer->GetActiveCamera();
		//qDebug() << "pos:"<<camera->GetPosition()[0] << camera->GetPosition()[1] << camera->GetPosition()[2];
		//qDebug() << "up:"<<camera->GetViewUp()[0] << camera->GetViewUp()[1] << camera->GetViewUp()[2];
		//qDebug() << "focal:"<<camera->GetFocalPoint()[0] << camera->GetFocalPoint()[1] << camera->GetFocalPoint()[2];
		actorBrush->SetVisibility(1);
		actorBrush->SetScale((size*2+1) * space[0], (size*2+1) * space[1], 1);
		double* origin = foregroundImageActor->GetInput()->GetOrigin();
		picked[0] = picked[0] - origin[0];
		picked[1] = picked[1] - origin[1];

		int sliceX = picked[0] / space[0];
		double deltX = picked[0] - sliceX * space[0];
		if (deltX > space[0] / 2.0) sliceX++;

		int sliceY = picked[1] / space[1];
		double deltY = picked[1] - sliceY * space[1];
		if (deltY > space[1] / 2.0)	sliceY++;
		double posX = sliceX * space[0] + origin[0];
		double posY = sliceY * space[1] + origin[1];
		actorBrush->SetPosition(posX, posY, foregroundImageActor->GetPosition()[2]+0.001);

		this->Interactor->Render();
	}
	vtkInteractorStyleImage::OnMouseMove();
}

void LineInteractorStyle::setVtkImage(vtkSmartPointer<vtkImageData> image)
{
	vtkImage = image;
	this->WindowLevelFore = vtkSmartPointer<vtkImageMapToWindowLevelColors>::New();
	this->WindowLevelBack = vtkSmartPointer<vtkImageMapToWindowLevelColors>::New();
	WindowLevelFore->SetInputData(vtkImage);
	WindowLevelBack->SetInputData(vtkImage);
	itkImage = VtkToItk(image);
}

void LineInteractorStyle::MoveSliceForward()
{
	++Slice;
	if (Slice >= SliceMax)
		Slice = SliceMax;
	setSlice(Slice);
	this->Interactor->Render();
}

void LineInteractorStyle::MoveSliceBackward()
{
	--Slice;
	if (Slice <= 0)
		Slice = 0;
	setSlice(Slice);
	this->Interactor->Render();
}

void LineInteractorStyle::setSlice(int slice)
{
	if (!foregroundImageActor||!backgroundImageActor)
		return;
	if(foregroundImageActor->GetInput())
	{
		vtkAlgorithm* input = WindowLevelFore->GetInputAlgorithm();
		input->UpdateInformation();
		vtkInformation* outInfo = input->GetOutputInformation(0);
		int* w_ext = outInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT());
		int slice_min = w_ext[this->SliceOrientation * 2];
		int slice_max = w_ext[this->SliceOrientation * 2 + 1];
		if (this->Slice < slice_min || this->Slice > slice_max)
		{
			this->Slice = static_cast<int>((slice_min + slice_max) * 0.5);
		}
		this->foregroundImageActor->SetDisplayExtent(
			w_ext[0], w_ext[1], w_ext[2], w_ext[3], this->Slice, this->Slice);
	}
	if(backgroundImageActor->GetInput())
	{
		vtkAlgorithm* input = WindowLevelBack->GetInputAlgorithm();
		input->UpdateInformation();
		vtkInformation* outInfo = input->GetOutputInformation(0);
		int* w_ext = outInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT());
		int slice_min = w_ext[this->SliceOrientation * 2];
		int slice_max = w_ext[this->SliceOrientation * 2 + 1];
		this->SliceMin = slice_min;
		this->SliceMax = slice_max;
		if (this->Slice < slice_min || this->Slice > slice_max)
		{
			this->Slice = static_cast<int>((slice_min + slice_max) * 0.5);
		}
		this->backgroundImageActor->SetDisplayExtent(
			w_ext[0], w_ext[1], w_ext[2], w_ext[3], this->Slice, this->Slice);
	}
}

void LineInteractorStyle::OnKeyDown()
{
	std::string key = this->GetInteractor()->GetKeySym();
	if (key.compare("Up") == 0)
	{
		MoveSliceForward();
	}
	else if (key.compare("Down") == 0)
	{
		MoveSliceBackward();
	}
	vtkInteractorStyleImage::OnKeyDown();
}

void LineInteractorStyle::OnMouseWheelForward()
{
	MoveSliceForward();
}

void LineInteractorStyle::OnMouseWheelBackward()
{
	MoveSliceBackward();
}
