#pragma once

#include <vtkInteractorStyleImage.h>
#include <vtkCornerAnnotation.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindowInteractor.h>
#include <sstream>
#include <opencv2/opencv.hpp>
#include <QImage>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkImageActor.h>
#include <QDebug>
class CT_2d_Widget;
class StatusMessage
{
public:
    static std::string Format(int slice, int maxSlice)
    {
        slice += 1;
        if (slice > maxSlice)
            slice = maxSlice;
        else if (slice <= 1)
            slice = 1;
        std::stringstream tmp;
        tmp << "Slice Number:" << slice << "/" << maxSlice;
        return tmp.str();
    }
};

class InteractorStyleImage : public vtkInteractorStyleImage
{
public:
    enum DRAW_MODE
    {
        DRAW_ELLIPSE,
        DRAW_RECTANGLE,
        DRAW_NOTUSED
    };

    static InteractorStyleImage* New();
    InteractorStyleImage();
    vtkTypeMacro(InteractorStyleImage, vtkInteractorStyleImage);

    void SetStatusMapper(vtkTextMapper* statusMapper);
    void SetWidget(CT_2d_Widget *widget);
    void setImageActor(vtkSmartPointer<vtkImageActor> actor);
    bool drawState = false;
    DRAW_MODE drawMode = DRAW_NOTUSED;
    bool showSegmentActor{ false };
    void applyROI();
protected:
	void MoveSliceForward();
    void MoveSliceBackward();
    vtkTextMapper* _StatusMapper;
    int _Slice{1};
    CT_2d_Widget* widget{nullptr};

protected:
    virtual void OnKeyDown();
    virtual void OnMouseWheelForward();
    virtual void OnMouseWheelBackward();
    void OnLeftButtonDown() override;
    void OnLeftButtonUp() override;
    void OnMouseMove() override;
private:
    
    QImage mask;
    cv::Point startPoint, endPoint;
    vtkSmartPointer<vtkImageActor> actor;
    vtkSmartPointer<vtkImageActor> actorSegment{nullptr};
    bool leftButtonPressed{ false };
    vtkSmartPointer<vtkImageData> paintboard;
};
