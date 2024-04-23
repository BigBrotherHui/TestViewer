#include "widget.h"
#include "ui_widget.h"
#include <QDir>
#include <QDebug>
#include <QFileDialog>
#include <itkGDCMSeriesFileNames.h>
#include <itkImageSeriesReader.h>
#include <itkGDCMImageIO.h>
#include "drr.hpp"

#include <itkImageToVTKImageFilter.h>
#include <vtkImageActor.h>
#include "itkImage.h"
#include "itkImageFileReader.h"

#include <vtkSmartPointer.h>
#include <vtkImageActor.h>

#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkOutputWindow.h>
#include <vtkConeSource.h>
#include <vtkCubeSource.h>
#include <vtkAppendPolyData.h>
#include <vtkPolyDataMapper.h>

#include <vtkMatrix4x4.h>
#include <vtkTransform.h>
#include <itkImageDuplicator.h>
#include <vtkColorTransferFunction.h>
#include <itkPNGImageIO.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkCamera.h>
#include <vtkFrustumSource.h>
#include <vtkPlaneSource.h>
#include <vtkTexture.h>
#include <vtkPiecewiseFunction.h>
#include <vtkNamedColors.h>
#include <vtkPlanes.h>
#include <vtkPoints.h>
#include <vtkLine.h>
#include <itkMetaImageIOFactory.h>
#include <vtkInteractorStyleImage.h>
#include <vtkVolumeProperty.h>
#include <vtkSmartVolumeMapper.h>
#include <QDateTime>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
	
    ui->setupUi(this);
	/*m_DisplayActionEventBroadcast = mitk::DisplayActionEventBroadcast::New();
	m_DisplayActionEventBroadcast->LoadStateMachine("DisplayInteraction.xml");
	m_DisplayActionEventBroadcast->SetEventConfig("DisplayConfigMITKBase.xml");*/
	//m_DisplayActionEventBroadcast->AddEventConfig("DisplayConfigCrosshair.xml");
	/*m_DisplayActionEventHandler = std::make_unique<mitk::DisplayActionEventHandlerStd>();
	m_DisplayActionEventHandler->SetObservableBroadcast(m_DisplayActionEventBroadcast);
	m_DisplayActionEventHandler->InitActions();*/
	//vtkOutputWindow::GlobalWarningDisplayOff();
	m_renderwindow = new QVTKOpenGLNativeWidget(this);
    ui->gridLayout->addWidget(m_renderwindow,1, 0, 1,1);
	m_renderwindow->setRenderWindow(rw3d);
	rw3d->AddRenderer(r3d);
	m_renderwindow2dside = new QVTKOpenGLNativeWidget(this);
	m_renderwindow2dside->setRenderWindow(rwside);
	rwside->AddRenderer(rside);
	rside->AddActor(actorside);
        ui->gridLayout->addWidget(m_renderwindow2dside, 1, 2, 1, 1);
	m_renderwindow2dside->interactor()->SetInteractorStyle(vtkInteractorStyleImage::New());
	m_renderwindow2dfront = new QVTKOpenGLNativeWidget(this);
	m_renderwindow2dfront->setRenderWindow(rwfront);
	rwfront->AddRenderer(rfront);
	rfront->AddActor(actorfront);
        ui->gridLayout->addWidget(m_renderwindow2dfront, 1, 1, 1, 1);
	m_renderwindow2dfront->interactor()->SetInteractorStyle(vtkInteractorStyleImage::New());
	//m_data_storage_ = mitk::StandaloneDataStorage::New();
	//m_data_storage_2dside= mitk::StandaloneDataStorage::New();
	//m_data_storage_2dfront = mitk::StandaloneDataStorage::New();
	//m_renderwindow->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);
	//m_renderwindow->GetRenderer()->GetSliceNavigationController()->SetDefaultViewDirection(mitk::AnatomicalPlane::Coronal);
	//m_renderwindow->GetRenderer()->SetDataStorage(m_data_storage_);
	//m_renderwindow2dside->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard2D);
	//m_renderwindow2dside->GetSliceNavigationController()->SetDefaultViewDirection(mitk::AnatomicalPlane::Axial);
	//m_renderwindow2dside->GetRenderer()->SetDataStorage(m_data_storage_2dside);
	//m_renderwindow2dfront->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard2D);
	//m_renderwindow2dfront->GetSliceNavigationController()->SetDefaultViewDirection(mitk::AnatomicalPlane::Axial);
	//m_renderwindow2dfront->GetRenderer()->SetDataStorage(m_data_storage_2dfront);
 //   QList<int> sizes;
	//sizes << 900 << 100;
 //   ui->splitter->setSizes(sizes);
 //   QHBoxLayout* l = new QHBoxLayout(ui->widget_window);
 //   l->addWidget(m_renderwindow);
	//m_lw= new QmitkLevelWindowWidget(this);
	//m_lw->SetDataStorage(m_data_storage_);
	////m_lw->GetManager()->LevelWindowChanged.AddListener(mitk::MessageDelegate1<Widget, const mitk::LevelWindow&>(this, &Widget::levelWindowChanged));
	//l->addWidget(m_lw);
	//l->addWidget(m_renderwindow2dfront);
	////l->addWidget(m_renderwindow2dside);
	//m_renderwindow2dside->setVisible(false);
	//l->setStretchFactor(m_renderwindow, 5);
	//l->setStretchFactor(m_lw, 1);
	//l->setStretchFactor(m_renderwindow2dfront, 5);
	//l->setStretchFactor(m_renderwindow2dside, 5);

	//auto points = vtkSmartPointer<vtkPoints>::New();
	//auto polydata = vtkSmartPointer<vtkPolyData>::New();
	//auto cells = vtkSmartPointer<vtkCellArray>::New();
	//for (int i = 0; i < 10; ++i) {
	//	points->InsertNextPoint(i * 3, i * 3, i * 3);
	//	if (i % 2) {
	//		vtkSmartPointer<vtkLine> line = vtkSmartPointer<vtkLine>::New();
	//		line->GetPointIds()->SetId(0, i - 1);
	//		line->GetPointIds()->SetId(1, i);
	//		cells->InsertNextCell(line);
	//	}
	//}
	//polydata->SetPoints(points);
	//polydata->SetLines(cells);
	//mitk::DataNode::Pointer dd = mitk::DataNode::New();
	//mitk::Surface::Pointer pp = mitk::Surface::New();
	//pp->SetVtkPolyData(polydata);
	//dd->SetData(pp);
	//m_data_storage_->Add(dd);
}

