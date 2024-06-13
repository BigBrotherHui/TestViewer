#ifndef LATTICEWIDGET_H
#define LATTICEWIDGET_H

#include <QWidget>
#include <vtkImageResliceToColors.h>
#include "LatticeResliceWidget.h"
QT_BEGIN_NAMESPACE
namespace Ui { class LatticeWidget; }
QT_END_NAMESPACE

class LatticeWidget : public QWidget
{
    Q_OBJECT

public:
    LatticeWidget(QWidget *parent = nullptr);
    ~LatticeWidget();
    void setImageReslicers(
        const vtkSmartPointer<vtkImageResliceToColors>& m_firstReslice,
        const vtkSmartPointer<vtkImageResliceToColors>& m_secondReslice,
        const vtkSmartPointer<vtkImageResliceToColors>& m_thirdReslice);
    void setReslicersMatrix(vtkMatrix4x4* m_firstResliceMatrix,
        vtkMatrix4x4* m_secondResliceMatrix,
        vtkMatrix4x4* m_thirdResliceMatrix);
    void centerImageActors(int index, std::array<std::array<double, 3>, 3> position);
    void Render();
    void setResliceSpacing(double slicespacing);
    void setSlice(int frontSliceNum,int backSliceNum,int pickedSlice,double scale,int slabSliceCount);
    LatticeResliceWidget *getLatticeResliceWidget(int index);

protected:
    void centerImageActors();

private:
    Ui::LatticeWidget *ui;
    double m_sliceSpacing{1};
    int m_frontSliceNum{10};
    int m_backSliceNum{10};
    int m_pickedSlice{0};
    double m_scale{5};
    int m_slabSliceCount{1};
};
#endif // LATTICEWIDGET_H
