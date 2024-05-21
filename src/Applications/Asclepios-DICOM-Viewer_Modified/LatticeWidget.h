#ifndef LATTICEWIDGET_H
#define LATTICEWIDGET_H

#include <QWidget>
#include <vtkImageResliceToColors.h>

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
    void centerImageActors(std::array<std::array<double, 3>, 3> position);
    void Render();

protected:
    void centerImageActors();

private:
    Ui::LatticeWidget *ui;
};
#endif // LATTICEWIDGET_H