Widget::~Widget()
{
    delete ui;
}

//void Widget::levelWindowChanged(const mitk::LevelWindow& levelWindow)
//{
//	auto subset = m_data_storage_->GetSubset(mitk::NodePredicateDataType::New("Image"));
//	for(auto pNode : *subset)
//	{
//		bool v;
//		pNode->GetBoolProperty("volumerendering", v);
//		if(!v)
//		{
//			continue;
//		}
//		double opacity = 1;
//		auto levelWindow = m_lw->GetManager()->GetLevelWindow();
//		auto level = levelWindow.GetLevel();
//		auto window = levelWindow.GetWindow();
//		auto min = level - window / 2;
//		auto perWindow = window / 8;
//		pNode->SetProperty("volumerendering.usegpu", mitk::BoolProperty::New(1));
//		pNode->SetProperty("volumerendering.usemip", mitk::BoolProperty::New(false));
//		pNode->SetProperty("volumerendering.blendmode", mitk::IntProperty::New(0));  //使用混合形式的投影
//		auto transferFunction = mitk::TransferFunction::New();
//		double scalarOpacities[8] = { 0, 0.001846, 0.024414, 0.100113, 0.467300, 0.711914, 0.915909, 1 };
//		transferFunction->GetScalarOpacityFunction()->RemoveAllPoints();
//		for (int i = 0; i < 8; i++)
//		{
//			auto x = min + i * perWindow;
//			if (i >= 4)
//			{
//				x += perWindow;
//			}
//			auto y = scalarOpacities[i] * opacity;
//			transferFunction->GetScalarOpacityFunction()->AddPoint(x, y);
//		}
//		transferFunction->GetScalarOpacityFunction()->Modified();
//
//		transferFunction->GetGradientOpacityFunction()->RemoveAllPoints();
//		transferFunction->GetGradientOpacityFunction()->AddPoint(0, 1.000000 * opacity);
//		transferFunction->GetGradientOpacityFunction()->Modified();
//
//		transferFunction->GetColorTransferFunction()->RemoveAllPoints();
//		transferFunction->GetColorTransferFunction()->AddRGBPoint(312.382940, 1.000000, 0.564706, 0.274510);
//		transferFunction->GetColorTransferFunction()->AddRGBPoint(455.103448, 1.000000, 0.945098, 0.768627);
//		transferFunction->GetColorTransferFunction()->AddRGBPoint(623.773140, 1.000000, 0.800000, 0.333333);
//		transferFunction->GetColorTransferFunction()->AddRGBPoint(796.767695, 1.000000, 0.901961, 0.815686);
//		transferFunction->GetColorTransferFunction()->AddRGBPoint(930.838475, 1.000000, 1.000000, 1.000000);
//		transferFunction->GetColorTransferFunction()->AddRGBPoint(1073.558984, 1.000000, 0.839216, 0.423529);
//		transferFunction->GetColorTransferFunction()->AddRGBPoint(1220.604356, 1.000000, 0.772549, 0.490196);
//		transferFunction->GetColorTransferFunction()->Modified();
//		pNode->SetProperty("TransferFunction", mitk::TransferFunctionProperty::New(transferFunction.GetPointer()));
//	}
//	mitk::RenderingManager::GetInstance()->RequestUpdate(m_renderwindow->GetVtkRenderWindow());
//}

void Widget::requestUpdateAll()
{
	rw3d->Render();
	rwfront->Render();
	rwside->Render();
}

