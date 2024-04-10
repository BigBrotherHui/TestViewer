#include "widget2d.h"
#include "ui_widget2d.h"
#include <vtkImageData.h>
#include <vtkRenderer.h>
#include <vtkInteractorStyleImage.h>
#include <vtkImageProperty.h>
#include <vtkCamera.h>
#include <QDebug>
class vtkImageInteractionCallback : public vtkCommand {
public:
    static vtkImageInteractionCallback *New()  //回调函数初始化函数
    {
        return new vtkImageInteractionCallback;
    }
    vtkImageInteractionCallback()
    {
        this->Slicing = 0;
        this->ImageReslice = 0;
        this->Interactor = 0;
    }
    Widget2D::SliceOrientation ori{Widget2D::Axial};
    void SetImageReslice(vtkImageResliceToColors *reslice) { this->ImageReslice = reslice; }
    vtkImageReslice *GetImageReslice() { return this->ImageReslice; }

    void SetInteractor(vtkRenderWindowInteractor *interactor) { this->Interactor = interactor; }
    vtkRenderWindowInteractor *GetInteractor() { return this->Interactor; }
    virtual void Execute(vtkObject *, unsigned long event, void *)
    {
        vtkRenderWindowInteractor *interactor = GetInteractor();
        int lastPos[2];
        interactor->GetLastEventPosition(lastPos);
        int currPos[2];
        interactor->GetEventPosition(currPos);

        if (event == vtkCommand::LeftButtonPressEvent) {
            this->Slicing = 1;  //标志位
        }
        else if (event == vtkCommand::LeftButtonReleaseEvent) {
            this->Slicing = 0;  //标志位
        }
        else if (event == vtkCommand::MouseMoveEvent) {
            if (this->Slicing)  //检验鼠标左键已经按下 正在执行操作
            {
                vtkImageResliceToColors *reslice = this->ImageReslice;

                //记下鼠标Y向变化的幅值大小
                int deltaY = lastPos[1] - currPos[1];

                //记下鼠标X向变化的幅值大小
                int deltaX = lastPos[0] - currPos[0];

                // reslice->Update();
                //注意这里获取的sliceSpacing并不是z轴的spacing而是与当前slice的面垂直的那个方向的spacing，比如axial就是spacing[2]，
                //如果是sagittal或是coronal就是spacing[0]和spacing[1]
                double sliceSpacing = reslice->GetOutput()->GetSpacing()[2];
                /*qDebug() << reslice->GetOutput()->GetSpacing()[0] << reslice->GetOutput()->GetSpacing()[1]
                         << reslice->GetOutput()->GetSpacing()[2];*/
                vtkMatrix4x4 *matrix = reslice->GetResliceAxes();
                //重新定位切片需要经过的中心点
                double point[4];
                double center[4];
                point[0] = 0;
                point[1] = 0;
                point[2] = sliceSpacing * deltaY;
                point[3] = 1.0;
                matrix->MultiplyPoint(point, center);

                matrix->SetElement(0, 3, center[0]);
                matrix->SetElement(1, 3, center[1]);
                matrix->SetElement(2, 3, center[2]);
                static double colorWindowLevel = 500;
                static double colorWindowWith = 2000;
                colorWindowLevel += deltaX;

                /*reslice->GetLookupTable()->SetRange(colorWindowLevel - colorWindowWith / 2,
                                                     colorWindowLevel + colorWindowWith / 2);*/

                reslice->Update();
                interactor->Render();
            }
            else {
                vtkInteractorStyle *style = vtkInteractorStyle::SafeDownCast(interactor->GetInteractorStyle());
                if (style) {
                    style->OnMouseMove();
                }
            }
        }
    }

private:
    int Slicing;
    vtkImageResliceToColors *ImageReslice;
    vtkRenderWindowInteractor *Interactor;
};
Widget2D::Widget2D(QWidget* parent) : ui(new Ui::Widget2D)
{
    ui->setupUi(this);
    m_renderwindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    ui->widget->setRenderWindow(m_renderwindow);
    m_resliceAxes = vtkSmartPointer<vtkMatrix4x4>::New();
    m_reslice = vtkSmartPointer<vtkImageResliceToColors>::New();
    m_reslice->BypassOff();
    m_reslice->SetOutputDimensionality(2);
    m_reslice->SetSlabNumberOfSlices(0);
    m_reslice->SetOutputFormatToRGB();
    m_colorTable = vtkSmartPointer<vtkLookupTable>::New();
    m_colorTable->SetRange(-500, 1500);
    m_colorTable->SetValueRange(0.0, 1.0);
    m_colorTable->SetSaturationRange(0.0, 0.0);
    m_colorTable->SetRampToLinear();
    m_colorTable->Build();
    m_reslice->SetLookupTable(m_colorTable);

    m_imgActor = vtkSmartPointer<vtkImageActor>::New();
    m_renderer = vtkSmartPointer<vtkRenderer>::New();
    m_renderer->AddActor(m_imgActor);
    m_renderer->SetBackground(0.4, 0.5, 0.6);
    m_renderwindow->AddRenderer(m_renderer);
    m_mapper->SeparateWindowLevelOperationOff();
    m_mapper->SetInputConnection(m_reslice->GetOutputPort());
    m_imgActor->GetProperty()->SetInterpolationTypeToCubic();
    m_imgActor->SetMapper(m_mapper);
    m_renderer->GetActiveCamera()->SetParallelProjection(1);

    vtkSmartPointer<vtkInteractorStyleImage> imagestyle = vtkSmartPointer<vtkInteractorStyleImage>::New();
    ui->widget->interactor()->SetInteractorStyle(imagestyle);
    vtkSmartPointer<vtkImageInteractionCallback> callback = vtkSmartPointer<vtkImageInteractionCallback>::New();
    callback->SetImageReslice(m_reslice);
    callback->SetInteractor(ui->widget->interactor());

    imagestyle->AddObserver(vtkCommand::MouseMoveEvent, callback);
    imagestyle->AddObserver(vtkCommand::LeftButtonPressEvent, callback);
    imagestyle->AddObserver(vtkCommand::LeftButtonReleaseEvent, callback);
    setSliceOrientation(Coronal);
}

