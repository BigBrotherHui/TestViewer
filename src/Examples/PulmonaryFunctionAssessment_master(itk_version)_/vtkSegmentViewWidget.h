#pragma once
#include <QVTKOpenGLNativeWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include "LineInteractorStyle.h"
#include <vtkGenericRenderWindowInteractor.h>
class vtkSegmentViewWidget : public QVTKOpenGLNativeWidget
{
    Q_OBJECT
public:
    explicit vtkSegmentViewWidget(QWidget* parent = nullptr);
    ~vtkSegmentViewWidget();
	void showImage(vtkSmartPointer<vtkImageData> image);
    void setConnectedThresholdMinMax(int min,int max);
    void FillHole();
    void ExtractLargestRegion();
    vtkSmartPointer<vtkImageData> getForegroundImageData();
    void addBrush();
protected:
    void fillHole(vtkSmartPointer<vtkImageData> image);
    void extractLargestRegion(vtkSmartPointer<vtkImageData> image);
private:
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;
    vtkSmartPointer<vtkRenderer> renderer;
	vtkSmartPointer<LineInteractorStyle> style;
    vtkSmartPointer<vtkImageActor> actorForeground;
    vtkSmartPointer<vtkImageActor> actorBackground;
    vtkSmartPointer<vtkActor> actorBrush{nullptr};

    //vtkSmartPointer<vtkCustomImageViewer2> viewer2;
};
;