void Widget::setImage(bool front, itk::Image<unsigned char, 3>::Pointer img)
{
	itk::ImageToVTKImageFilter<itk::Image<unsigned char, 3>>::Pointer cast = itk::ImageToVTKImageFilter<itk::Image<unsigned char, 3>>::New();
	cast->SetInput(img);
	cast->Update();
	vtkImageData *vimg=cast->GetOutput();
	if (front)
		/*m_data_storage_2dfront->GetNamedObject<mitk::Image>("drrfront")->SetImportChannel(img->GetBufferPointer());*/
		actorfront->SetInputData(vimg);
	else
		//m_data_storage_2dside->GetNamedObject<mitk::Image>("drrside")->SetImportChannel(img->GetBufferPointer());
		actorside->SetInputData(vimg);
	vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();	
	transform->PreMultiply();
	
	transform->Translate(isocenter[0], isocenter[1], isocenter[2]);
	transform->RotateX(ui->horizontalSlider_rotate_x->value());
	transform->RotateY(ui->horizontalSlider_rotate_y->value());
	transform->RotateZ(ui->horizontalSlider_rotate_z->value());
	transform->Translate(-isocenter[0], -isocenter[1], -isocenter[2]);
	transform->Translate(ui->horizontalSlider_translate_x->value(), ui->horizontalSlider_translate_y->value(), ui->horizontalSlider_translate_z->value());
	transform->GetMatrix()->Invert();
	cameraActor->SetUserMatrix(transform->GetMatrix());
	/*vtkCamera* camera = m_renderwindow->GetRenderer()->GetVtkRenderer()->GetActiveCamera();
	double* focalpoint = camera->GetFocalPoint();
	double* position = camera->GetPosition();
	double* viewup = camera->GetViewUp();
	mitk::RenderingManager::GetInstance()->InitializeViewByBoundingObjects(m_renderwindow->GetVtkRenderWindow(), m_data_storage_);
	camera->SetPosition(position);
	camera->SetViewUp(viewup);
	camera->SetFocalPoint(focalpoint);
	m_renderwindow->GetRenderer()->GetVtkRenderer()->ResetCameraClippingRange();*/

	static bool flag{ false };
	if (flag)
	{
		typedef itk::FlipImageFilter< itk::Image<unsigned char, 3> > FlipFilterType;
		FlipFilterType::Pointer flipFilter = FlipFilterType::New();
		typedef FlipFilterType::FlipAxesArrayType FlipAxesArrayType;
		FlipAxesArrayType flipArray;
		flipArray[0] = 0;
		flipArray[1] = 1;
		flipFilter->SetFlipAxes(flipArray);
		flipFilter->SetInput(img);
		flipFilter->Update();
		itk::ImageToVTKImageFilter<itk::Image<unsigned char, 3>>::Pointer f = itk::ImageToVTKImageFilter<itk::Image<unsigned char, 3>>::New();
		f->SetInput(flipFilter->GetOutput());
		f->Update();
		m_actor_farplane->GetTexture()->SetInputData(f->GetOutput());
		vtkSmartPointer<vtkMatrix4x4> vmt = vtkSmartPointer<vtkMatrix4x4>::New();
		vmt->DeepCopy(transform->GetMatrix());
		for(int i=0;i<3;++i)
		{
			vmt->SetElement(i, 3, vmt->GetElement(i, 3) + vmt->GetElement(i, 1) * 200);
		}
		m_actor_farplane->SetUserMatrix(vmt);

		//vtkCamera* camera = m_renderwindow->GetRenderer()->GetVtkRenderer()->GetActiveCamera();
		//double planesArray[24];
		//// 获取当前视锥体的6个平面
		//camera->GetFrustumPlanes(1.0, planesArray);

		//vtkSmartPointer<vtkPlanes> planes =
		//	vtkSmartPointer<vtkPlanes>::New();
		//planes->SetFrustumPlanes(planesArray);

		//// 创建视锥体
		//vtkSmartPointer<vtkFrustumSource> frustumSource =
		//	vtkSmartPointer<vtkFrustumSource>::New();
		//frustumSource->ShowLinesOff();
		//frustumSource->SetPlanes(planes);
		//frustumSource->Update();
		//static_cast<vtkPolyDataMapper *>(actorfrustum->GetMapper())->SetInputData(frustumSource->GetOutput());
		//TODO:初始不要先移动Z轴；移动Z轴后平面纹理不对；绕x、z旋转时旋转轴不对
		m_renderwindow->renderWindow()->Render();
		return;
	}
	if(!flag)
	{
		flag = 1;
	}
	vtkSmartPointer<vtkPlaneSource> farplane = vtkSmartPointer<vtkPlaneSource>::New();
	Eigen::Vector3d eisocenter{isocenter};
	//eisocenter[1] += 200;
	eisocenter[0] -= 512 / 2;
	eisocenter[2] -= 512 / 2;

	//farplane仅对x、z平移做出反应，对旋转和y轴的平移应当忽略
	farplane->SetOrigin((eisocenter.data()));
	farplane->SetPoint1(Eigen::Vector3d(eisocenter + 512 * Eigen::Vector3d::UnitX()).data());
	farplane->SetPoint2(Eigen::Vector3d(eisocenter + 512 * Eigen::Vector3d::UnitZ()).data());
	farplane->Update();
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputData(farplane->GetOutput());
	m_actor_farplane = vtkSmartPointer<vtkActor>::New();
	m_actor_farplane->SetMapper(mapper);
	vtkSmartPointer<vtkTexture> texture = vtkSmartPointer<vtkTexture>::New();
	typedef itk::FlipImageFilter< itk::Image<unsigned char, 3> > FlipFilterType;
	FlipFilterType::Pointer flipFilter = FlipFilterType::New();
	typedef FlipFilterType::FlipAxesArrayType FlipAxesArrayType;
	FlipAxesArrayType flipArray;
	flipArray[0] = 0;
	flipArray[1] = 1;
	flipFilter->SetFlipAxes(flipArray);
	flipFilter->SetInput(img);
	flipFilter->Update();
	itk::ImageToVTKImageFilter<itk::Image<unsigned char, 3>>::Pointer f = itk::ImageToVTKImageFilter<itk::Image<unsigned char, 3>>::New();
	f->SetInput(flipFilter->GetOutput());
	f->Update();
	texture->SetInputData(f->GetOutput());
	m_actor_farplane->SetTexture(texture);
	r3d->AddActor(m_actor_farplane);

	// 通过该类得到指定物体的颜色
	vtkSmartPointer<vtkNamedColors> colors =
		vtkSmartPointer<vtkNamedColors>::New();

	//vtkCamera* camera = m_renderwindow->GetRenderer()->GetVtkRenderer()->GetActiveCamera();
	//double planesArray[24];
	//// 获取当前视锥体的6个平面
	//camera->GetFrustumPlanes(1.0, planesArray);

	//vtkSmartPointer<vtkPlanes> planes =
	//	vtkSmartPointer<vtkPlanes>::New();
	//planes->SetFrustumPlanes(planesArray);

	//// 创建视锥体
	//vtkSmartPointer<vtkFrustumSource> frustumSource =
	//	vtkSmartPointer<vtkFrustumSource>::New();
	//frustumSource->ShowLinesOff();
	//frustumSource->SetPlanes(planes);
	//frustumSource->Update();
	//vtkSmartPointer<vtkPolyDataMapper> mapperfrustum = vtkSmartPointer<vtkPolyDataMapper>::New();
	//mapperfrustum->SetInputData(frustumSource->GetOutput());
	//actorfrustum = vtkSmartPointer<vtkActor>::New();
	//actorfrustum->SetMapper(mapperfrustum);
	//m_renderwindow->GetRenderer()->GetVtkRenderer()->AddActor(actorfrustum);

	m_renderwindow->renderWindow()->Render();
}

