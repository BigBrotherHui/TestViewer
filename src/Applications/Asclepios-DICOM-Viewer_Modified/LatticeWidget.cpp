#include "LatticeWidget.h"
#include "ui_LatticeWidget.h"
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
LatticeWidget::LatticeWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::LatticeWidget)
{
    ui->setupUi(this);
}

LatticeWidget::~LatticeWidget()
{
    delete ui;
}

void LatticeWidget::setImageReslicers(const vtkSmartPointer<vtkImageResliceToColors>& m_firstReslice,
    const vtkSmartPointer<vtkImageResliceToColors>& m_secondReslice,
    const vtkSmartPointer<vtkImageResliceToColors>& m_thirdReslice)
{
    ui->widget_front->setImageReslicer(m_firstReslice);
    ui->widget_mid->setImageReslicer(m_secondReslice);
    ui->widget_back->setImageReslicer(m_thirdReslice);
}

void LatticeWidget::setReslicersMatrix(vtkMatrix4x4* m_firstResliceMatrix,
    vtkMatrix4x4* m_secondResliceMatrix,
    vtkMatrix4x4* m_thirdResliceMatrix)
{
    ui->widget_front->setResliceMatrix(m_firstResliceMatrix);
    ui->widget_mid->setResliceMatrix(m_secondResliceMatrix);
    ui->widget_back->setResliceMatrix(m_thirdResliceMatrix);
}

void LatticeWidget::centerImageActors(std::array<std::array<double, 3>, 3> position)
{
    auto reslicer = ui->widget_mid->getImageResicer();
    vtkSmartPointer<vtkMatrix4x4> matrixfront = vtkSmartPointer<vtkMatrix4x4>::New();
    vtkSmartPointer<vtkMatrix4x4> matrixcenter = vtkSmartPointer<vtkMatrix4x4>::New();
    vtkSmartPointer<vtkMatrix4x4> matrixback = vtkSmartPointer<vtkMatrix4x4>::New();
    vtkMatrix4x4* vmt = reslicer->GetResliceAxes();
    matrixfront->DeepCopy(vmt);
    matrixcenter->DeepCopy(vmt);
    matrixback->DeepCopy(vmt);
    const double sliceSpacing = m_sliceSpacing;
    double point[4];
    double center[4];
    point[0] = 0;
    point[1] = 0;
    point[2] = sliceSpacing * m_frontSliceNum*m_scale;
    point[3] = 1.0;
    matrixfront->MultiplyPoint(point, center);
    matrixfront->SetElement(0, 3, center[0]);
    matrixfront->SetElement(1, 3, center[1]);
    matrixfront->SetElement(2, 3, center[2]);

    point[2] = sliceSpacing * m_pickedSlice * m_scale;
    matrixcenter->MultiplyPoint(point, center);
    matrixcenter->SetElement(0, 3, center[0]);
    matrixcenter->SetElement(1, 3, center[1]);
    matrixcenter->SetElement(2, 3, center[2]);

    point[2] = -sliceSpacing * m_backSliceNum*m_scale;
    matrixback->MultiplyPoint(point, center);
    matrixback->SetElement(0, 3, center[0]);
    matrixback->SetElement(1, 3, center[1]);
    matrixback->SetElement(2, 3, center[2]);

    ui->widget_front->setResliceMatrix(matrixfront);
    ui->widget_mid->setResliceMatrix(matrixcenter);
    ui->widget_back->setResliceMatrix(matrixback);

    //ui->widget_front->centerImageActor(position[0]);
    //ui->widget_mid->centerImageActor(position[1]);
    //ui->widget_back->centerImageActor(position[2]);
}

void LatticeWidget::Render()
{
    ui->widget_front->renderWindow()->Render();
    ui->widget_mid->renderWindow()->Render();
    ui->widget_back->renderWindow()->Render();
}

void LatticeWidget::setResliceSpacing(double slicespacing)
{
    m_sliceSpacing = slicespacing;
}

void LatticeWidget::setSlice(int frontSliceNum, int backSliceNum, int pickedSlice,double scale)
{
    m_frontSliceNum = frontSliceNum;
    m_backSliceNum = backSliceNum;
    m_pickedSlice = pickedSlice;
    m_scale = scale;
}
