#include "mainwindow.h"
#include <QApplication>
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);
#include <vtkOutputWindow.h>
int main(int argc, char *argv[])
{
    vtkOutputWindow::GlobalWarningDisplayOff();
    QApplication a(argc, argv);
    MainWindow w;
    w.setAttribute(Qt::WA_DeleteOnClose);
    w.show();

    return a.exec();
}
//typedef short PixelType;
//const unsigned int   Dimension = 3;
//typedef itk::Image< PixelType, Dimension > Input2dImageType;
//typedef itk::NiftiImageIO   ImageIOType;//GDCMImageIO¶ÁDICOM
//ImageIOType::Pointer gdcmImageIO = ImageIOType::New();
//typedef itk::ImageFileReader< Input2dImageType > ReaderType2d;
//ReaderType2d::Pointer reader2d = ReaderType2d::New();
//vtkSmartPointer<vtkImageViewer2> viewer = vtkSmartPointer<vtkImageViewer2>::New();
//

//

//

//
//class vtkImageInteractionCallback : public vtkCommand
//{
//public:
//	static vtkImageInteractionCallback* New()
//	{
//		return new vtkImageInteractionCallback;
//	}
//	vtkImageInteractionCallback()
//	{
//		this->Slicing = 0;
//		this->ImageReslice = nullptr;
//		this->Interactor = nullptr;
//	}
//
//	void SetImageReslice(vtkImageReslice* reslice)
//	{
//		this->ImageReslice = reslice;
//	}
//
//	void SetInteractor(vtkRenderWindowInteractor* interactor)
//	{
//		this->Interactor = interactor;
//	}
//
//	void SetRenderWindow(vtkRenderWindow* window)
//	{
//		this->RenderWindow = window;
//	}
//
//	void Execute(vtkObject* caller, unsigned long eventId, void* callData) override
//	{
//		int lastPos[2], curPos[2];
//		this->Interactor->GetLastEventPosition(lastPos);
//		this->Interactor->GetEventPosition(curPos);
//
//		if (eventId == vtkCommand::LeftButtonPressEvent)
//		{
//			this->Slicing = 1;
//			vtkInteractorStyle* style = vtkInteractorStyle::SafeDownCast(
//				this->Interactor->GetInteractorStyle());
//			if (style)
//				style->OnLeftButtonDown();
//		}
//		else if (eventId == vtkCommand::LeftButtonReleaseEvent)
//		{
//			this->Slicing = 0;
//			vtkInteractorStyle* style = vtkInteractorStyle::SafeDownCast(
//				this->Interactor->GetInteractorStyle());
//			if (style)
//				style->OnLeftButtonUp();
//		}
//		else if (eventId == vtkCommand::MouseMoveEvent)
//		{
//			if (this->Slicing)
//			{
//				int deltaY = lastPos[1] - curPos[1];
//				this->ImageReslice->Update();
//				double spacing = this->ImageReslice->GetOutput()->GetSpacing()[2];
//				vtkMatrix4x4* matrix = this->ImageReslice->GetResliceAxes();
//				double point[4], center[4];
//				point[0] = 0.0;
//				point[1] = 0.0;
//				point[2] = spacing * deltaY;
//				point[3] = 1.0;
//
//				matrix->MultiplyPoint(point, center);
//				matrix->SetElement(0, 3, center[0]);
//				matrix->SetElement(1, 3, center[1]);
//				matrix->SetElement(2, 3, center[2]);
//
//				//viewer->SetInputData(ImageReslice->GetOutput());
//				
//				this->Interactor->Render();
//			}
//			else
//			{
//				vtkInteractorStyle* style = vtkInteractorStyle::SafeDownCast(
//					this->Interactor->GetInteractorStyle());
//				if (style)
//					style->OnMouseMove();
//			}
//		}
//	}
//	vtkImageViewer2* viewer;
//
//private:
//	int Slicing;
//	vtkImageReslice* ImageReslice;
//	vtkImageMapToColors* MapToColors;
//	vtkRenderWindowInteractor* Interactor;
//	vtkRenderWindow* RenderWindow;
//};
//