void Widget::on_pushButton_resetView_clicked()
{
	//mitk::RenderingManager::GetInstance()->InitializeViewByBoundingObjects(m_renderwindow->GetVtkRenderWindow(), m_data_storage_);
	r3d->ResetCamera();
}

vtkSmartPointer<vtkPolyData> Widget::transformPolyData(vtkSmartPointer<vtkMatrix4x4> mt, vtkSmartPointer<vtkPolyData> p)
{
	if (!p)
		return nullptr;
	if (!mt)
		mt = vtkSmartPointer<vtkMatrix4x4>::New();
	vtkSmartPointer<vtkPolyData> ret = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
	transform->SetMatrix(mt);
	vtkSmartPointer<vtkTransformPolyDataFilter> fi = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	fi->SetTransform(transform);
	fi->SetInputData(p);
	fi->Update();
	ret->DeepCopy(fi->GetOutput());
	return ret;
}

void Widget::addPoint(double* pt)
{
	/*mitk::DataNode::Pointer dt1 = mitk::DataNode::New();
	dt1->SetProperty("pointsize", mitk::FloatProperty::New(10));
	mitk::PointSet::Pointer ps1 = mitk::PointSet::New();
	dt1->SetData(ps1);
	ps1->SetPoint(0, pt);
	m_data_storage_->Add(dt1);
	mitk::RenderingManager::GetInstance()->InitializeViewByBoundingObjects(m_renderwindow->GetVtkRenderWindow(), m_data_storage_);*/

}

void savedata(const float* rst1, int len, std::string  st)
{
	FILE* fpwrt = NULL;
	const char* file_c = st.c_str();
	fopen_s(&fpwrt, file_c, "wb+");
	if (fpwrt == NULL)
	{
		std::cout << "error write file" << std::endl;
	}
	fwrite(rst1, sizeof(float), len, fpwrt);
	fclose(fpwrt);
}