void Widget2D::setSliceOrientation(SliceOrientation ori)
{
    switch (ori)
    {
        case Axial: {
            m_resliceAxes->DeepCopy(m_axialMatrix);
            break;
        }
        case Sagittal: {
            m_resliceAxes->DeepCopy(m_coronalMatrix);
            break;
        }
        case Coronal: {
            m_resliceAxes->DeepCopy(m_sagittalMatrix);
            break;
        }
    }
}

void Widget2D::setInputImage(vtkImageData *image)
{
    if (!image || m_reslice->GetInput() == image) return;
    int extent[6];
    double spacing[3];
    double origin[3];
    image->GetExtent(extent);
    image->GetSpacing(spacing);
    image->GetOrigin(origin);
    double center[3];
    center[0] = origin[0] + spacing[0] * 0.5 * (extent[0] + extent[1]);
    center[1] = origin[1] + spacing[1] * 0.5 * (extent[2] + extent[3]);
    center[2] = origin[2] + spacing[2] * 0.5 * (extent[4] + extent[5]);
    m_resliceAxes->SetElement(0, 3, center[0]);
    m_resliceAxes->SetElement(1, 3, center[1]);
    m_resliceAxes->SetElement(2, 3, center[2]);
    m_reslice->SetInputData(image);
    m_reslice->SetInformationInput(image);
    m_reslice->SetOutputDimensionality(2);
    m_reslice->SetResliceAxes(m_resliceAxes);
    m_reslice->SetInterpolationModeToLinear();
}

void Widget2D::resetView()
{
    m_renderer->ResetCamera();
    renderImage();
}

void Widget2D::renderImage()
{
    m_renderwindow->Render();
}