//void showImage(Input2dImageType::Pointer itkImage)
//{
//	vtkSmartPointer<vtkImageData> img = itkToVtk(itkImage);
//	viewer->SetInputData(img);
//	viewer->SetSliceOrientationToXY();
//	viewer->Render();
//	viewer->GetRenderer()->SetBackground(0.5, 0.5, 0.5);
//
//	vtkSmartPointer<LineInteractorStyle> style = vtkSmartPointer<LineInteractorStyle>::New();
//	style->itkImage = itkImage;
//	vtkSmartPointer<vtkRenderWindowInteractor> rwi =
//		vtkSmartPointer<vtkRenderWindowInteractor>::New();
//	rwi->SetPicker(vtkPointPicker::New());
//
//	viewer->SetupInteractor(rwi);
//	viewer->Render();
//
//	int extent[6];
//	double spacing[3];
//	double origin[3];
//	double center[3];
//
//	img->GetExtent(extent);
//	img->GetSpacing(spacing);
//	img->GetOrigin(origin);
//
//	center[0] = origin[0] + spacing[0] * 0.5 * (extent[0] + extent[1]);
//	center[1] = origin[1] + spacing[1] * 0.5 * (extent[2] + extent[3]);
//	center[2] = origin[2] + spacing[2] * 0.5 * (extent[4] + extent[5]);
//
//	double x[3] = { 1, 0, 0 };
//	double y[3] = { 0, 1, 0 };
//	double z[3] = { 0, 0, 1 };
//	auto ImageReslice = vtkSmartPointer<vtkImageReslice>::New();
//	ImageReslice->SetInputData(img);
//	ImageReslice->SetOutputDimensionality(2);
//	ImageReslice->SetResliceAxesDirectionCosines(x, y, z);
//	ImageReslice->SetResliceAxesOrigin(center);
//	ImageReslice->SetInterpolationModeToLinear();
//
//	rwi->SetRenderWindow(viewer->GetRenderWindow());
//
//	auto callback = vtkSmartPointer<vtkImageInteractionCallback>::New();
//	callback->SetImageReslice(ImageReslice);
//	callback->SetInteractor(rwi);
//	callback->SetRenderWindow(viewer->GetRenderWindow());
//	callback->viewer = viewer;
//	style->AddObserver(vtkCommand::LeftButtonPressEvent, callback);
//	style->AddObserver(vtkCommand::LeftButtonReleaseEvent, callback);
//	style->AddObserver(vtkCommand::MouseMoveEvent, callback);
//	vtkSmartPointer<vtkSliderRepresentation2D> sliderRep = vtkSmartPointer<vtkSliderRepresentation2D>::New();
//	sliderRep->SetMinimumValue(viewer->GetSliceMin());
//	sliderRep->SetMaximumValue(viewer->GetSliceMax());
//	sliderRep->SetValue(5.0);
//	sliderRep->GetSliderProperty()->SetColor(1, 0, 0);//red
//	sliderRep->GetTitleProperty()->SetColor(1, 0, 0);//red
//	sliderRep->GetLabelProperty()->SetColor(1, 0, 0);//red
//	sliderRep->GetSelectedProperty()->SetColor(0, 1, 0);//green
//	sliderRep->GetTubeProperty()->SetColor(1, 1, 0);//yellow
//	sliderRep->GetCapProperty()->SetColor(1, 1, 0);//yellow
//	sliderRep->GetPoint1Coordinate()->SetCoordinateSystemToDisplay();
//	sliderRep->GetPoint1Coordinate()->SetValue(40, 40);
//	sliderRep->GetPoint2Coordinate()->SetCoordinateSystemToDisplay();
//	sliderRep->GetPoint2Coordinate()->SetValue(500, 40);
//	vtkSmartPointer<vtkSliderWidget> sliderWidget = vtkSmartPointer<vtkSliderWidget>::New();
//	sliderWidget->SetInteractor(rwi);
//	sliderWidget->SetRepresentation(sliderRep);
//	sliderWidget->SetAnimationModeToAnimate();
//	sliderWidget->EnabledOn();
//	vtkSmartPointer<vtkSliderCallback1> cb = vtkSmartPointer<vtkSliderCallback1>::New();
//	cb->viewer = viewer;
//	sliderWidget->AddObserver(vtkCommand::InteractionEvent, cb);
//
//	//ÉèÖÃ½»»¥ÊôÐÔ
//	rwi->SetInteractorStyle(style);
//
//	rwi->Initialize();
//
//	rwi->Start();
//
//
//}
//
//int main()
//{
//	Input2dImageType::Pointer itkImage = read2dImage("D:\\image\\lung.nii");
//	showImage(itkImage);
//	return 0;
//}