void Widget::on_pushButton_importImage_clicked()
{
    QString dir = QFileDialog::getOpenFileName(this, "选择图像", "D:/Images/", "*.mhd");
    if (dir.isEmpty())
        return;
	//using SeriesFileNamesType = itk::GDCMSeriesFileNames;
	//using ImageType = itk::Image<float, 3>;
	//using ReaderType = itk::ImageSeriesReader<ImageType>;
	//using ImageIOType = itk::GDCMImageIO;
	//ImageIOType::Pointer m_gdcmImageIO = ImageIOType::New();
	//SeriesFileNamesType::Pointer m_gdcmSeriesFileNames = SeriesFileNamesType::New();
	//m_gdcmSeriesFileNames->SetUseSeriesDetails(true);
	//// m_gdcmSeriesFileNames->AddSeriesRestriction("0008|0021");
	//m_gdcmSeriesFileNames->SetDirectory(dir.toStdString());
	//using SeriesIdContainer = std::vector<std::string>;
	//const SeriesIdContainer& seriesUID = m_gdcmSeriesFileNames->GetSeriesUIDs();
	//const std::string& seriesUIDStr = seriesUID.at(0);
	//using FileNamesContainer = std::vector<std::string>;
	//FileNamesContainer fileNames = m_gdcmSeriesFileNames->GetFileNames(seriesUIDStr);
	//ReaderType::Pointer reader = ReaderType::New();
	//reader->SetImageIO(m_gdcmImageIO);
	//reader->SetFileNames(fileNames);
	//reader->ForceOrthogonalDirectionOff();
	itk::ObjectFactoryBase::RegisterFactory(itk::MetaImageIOFactory::New());
	typedef itk::Image<PixelType, 3> RawImageType;
	typedef itk::ImageFileReader<RawImageType> RawReaderType;
	RawReaderType::Pointer reader = RawReaderType::New();
	reader->SetFileName(dir.toUtf8().data());
	try {
		reader->Update();
		//typedef itk::IntensityWindowingImageFilter <ImageType, ImageType> IntensityWindowingImageFilterType;
		//IntensityWindowingImageFilterType::Pointer intensityFilter = IntensityWindowingImageFilterType::New();
		//intensityFilter->SetInput(reader->GetOutput());
		//intensityFilter->SetWindowLevel(1000, 400);//window,level//180,90
		//intensityFilter->SetOutputMinimum(0);
		//intensityFilter->SetOutputMaximum(255);
		//intensityFilter->Update();
		m_image = reader->GetOutput(); 
//#include "itkImageDuplicator.h"
//		using DuplicatorType = itk::ImageDuplicator<RawImageType>;
//		auto duplicator = DuplicatorType::New();
//		duplicator->SetInputImage(m_image);
//		duplicator->Update();
//		RawImageType::Pointer clonedImage = duplicator->GetOutput();
//		imageinterpolation(clonedImage, clonedImage->GetSpacing()[0]);
//		CT3DNormalization(clonedImage,100);
//		//float or [3]{0,0,0};
//		//clonedImage->SetOrigin(or );
//		float spacing[3];
//		int sz[3];
//		for(int i=0;i<3;++i)
//		{
//			spacing[i] = clonedImage->GetSpacing()[i];
//			sz[i] = clonedImage->GetLargestPossibleRegion().GetSize()[i];
//		}
//		siddon->SetImg3d((float *)clonedImage->GetBufferPointer(), spacing, sz);
//		float spacing2d[2]{ .5,.5 };
//		int imageSize[2]{ 512,512 };
//		siddon->SetImg2dParameter(spacing2d, imageSize);
//		float tmp[12]{ 0 };
//		siddon->SetTransformMatrix(tmp);
//		float *result = new float[imageSize[0]* imageSize[1]] {0.0};
//		float transformMatrixFront[12];
//		MatrixHelper = new GenerateMatrixHelper;
//		MatrixHelper->generateEulerTransform(0,0,0,0,0,0);
//		MatrixHelper->setPixelSpacing(spacing);
//		MatrixHelper->setFocalDistance(/*5042.04*/100);// * .5
//		MatrixHelper->setImageSize(imageSize);
//		float centerOffset[3]{ 0.488283,49.8047,-3.75 };//mask1
//		MatrixHelper->setCenterOffset(centerOffset);
//		MatrixHelper->getTransformMatrix(true, transformMatrixFront);
//
//		siddon->Run(transformMatrixFront,result);
//		int width=512,height=512;
//		float minVal = *std::min_element(result, result + width * height);
//		float maxVal = *std::max_element(result, result + width * height);
//		cv::Mat grayImage(512, 512, CV_16UC1);
//		for (int i = 0; i < width; ++i) {
//			for (int j = 0; j < height; ++j) {
//				grayImage.at<ushort>(i, j) =4095*( static_cast<ushort>(width *i+j)-minVal)/(maxVal-minVal);
//			}
//		}
//		cv::imshow("output", grayImage);
//		delete result;
//		result = nullptr;
		/*delete MatrixHelper;
		MatrixHelper = nullptr;*/
	}
	catch (itk::ExceptionObject& err)
	{
		std::cerr << "ERROR: ExceptionObject caught !" << std::endl;
		std::cerr << err << std::endl;
		return;
	}
	// image construct of mitk
	//mitk::Image::Pointer mitkImage = mitk::Image::New();
	//mitk::GrabItkImageMemory(m_image, mitkImage);
	//mitk::DataNode::Pointer dt = mitk::DataNode::New();
	////mitk::Point3D origin;
	////origin[0] = origin[1] = origin[2] = 0;
	////mitkImage->SetOrigin(origin);
	//dt->SetName("image");
	//dt->SetBoolProperty("volumerendering", 1);
	//dt->SetData(mitkImage);
	//m_data_storage_->Add(dt);
	itk::ImageToVTKImageFilter<itk::Image<PixelType, 3>>::Pointer cast = itk::ImageToVTKImageFilter<itk::Image<PixelType, 3>>::New();
	cast->SetInput(m_image);
	cast->Update();
	vtkImageData* vimg = cast->GetOutput();
	vtkSmartPointer<vtkVolumeProperty> volumeProperty =
		vtkSmartPointer<vtkVolumeProperty>::New();
	volumeProperty->SetInterpolationTypeToLinear();
	volumeProperty->ShadeOn();  //打开或者关闭阴影测试
	volumeProperty->SetAmbient(0.4);
	volumeProperty->SetDiffuse(0.6);  //漫反射
	volumeProperty->SetSpecular(0.2); //镜面反射
	//设置不透明度
	vtkSmartPointer<vtkPiecewiseFunction> compositeOpacity =
		vtkSmartPointer<vtkPiecewiseFunction>::New();
	compositeOpacity->AddPoint(70, 0.00);
	compositeOpacity->AddPoint(90, 0.40);
	compositeOpacity->AddPoint(180, 0.60);
	volumeProperty->SetScalarOpacity(compositeOpacity); //设置不透明度传输函数
	//compositeOpacity->AddPoint(120,  0.00);//测试隐藏部分数据,对比不同的设置
	//compositeOpacity->AddPoint(180,  0.60);
	//volumeProperty->SetScalarOpacity(compositeOpacity);
	//设置梯度不透明属性
	vtkSmartPointer<vtkPiecewiseFunction> volumeGradientOpacity =
		vtkSmartPointer<vtkPiecewiseFunction>::New();
	volumeGradientOpacity->AddPoint(10, 0.0);
	volumeGradientOpacity->AddPoint(90, 0.5);
	volumeGradientOpacity->AddPoint(100, 1.0);
	volumeProperty->SetGradientOpacity(volumeGradientOpacity);//设置梯度不透明度效果对比
	//设置颜色属性
	vtkSmartPointer<vtkColorTransferFunction> color =
		vtkSmartPointer<vtkColorTransferFunction>::New();
	color->AddRGBPoint(0.000, 0.00, 0.00, 0.00);
	color->AddRGBPoint(64.00, 1.00, 0.52, 0.30);
	color->AddRGBPoint(190.0, 1.00, 1.00, 1.00);
	color->AddRGBPoint(220.0, 0.20, 0.20, 0.20);
	volumeProperty->SetColor(color);
	/********************************************************************************/
	vtkSmartPointer<vtkVolume> volume =
		vtkSmartPointer<vtkVolume>::New();
	vtkSmartPointer<vtkSmartVolumeMapper> volumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
	volume->SetMapper(volumeMapper);
        volumeMapper->SetInputData(vimg);
	volume->SetProperty(volumeProperty);
	r3d->AddActor(volume);
	//float scd = 200;
	//drr(reader->GetOutput(), "drr.png", 200);

	//mitk::IOUtil::Load("drr.png", *m_data_storage_2d);
        //rw3d->Render();
	//mitk::RenderingManager::GetInstance()->InitializeViewByBoundingObjects(m_renderwindow->GetVtkRenderWindow(), m_data_storage_);
	/*using PixelType = unsigned char;
	const unsigned int  Dimension = 2;
	typedef itk::Image<PixelType, Dimension> DRRImageType;
	typedef itk::ImageFileReader<DRRImageType> DRRReaderType;*/
	//DRRReaderType::Pointer drrreader = DRRReaderType::New();
	//drrreader->SetFileName("drr.dcm");
	//using ImageIOType = itk::GDCMImageIO;
	//ImageIOType::Pointer gdcmImageIO = ImageIOType::New();
	//drrreader->SetImageIO(gdcmImageIO);
	//try
	//{
	//	drrreader->Update();
	//}
	//catch (itk::ExceptionObject& e)
	//{
	//	std::cerr << "exception in file reader" << std::endl;
	//	std::cerr << e << std::endl;
	//	return;
	//}
	//mitk::Image::Pointer mitkImagedrr = mitk::Image::New();
	//mitk::GrabItkImageMemory(drrreader->GetOutput(), mitkImagedrr);
	//mitk::DataNode::Pointer dtdrr = mitk::DataNode::New();
	//dtdrr->SetName("drr");
	//dtdrr->SetBoolProperty("volumerendering", 1);
	//dtdrr->SetData(mitkImagedrr);
	//m_data_storage_->Add(dtdrr);

	//typedef itk::ImageToVTKImageFilter<DRRImageType> ConnectorType;
	//ConnectorType::Pointer connector = ConnectorType::New();
	//connector->SetInput(drrreader->GetOutput());
	//try
	//{
	//	connector->Update();
	//}
	//catch (itk::ExceptionObject& e)
	//{
	//	std::cerr << "exception in file reader" << std::endl;
	//	std::cerr << e << std::endl;
	//	return;
	//}
	double imOrigin[3]{ 0,0,0 };
	double* imRes = (double*)m_image->GetSpacing().GetDataPointer();
	double imSize[3];
	auto sz = m_image->GetLargestPossibleRegion().GetSize();
	imSize[0] = sz[0];
	imSize[1] = sz[1];
	imSize[2] = sz[2];
	isocenter[0] = imOrigin[0] + imRes[0] * static_cast<double>(imSize[0]) / 2.0;
	isocenter[1] = imOrigin[1] + imRes[1] * static_cast<double>(imSize[1]) / 2.0;
	isocenter[2] = imOrigin[2] + imRes[2] * static_cast<double>(imSize[2]) / 2.0;

	vtkNew<vtkConeSource> camCS;
	camCS->SetHeight(1.6);
	camCS->SetResolution(12);
	camCS->SetRadius(0.4);
	camCS->Update();
	vtkSmartPointer<vtkTransform> t = vtkSmartPointer<vtkTransform>::New();
	t->RotateZ(-90);
	t->Translate(0.8, 0, 0);

	vtkNew<vtkCubeSource> camCBS;
	camCBS->SetXLength(.8);
	camCBS->SetYLength(1.2);
	camCBS->SetZLength(.8);
	camCBS->SetCenter(0, -1.0, 0);
	camCBS->Update();

	vtkSmartPointer<vtkPlaneSource> closeplane= vtkSmartPointer<vtkPlaneSource>::New();
	closeplane->SetCenter(0, 0, 0);
	closeplane->SetNormal(0, 1, 0);
	closeplane->Update();

	vtkNew<vtkAppendPolyData> camAPD;
	camAPD->AddInputData(transformPolyData(t->GetMatrix(),camCS->GetOutput()));
	camAPD->AddInputData(camCBS->GetOutput());
	camAPD->AddInputData(closeplane->GetOutput());
	camAPD->Update();

	/*vtkSmartPointer<vtkFrustumSource> frustumSource =
		vtkSmartPointer<vtkFrustumSource>::New();
	frustumSource->ShowLinesOff();
	frustumSource->SetPlanes();*/


	//mitk::Surface::Pointer sur = mitk::Surface::New();
	vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
	transform->Translate(isocenter[0], isocenter[1], isocenter[2]);
	transform->Scale(50, 50, 50);
	vtkSmartPointer<vtkTransformPolyDataFilter> filter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	filter->SetInputData(camAPD->GetOutput());
	filter->SetTransform(transform);
	filter->Update();
	//sur->SetVtkPolyData(filter->GetOutput());
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	cameraActor->SetMapper(mapper);
	mapper->SetInputData(filter->GetOutput());
	r3d->AddActor(cameraActor);
	/*mitk::DataNode::Pointer surNode = mitk::DataNode::New();
	surNode->SetData(sur);
	surNode->SetName("cam");*/
	//mitk::Gizmo::AddGizmoToNode(surNode, m_data_storage_);
	//m_data_storage_->Add(surNode);
	
	//mitk::RenderingManager::GetInstance()->InitializeViewByBoundingObjects(m_renderwindow->GetVtkRenderWindow(), m_data_storage_);

        r3d->ResetCamera();
        rfront->ResetCamera();
        rside->ResetCamera();
	requestUpdateAll();
}

