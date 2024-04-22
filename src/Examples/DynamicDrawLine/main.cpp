#include "Windows.h"
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkStructuredPoints.h>
#include <vtkStructuredPointsReader.h>
#include <vtkVolumeProperty.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkDicomImageReader.h>
#include <vtkSmartPointer.h>
#include <vtkAutoInit.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPointPicker.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkSetGet.h>
#include <vtkImageViewer2.h>
#include <vtkImageActor.h>
#include <vtkRendererCollection.h>
#include <vtkProperty.h>
#include <winuser.h>
#include <iostream>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);
class PointPickerInteractorStyle1 : public vtkInteractorStyleTrackballCamera {
public:
    static PointPickerInteractorStyle1* New();
    vtkTypeMacro(PointPickerInteractorStyle1, vtkInteractorStyleTrackballCamera);
    virtual void OnLeftButtonDown()
    {
        isLeftMousePressed = 1;
        m_points = vtkSmartPointer<vtkPoints>::New();
        cells = vtkSmartPointer<vtkCellArray>::New();
        // 注册拾取点函数
        this->Interactor->GetPicker()->Pick(
            this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1],
            0,  //主要获取的是二维平面的像素点坐标
            this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()  //获取vtkrender对象
        );

        // 打印拾取点空间位置
        double picked[3];
        this->Interactor->GetPicker()->GetPickPosition(picked);
        m_points->InsertNextPoint(picked[0], picked[1], picked[2]);
        polyLine->GetPointIds()->SetNumberOfIds(m_points->GetNumberOfPoints());
        for (int i = 0; i < m_points->GetNumberOfPoints(); i++) {
            polyLine->GetPointIds()->SetId(i, i);
        };
        cells->InsertNextCell(polyLine);
        outputVector->SetPoints(m_points);
        outputVector->SetLines(cells);
        vtkSmartPointer<vtkPolyDataMapper> LineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        LineMapper->SetInputData(outputVector);
        vtkSmartPointer<vtkActor> LineActor = vtkSmartPointer<vtkActor>::New();
        LineActor->SetMapper(LineMapper);
        LineActor->GetProperty()->SetColor(1, 0, 0);
        LineActor->GetProperty()->SetLineWidth(2);
        this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(LineActor);
        this->Interactor->Render();
        //return vtkInteractorStyleTrackballCamera::OnLeftButtonDown(); 
    }
    virtual void OnLeftButtonUp() { isLeftMousePressed = false;
        //return vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
    }
    virtual void OnMouseMove()
    {
        if (isLeftMousePressed) {

            this->Interactor->GetPicker()->Pick(
                this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1],
                0,this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
            double picked[3];
            this->Interactor->GetPicker()->GetPickPosition(picked);
            m_points->InsertNextPoint(picked[0], picked[1], picked[2]);
            int numOfPoints = m_points->GetNumberOfPoints();
            polyLine->GetPointIds()->InsertNextId(numOfPoints);
            polyLine->GetPointIds()->SetId(numOfPoints - 1, numOfPoints - 1);
            m_points->Modified();
            cells->Initialize();              // reset the cells to remove the old spiral
            cells->InsertNextCell(polyLine);  // re-insert the updated spiral
            cells->Modified();                // required to update
            outputVector->Modified();
            // renderer->GetRenderWindow()->GetInteractor()->Initialize();
            //  renderer->Render();
            this->Interactor->GetRenderWindow()->Render();
            //return vtkInteractorStyleTrackballCamera::OnMouseMove(); 
        }
    };

private:
    vtkSmartPointer<vtkPoints> m_points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
    vtkSmartPointer<vtkPolyData> outputVector = vtkSmartPointer<vtkPolyData>::New();
    bool isLeftMousePressed{false};
};

vtkStandardNewMacro(PointPickerInteractorStyle1)
#include <vtkCubeSource.h>
#include <vtkCellPicker.h>
int main()
{
    //vtkSmartPointer<vtkDICOMImageReader> jpegReader = vtkSmartPointer<vtkDICOMImageReader>::New();
    //jpegReader->SetFileName("D:\\image\\CTJ212345\\exported0000.dcm");  // 替换为你的图像文件路径

    //vtkSmartPointer<vtkImageViewer2> imageViewer = vtkSmartPointer<vtkImageViewer2>::New();
    //imageViewer->SetInputConnection(jpegReader->GetOutputPort());
    vtkNew<vtkCubeSource> cube;
    cube->Update();
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(cube->GetOutput());
    vtkNew<vtkActor> ac;
    ac->SetMapper(mapper);
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer);
    renderWindow->SetWindowName("Planar");
    //renderer->AddActor(imageViewer->GetImageActor());
    renderer->AddActor(ac);
    renderer->SetBackground(0, 0, 0);
    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
        vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindowInteractor->SetRenderWindow(renderWindow);
    vtkSmartPointer<PointPickerInteractorStyle1> style = vtkSmartPointer<PointPickerInteractorStyle1>::New();
    renderWindowInteractor->SetInteractorStyle(style);
    vtkNew<vtkCellPicker> picker;
    renderWindowInteractor->SetPicker(picker);
    //renderWindow->SetMultiSamples(0);
    //renderWindow->SetPointSmoothing(0);
    //renderWindow->SetLineSmoothing(0);
    //renderWindow->SetPolygonSmoothing(0);
    renderWindow->Render();
    renderWindowInteractor->Initialize();
    renderWindowInteractor->Start();

    return EXIT_SUCCESS;
}