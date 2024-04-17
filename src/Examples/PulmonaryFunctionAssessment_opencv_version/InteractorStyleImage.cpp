#include "InteractorStyleImage.h"
#include <vtkTextMapper.h>
#include "ct_2d_widget.h"
#include "uStatus.h"
#include <vtkPointPicker.h>
#include <vtkRendererCollection.h>
#include <QPainter>
#include <QDebug>
#include <QLabel>
#include <vtkImageProperty.h>
vtkStandardNewMacro(InteractorStyleImage);

InteractorStyleImage::InteractorStyleImage()
{
	paintboard = vtkSmartPointer<vtkImageData>::New();
}

void InteractorStyleImage::SetStatusMapper(vtkTextMapper* statusMapper)
{
    _StatusMapper = statusMapper;
}

void InteractorStyleImage::SetWidget(CT_2d_Widget* widget)
{
    this->widget = widget;
}

void InteractorStyleImage::setImageActor(vtkSmartPointer<vtkImageActor> actor)
{
	if(!actor)
	{
		qDebug() << "actor is nullptr";
		return;
	}
	if(!actor->GetInput())
	{
		qDebug() << "the input of actor is empty";
		return;
	}
	this->actor = actor;
	//this->actor->SetPickable(false);
	if(!actorSegment && showSegmentActor)
	{
		actorSegment = vtkSmartPointer<vtkImageActor>::New();
		this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(actorSegment);
		actorSegment->SetInputData(paintboard);
		actorSegment->SetOpacity(.5);
	}
}

void InteractorStyleImage::applyROI()
{
	if (mask.isNull())
		return;
	QImage msk = mask.copy();
	QPainter painter(&msk);
	painter.setPen(Qt::white);
	painter.setBrush(Qt::white);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.drawRect(startPoint.x, startPoint.y, endPoint.x - startPoint.x, endPoint.y - startPoint.y);
	int* dims = paintboard->GetDimensions();
	uStatus::g_ctimage.getAverageMap().clear();
	for(int m=0;m<uStatus::g_ctimage.getNumberOfImages();++m)
	{
		cv::Mat cvmask = cv::Mat(msk.height(), msk.width(), CV_8UC3, (void*)msk.constBits(), msk.bytesPerLine());
		cv::Mat result(dims[0], dims[1], CV_8UC1, uStatus::g_ctimage.getCTImageDataVtk_Segmented_Src(m)->GetScalarPointer());
		int averagePixel = 0;
		int pixelNum = 0;
		for (int x = 0; x < cvmask.rows; x++)
			for (int y = 0; y < cvmask.cols; y++)
			{
				uchar v1 = cvmask.at<cv::Vec3b>(x, y)[0] == 255 ? 1 : 0;
				uchar v2 = cvmask.at<cv::Vec3b>(x, y)[1] == 255 ? 1 : 0;
				uchar v3 = cvmask.at<cv::Vec3b>(x, y)[2] == 255 ? 1 : 0;
				if (v1 || v2 || v3) {
					uchar v = result.at<uchar>(x, y);
					++ pixelNum;
					averagePixel += v;
				}
			}
		averagePixel = averagePixel *1./pixelNum;
		uStatus::g_ctimage.getAverageMap().emplace(m, averagePixel);
	}
	this->Interactor->Render();
}

void InteractorStyleImage::MoveSliceForward()
{
    if (_Slice < uStatus::g_ctimage.getNumberOfImages()-1)
    {
        _Slice += 1;
        widget->setSlice(_Slice);
		if (actorSegment)
		{
			paintboard->DeepCopy(uStatus::g_ctimage.getCTImageDataVtk_Segmented(_Slice));
			actorSegment->SetInputData(paintboard);
			actorSegment->GetProperty()->SetColorLevel(actor->GetProperty()->GetColorLevel());
			actorSegment->GetProperty()->SetColorWindow(actor->GetProperty()->GetColorWindow());
			if (!this->mask.isNull()) {
				int* dimension = paintboard->GetDimensions();
				cv::Mat src(dimension[0], dimension[1], CV_8UC3, paintboard->GetScalarPointer());

				QImage qimage((uchar*)src.data, src.cols, src.rows, src.step, QImage::Format_RGB888);
				QPainter painter(&qimage);
				painter.setPen(Qt::white);
				painter.setRenderHint(QPainter::Antialiasing);
				painter.drawRect(startPoint.x, startPoint.y, endPoint.x - startPoint.x, endPoint.y - startPoint.y);

			}
			paintboard->Modified();
			actorSegment->SetInputData(paintboard);
		}
        std::string msg = StatusMessage::Format(_Slice, uStatus::g_ctimage.getNumberOfImages());
        _StatusMapper->SetInput(msg.c_str());
        widget->Render();
    }
}