void Widget::on_horizontalSlider_scd_valueChanged(int v)
{
	auto frontImage = drr(m_image, "", v, ui->horizontalSlider_rotate_x->value(), ui->horizontalSlider_rotate_y->value(), ui->horizontalSlider_rotate_z->value(),
		ui->horizontalSlider_translate_x->value(), ui->horizontalSlider_translate_y->value(), ui->horizontalSlider_translate_z->value());
	setImage(1, frontImage);
	//mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	requestUpdateAll();
}

void Widget::on_horizontalSlider_translate_x_valueChanged(int v)
{
	auto frontImage = drr(m_image, "",  ui->horizontalSlider_scd->value(), ui->horizontalSlider_rotate_x->value(), ui->horizontalSlider_rotate_y->value(), ui->horizontalSlider_rotate_z->value(),
		v,ui->horizontalSlider_translate_y->value(),ui->horizontalSlider_translate_z->value());
	setImage(1, frontImage);
	//mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	requestUpdateAll();
}

void Widget::on_horizontalSlider_translate_y_valueChanged(int v)
{
	auto frontImage = drr(m_image, "", ui->horizontalSlider_scd->value(), ui->horizontalSlider_rotate_x->value(), ui->horizontalSlider_rotate_y->value(), ui->horizontalSlider_rotate_z->value(),
		ui->horizontalSlider_translate_x->value(), v,  ui->horizontalSlider_translate_z->value());
	setImage(1, frontImage);
	//mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	requestUpdateAll();
}

