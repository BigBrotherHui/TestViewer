#include "VolumeRenderer.h"

// Vtk class
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkDICOMImageReader.h>
#include <vtkImageFlip.h>
#include <vtkCallbackCommand.h>
static void PropertyCallback(vtkObject* caller, long unsigned int eventId,
	void* clientData, void* callData);


void PropertyCallback(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData)
{
	//std::cout << "PropertyCallback"<<std::endl;
	QVTKOpenGLNativeWidget* wp = (QVTKOpenGLNativeWidget*)(clientData);
	wp->GetRenderWindow()->Render();
}
VolumeRenderer::VolumeRenderer(QWidget *parent)
	: QMainWindow(parent), ui(new Ui::VolumeRenderer)
{
	ui->setupUi(this);

	// add VTK widgets
	ui->verticalLayout->addWidget(&widget);
	ui->verticalLayout_2->addWidget(&volumePropertywidget);

	// set up interactor
	interactor = /*vtkSmartPointer<vtkRenderWindowInteractor>::New()*/widget.interactor();
	//interactor->SetRenderWindow(widget.GetRenderWindow());

	// allow the user to interactively manipulate (rotate, pan, etc.) the camera, the viewpoint of the scene.
	auto style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
	interactor->SetInteractorStyle(style);
}

void VolumeRenderer::on_actionOpenImage_triggered()
{
	// show file dialog. change filename only when the new filename is not empty.
	QString filter("Meta image file (*.mhd *.mha)");
	QString filename_backup = filename;
	filename_backup = QFileDialog::getExistingDirectory(this, QString(tr("Open a volume data set")), filename_backup);//, filter;
	if (!filename_backup.trimmed().isEmpty())
	{
		filename = filename_backup;
	}
	else
	{
		return;
	}

	this->SetInputImage(filename);
}

VolumeRenderer::~VolumeRenderer()
{
	delete ui;
}

void VolumeRenderer::on_action_Exit_triggered()
{
	this->close();
}

void VolumeRenderer::on_action_About_triggered()
{
	QMessageBox msgBox;
	msgBox.setText(QString::fromUtf8("Volume Renderer"));
	msgBox.exec();
}

