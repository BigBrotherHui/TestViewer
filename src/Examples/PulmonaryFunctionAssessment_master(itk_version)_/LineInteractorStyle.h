#pragma once
#include <vtkInteractorStyleImage.h>
#include <itkImage.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include "vtkCustomImageViewer2.h"
#include <vtkPointPicker.h>
#include <vtkCellPicker.h>

class LineInteractorStyle : public vtkInteractorStyleImage
{
public:
	typedef short PixelType;
	static const unsigned int Dimension = 3;
	typedef itk::Image< PixelType, Dimension> ImageType;
	static LineInteractorStyle* New();
	vtkTypeMacro(LineInteractorStyle, vtkInteractorStyleImage);
	vtkSmartPointer<vtkImageData> itkToVtk(ImageType::Pointer image);
	ImageType::Pointer VtkToItk(vtkSmartPointer<vtkImageData>);

	ImageType::Pointer connectedThreshold(int range[2], ImageType::Pointer itkImage, ImageType::IndexType seed);
	LineInteractorStyle();
	~LineInteractorStyle();
	virtual void OnLeftButtonDown();
	virtual void OnRightButtonDown();
	virtual void OnMouseMove() override;
	void setVtkImage(vtkSmartPointer<vtkImageData> image);
	ImageType::Pointer itkImage = nullptr;
	vtkSmartPointer<vtkImageData> vtkImage{nullptr};
	vtkSmartPointer<vtkImageActor> foregroundImageActor{nullptr};
	vtkSmartPointer<vtkImageActor> backgroundImageActor{ nullptr };
	int min{ 30 }, max{3000};
	vtkSmartPointer<vtkImageData> connectedThresholdResult;
	void MoveSliceForward();

	void MoveSliceBackward();
	vtkSmartPointer<vtkActor> actorBrush;
	bool mm{ false };
	bool openBrush{ false };
protected:
	void setSlice(int slice);
	virtual void OnKeyDown();

	virtual void OnMouseWheelForward();

	virtual void OnMouseWheelBackward();
private:
	vtkSmartPointer<vtkImageMapToWindowLevelColors> WindowLevelFore;
	vtkSmartPointer<vtkImageMapToWindowLevelColors> WindowLevelBack;
	int SliceOrientation = 2;
	int Slice = 1;
	int SliceMin{ 0 }, SliceMax{ 1 };
	vtkSmartPointer<vtkCellPicker> picker;
	vtkSmartPointer<vtkCellPicker> picker2;
};