void Widget::on_horizontalSlider_translate_z_valueChanged(int v)
{
    //QDateTime cur = QDateTime::currentDateTime();
	auto frontimage = drr(m_image, "", ui->horizontalSlider_scd->value(), ui->horizontalSlider_rotate_x->value(), ui->horizontalSlider_rotate_y->value(), ui->horizontalSlider_rotate_z->value(),
		ui->horizontalSlider_translate_x->value(), ui->horizontalSlider_translate_y->value(),v );
    //std::cout << "***********"<< QDateTime::currentDateTime().msecsTo(cur)<<std::endl;
	auto sideimage = drr(m_image, "drr.png", v, 0, 0, -90, 0);
	//itk::Image< unsigned char, 3 >::Pointer img=
	//if (!gpu)
	//	gpu = new SiddonGPU;
	//itk::Image<float, 3>::SizeType size = m_image->GetLargestPossibleRegion().GetSize();
	//qDebug() << size[0] << size[1] << size[2];
	//itk::Image<float, 3>::SpacingType spacing = m_image->GetSpacing();
	//qDebug() << spacing[0] << spacing[1] << spacing[2];
	//int _3dpixelNum[3];
	//float _3dpixelSpacing[3];
	//for (int i = 0; i < 3; i++)
	//{
	//	_3dpixelNum[i] = size[i];
	//	_3dpixelSpacing[i] = spacing[0];
	//}

	//gpu->SetImg3d((float *)m_image->GetBufferPointer(), _3dpixelSpacing, _3dpixelNum);

	//typedef itk::Euler3DTransform< double >  TransformType;
	//TransformType::Pointer transform = TransformType::New();
	//transform->SetComputeZYX(true);
	//TransformType::OutputVectorType translation;
	//translation[0] = 0;
	//translation[1] = 0;
	//translation[2] = 1000;
	//// constant for converting degrees into radians
	//const double dtr = (atan(1.0) * 4.0) / 180.0;
	//transform->SetTranslation(translation);
	//transform->SetRotation(dtr * 0, dtr * 0, dtr * -90);
	//itk::Image<short, 3>::PointType  imOrigin = m_image->GetOrigin();
	//itk::Image<short, 3>::SpacingType imRes = m_image->GetSpacing();
	//typedef itk::Image<short, 3>::RegionType     InputImageRegionType;
	//typedef InputImageRegionType::SizeType InputImageSizeType;
	//InputImageRegionType imRegion = m_image->GetBufferedRegion();
	//InputImageSizeType   imSize = imRegion.GetSize();
	//TransformType::InputPointType isocenter;
	//isocenter[0] = imOrigin[0] + imRes[0] * static_cast<double>(imSize[0]) / 2.0;
	//isocenter[1] = imOrigin[1] + imRes[1] * static_cast<double>(imSize[1]) / 2.0;
	//isocenter[2] = imOrigin[2] + imRes[2] * static_cast<double>(imSize[2]) / 2.0;
	//transform->SetCenter(isocenter);
	//TransformType::MatrixType mt = transform->GetMatrix();
	//float transformMatrix[12];
	//for (int i = 0; i < 3; ++i)
	//{
	//	for (int j = 0; j < 4; ++j)
	//		transformMatrix[i * 4 + j] = mt[i][j];
	//}
	//gpu->SetTransformMatrix(transformMatrix);

	//int imageSize[2]{ 512,512 };
	//int ImageSize = 512 * 512;
	//if (!mask)
	//	mask = new float[ImageSize] {1};

	//if (!result)
	//	result = new float[ImageSize] {0.0};

	//float tmp[12]{ 0 };
	//gpu->SetTransformMatrix(tmp);

	//float spacing2d[2]{ .5,.5 };
	//gpu->SetImg2dParameter(spacing2d, imageSize, mask);
	//gpu->Run(transformMatrix, result);



	//m_data_storage_->Remove(m_data_storage_->GetNamedNode("drr"));
	//mitk::IOUtil::Load("drr.png", *m_data_storage_2d);

	/*using PixelType = unsigned char;
	const unsigned int  Dimension = 2;
	typedef itk::Image<PixelType, Dimension> DRRImageType;
	typedef itk::ImageFileReader<DRRImageType> DRRReaderType;
	DRRReaderType::Pointer drrreader = DRRReaderType::New();
	drrreader->SetImageIO(itk::PNGImageIO::New());
	drrreader->SetFileName("drr.png");
	try
	{
		drrreader->Update();
	}
	catch (itk::ExceptionObject& e)
	{
		std::cerr << "exception in file reader" << std::endl;
		std::cerr << e << std::endl;
		return;
	}*/
	/*mitk::DataNode::Pointer frontnode= m_data_storage_2dfront->GetNamedNode("drrfront");
	if(!frontnode)
	{
		frontnode = mitk::DataNode::New();
		frontnode->SetName("drrfront");
		mitk::Image::Pointer img = mitk::Image::New();
		img->InitializeByItk(frontimage.GetPointer());
		img->SetImportChannel(frontimage->GetBufferPointer());
		frontnode->SetData(img);
		m_data_storage_2dfront->Add(frontnode);

		mitk::DataNode::Pointer sidenode = mitk::DataNode::New();
		sidenode->SetName("drrside");
		mitk::Image::Pointer imgside = mitk::Image::New();
		imgside->InitializeByItk(sideimage.GetPointer());
		imgside->SetImportChannel(sideimage->GetBufferPointer());
		sidenode->SetData(imgside);
		m_data_storage_2dside->Add(sidenode);

		mitk::RenderingManager::GetInstance()->InitializeViewByBoundingObjects(m_renderwindow2dfront->renderWindow(), m_data_storage_2dfront);
		mitk::RenderingManager::GetInstance()->InitializeViewByBoundingObjects(m_renderwindow2dside->renderWindow(), m_data_storage_2dside);
	}*/
	/*else
	{*/
		setImage(1, frontimage);
		setImage(0, sideimage);
	//}
		

	//mitk::Surface::Pointer sur =m_data_storage_->GetNamedObject<mitk::Surface>("sur");
	/*vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
	transform->Translate(0, 0, v);
	transform->RotateZ(-90);
	transform->Scale(50, 50, 50);
	sur->GetGeometry()->SetIndexToWorldTransformByVtkMatrix(transform->GetMatrix());*/
	//mitk::RenderingManager::GetInstance()->RequestUpdateAll();
		requestUpdateAll();
}