void VolumeRenderer::SetInputImage(QString inputFileName)
{
	this->filename = inputFileName;

	// show filename on window title
	this->setWindowTitle(QString::fromUtf8("Volume Renderer - ") + filename);

	QByteArray ba = filename.toLocal8Bit();
	const char *filename_str = ba.data();

#if 1
	// read Meta Image (.mhd or .mha) files
	auto reader = vtkSmartPointer<vtkDICOMImageReader>::New();
	reader->SetDirectoryName(filename_str);
	reader->Update();
	vtkSmartPointer<vtkImageFlip> vtkFlipFiter =
		vtkSmartPointer<vtkImageFlip>::New();
	vtkFlipFiter->SetInputData(reader->GetOutput());
	vtkFlipFiter->SetFilteredAxes(2);
	vtkFlipFiter->Update();
#elif 1
	// read a series of raw files in the specified folder
	auto reader = vtkSmartPointer<vtkVolume16Reader>::New();
	reader->SetDataDimensions(512, 512);
	reader->SetImageRange(1, 361);
	reader->SetDataByteOrderToBigEndian();
	reader->SetFilePrefix(filename_str);
	reader->SetFilePattern("%s%d");
	reader->SetDataSpacing(1, 1, 1);
#else
	// read NRRD files
	vtkNew<vtkNrrdReader> reader;
	if (!reader->CanReadFile(filename_str))
	{
		std::cerr << "Reader reports " << filename_str << " cannot be read.";
		exit(EXIT_FAILURE);
	}
	reader->SetFileName(filename_str);
	reader->Update();
#endif
	// scale the volume data to unsigned char (0-255) before passing it to volume mapper
	//auto shiftScale = vtkSmartPointer<vtkImageShiftScale>::New();
	//shiftScale->SetInputConnection(reader->GetOutputPort());
	//shiftScale->SetOutputScalarTypeToUnsignedChar();

	// Create transfer mapping scalar value to opacity.
	auto opacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
	opacityTransferFunction->AddPoint(0.0, 0.0);
	opacityTransferFunction->AddPoint(36.0, 0.125);
	opacityTransferFunction->AddPoint(72.0, 0.25);
	opacityTransferFunction->AddPoint(108.0, 0.375);
	opacityTransferFunction->AddPoint(144.0, 0.5);
	opacityTransferFunction->AddPoint(180.0, 0.625);
	opacityTransferFunction->AddPoint(216.0, 0.75);
	opacityTransferFunction->AddPoint(255.0, 0.875);
	//opacityTransferFunction->AddPoint(vtkFlipFiter->GetOutput()->GetScalarRange()[1], 1);

	// Create transfer mapping scalar value to color.
	auto colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
	colorTransferFunction->AddRGBPoint(0.0, 0.0, 0.0, 0.0);
	colorTransferFunction->AddRGBPoint(36.0, 1.0, 0.0, 0.0);
	colorTransferFunction->AddRGBPoint(72.0, 1.0, 1.0, 0.0);
	colorTransferFunction->AddRGBPoint(108.0, 0.0, 1.0, 0.0);
	colorTransferFunction->AddRGBPoint(144.0, 0.0, 1.0, 1.0);
	colorTransferFunction->AddRGBPoint(180.0, 0.0, 0.0, 1.0);
	colorTransferFunction->AddRGBPoint(216.0, 1.0, 0.0, 1.0);
	colorTransferFunction->AddRGBPoint(255.0, 1.0, 1.0, 1.0);
	//colorTransferFunction->AddRGBPoint(vtkFlipFiter->GetOutput()->GetScalarRange()[1], 1,1,1);
	// set up volume property
	auto volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
	volumeProperty->SetColor(colorTransferFunction);
	volumeProperty->SetScalarOpacity(opacityTransferFunction);
	volumeProperty->ShadeOff();
	volumeProperty->SetInterpolationTypeToLinear();

	// assign volume property to the volume property widget
	volumePropertywidget.setVolumeProperty(volumeProperty);
	vtkSmartPointer<vtkCallbackCommand> propertycallback =
		vtkSmartPointer<vtkCallbackCommand>::New();

	propertycallback->SetCallback(PropertyCallback);
	propertycallback->SetClientData(&this->widget);

	volumeProperty->AddObserver(vtkCommand::AnyEvent, propertycallback);

	volumeProperty->GetRGBTransferFunction()->AddObserver(vtkCommand::AnyEvent, propertycallback);
	// choose a volume mapper according to the checked menu item
	vtkSmartPointer<vtkVolumeMapper> volumeMapper;
	if (ui->action_vtkSlicerGPURayCastVolumeMapper->isChecked())
	{
		volumeMapper = vtkSmartPointer<vtkSmartVolumeMapper>::New();
	} 
	else
	{
		volumeMapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
	}
	volumeMapper->SetInputConnection(vtkFlipFiter->GetOutputPort());
	volumeMapper->SetBlendModeToAdditive();
	// The volume holds the mapper and the property and can be used to position/orient the volume.
	auto volume = vtkSmartPointer<vtkVolume>::New();
	volume->SetMapper(volumeMapper);
	volume->SetProperty(volumeProperty);

	// add the volume into the renderer
	auto renderer = vtkSmartPointer<vtkRenderer>::New();
	renderer->AddVolume(volume);
	renderer->SetBackground(1, 1, 1);

	// clean previous renderers and then add the current renderer
	auto window = widget.GetRenderWindow();
	auto collection = window->GetRenderers();
	auto item = collection->GetNextItem();
	while (item != NULL)
	{
		window->RemoveRenderer(item);
		item = collection->GetNextItem();
	}
	window->AddRenderer(renderer);
	window->Render();

	// initialize the interactor
	interactor->Initialize();
	interactor->Start();
}

void VolumeRenderer::on_action_vtkSlicerGPURayCastVolumeMapper_triggered()
{
	ui->action_vtkGPUVolumeRayCastMapper->setChecked(!ui->action_vtkSlicerGPURayCastVolumeMapper->isChecked());
}

void VolumeRenderer::on_action_vtkGPUVolumeRayCastMapper_triggered()
{
	ui->action_vtkSlicerGPURayCastVolumeMapper->setChecked(!ui->action_vtkGPUVolumeRayCastMapper->isChecked());
}

void VolumeRenderer::on_actionScreenshotViewPane_triggered()
{
	QString fileName = QFileDialog::getSaveFileName(this, "Save Screenshot", "", "Pircture Files (*.png *.jpg)");

	if (fileName.isEmpty())
	{
		return;
	}

	vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter =
		vtkSmartPointer<vtkWindowToImageFilter>::New();
	windowToImageFilter->SetInput(this->widget.GetRenderWindow());
	//windowToImageFilter->SetMagnification(3);
	windowToImageFilter->Update();

	vtkSmartPointer<vtkPNGWriter> writer =
		vtkSmartPointer<vtkPNGWriter>::New();
	writer->SetFileName(fileName.toStdString().c_str());
	writer->SetInputConnection(windowToImageFilter->GetOutputPort());
	writer->Write();
}