void InteractorStyleImage::MoveSliceBackward()
{
    if (_Slice > 0)
    {
        _Slice -= 1;
        widget->setSlice(_Slice);
		if (actorSegment) {
			paintboard->DeepCopy(uStatus::g_ctimage.getCTImageDataVtk_Segmented(_Slice));
			actorSegment->GetProperty()->SetColorLevel(actor->GetProperty()->GetColorLevel());
			actorSegment->GetProperty()->SetColorWindow(actor->GetProperty()->GetColorWindow());
			if (!this->mask.isNull()) {
				int* dimension = paintboard->GetDimensions();
				cv::Mat src(dimension[0], dimension[1], CV_8UC3, paintboard->GetScalarPointer());

				QImage qimage((uchar*)src.data, src.cols, src.rows, src.step, QImage::Format_RGB888);
				QPainter painter(&qimage);
				painter.setPen(Qt::white);
				painter.setRenderHint(QPainter::Antialiasing);
				painter.drawRect(startPoint.x, startPoint.y, endPoint.x - startPoint.x, endPoint.y - startPoint.y);
				
			}
			paintboard->Modified();
			actorSegment->SetInputData(paintboard);
		}
        std::string msg = StatusMessage::Format(_Slice, uStatus::g_ctimage.getNumberOfImages());
        _StatusMapper->SetInput(msg.c_str());
        widget->Render();
    }
}

void InteractorStyleImage::OnKeyDown()
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

void InteractorStyleImage::OnMouseWheelForward()
{
    MoveSliceForward();
}

void InteractorStyleImage::OnMouseWheelBackward()
{
    MoveSliceBackward();
}
void InteractorStyleImage::OnLeftButtonDown()
{
	if (drawState == false) {
		return Superclass::OnLeftButtonDown();
	}
	int* screenPos = this->Interactor->GetEventPosition();
	vtkRenderer* renderer = this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
	vtkPointPicker* picker = vtkPointPicker::SafeDownCast(this->Interactor->GetPicker());
	vtkImageData* image = actorSegment->GetInput();
	if (picker)
	{
		picker->Pick(screenPos[0], screenPos[1], 0, renderer);
		double* worldPos = picker->GetPickPosition();
		double ijk[3];
		image->TransformPhysicalNormalToContinuousIndex(worldPos, ijk);
		startPoint = cv::Point(ijk[0], ijk[1]);
		leftButtonPressed = 1;
	}
}

void InteractorStyleImage::OnMouseMove()
{
	if (!leftButtonPressed) {
		return Superclass::OnMouseMove();
	}
	int* screenPos = this->Interactor->GetEventPosition();
	vtkRenderer* renderer = this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
	vtkPointPicker* picker = vtkPointPicker::SafeDownCast(this->Interactor->GetPicker());
	if (picker)
	{
		picker->Pick(screenPos[0], screenPos[1], 0, renderer);
		double* worldPos = picker->GetPickPosition();
		double ijk[3];
		paintboard->TransformPhysicalNormalToContinuousIndex(worldPos, ijk);
		cv::Point endPoint = cv::Point(ijk[0], ijk[1]);
		int* dimension = paintboard->GetDimensions();
		paintboard->DeepCopy(uStatus::g_ctimage.getCTImageDataVtk_Segmented(_Slice));
		cv::Mat src(dimension[0], dimension[1], CV_8UC3, paintboard->GetScalarPointer());
		if (drawMode == DRAW_RECTANGLE)
		{
			QImage qimage((uchar*)src.data, src.cols, src.rows, src.step, QImage::Format_RGB888);
			QPainter painter(&qimage);
			painter.setPen(Qt::white);
			painter.setRenderHint(QPainter::Antialiasing);
			painter.drawRect(startPoint.x, startPoint.y, endPoint.x - startPoint.x, endPoint.y - startPoint.y);

			mask = qimage.copy();
			mask.fill(0);
			QPainter painter2(&mask);
			painter2.setPen(Qt::white);
			painter2.setRenderHint(QPainter::Antialiasing);
			//painter2.setBrush(Qt::white);
			painter2.drawRect(startPoint.x, startPoint.y, endPoint.x - startPoint.x, endPoint.y - startPoint.y);
		}
		else if (drawMode == DRAW_ELLIPSE)
		{
			QImage qimage((uchar*)src.data, src.cols, src.rows, src.step, QImage::Format_RGB888);
			QPainter painter(&qimage);
			painter.setPen(Qt::white);
			painter.setRenderHint(QPainter::Antialiasing);
			painter.drawEllipse(startPoint.x, startPoint.y, endPoint.x - startPoint.x, endPoint.y - startPoint.y);

			mask = qimage.copy();
			mask.fill(0);
			QPainter painter2(&mask);
			painter2.setPen(Qt::white);
			painter2.setRenderHint(QPainter::Antialiasing);
			//painter2.setBrush(Qt::white);
			painter2.drawEllipse(startPoint.x, startPoint.y, endPoint.x - startPoint.x, endPoint.y - startPoint.y);
		}
		paintboard->Modified();
		actorSegment->SetInputData(paintboard);
		this->Interactor->Render();
		this->endPoint = endPoint;
	}
}

void InteractorStyleImage::OnLeftButtonUp()
{
	leftButtonPressed = 0;
	return Superclass::OnLeftButtonUp();
}