void Widget::on_horizontalSlider_rotate_x_valueChanged(int v)
{
	auto frontImage = drr(m_image, "", ui->horizontalSlider_scd->value(), v, ui->horizontalSlider_rotate_y->value(), ui->horizontalSlider_rotate_z->value(),ui->horizontalSlider_translate_x->value()
		, ui->horizontalSlider_translate_y->value(), ui->horizontalSlider_translate_z->value());
	setImage(1, frontImage);
	//mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	requestUpdateAll();
}

void Widget::on_horizontalSlider_rotate_y_valueChanged(int v)
{
	auto frontImage = drr(m_image, "", ui->horizontalSlider_scd->value(), ui->horizontalSlider_rotate_x->value(), v,  ui->horizontalSlider_rotate_z->value(), ui->horizontalSlider_translate_x->value()
		, ui->horizontalSlider_translate_y->value(), ui->horizontalSlider_translate_z->value());
	setImage(1, frontImage);
	//mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	requestUpdateAll();
}

void Widget::on_horizontalSlider_rotate_z_valueChanged(int v)
{
	auto frontImage = drr(m_image, "", ui->horizontalSlider_scd->value(), ui->horizontalSlider_rotate_x->value(), ui->horizontalSlider_rotate_y->value(),v , ui->horizontalSlider_translate_x->value()
		, ui->horizontalSlider_translate_y->value(), ui->horizontalSlider_translate_z->value());
	setImage(1, frontImage);
	//mitk::RenderingManager::GetInstance()->RequestUpdateAll();
	requestUpdateAll();
}
