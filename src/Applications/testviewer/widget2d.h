#include <QWidget>
#include <vtkEventQtSlotConnect.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkMatrix4x4.h>
#include <vtkLookupTable.h>
#include <vtkImageMapToColors.h>
#include <vtkImageActor.h>
#include <vtkImageResliceToColors.h>
#include <vtkImageResliceMapper.h>

namespace Ui 
{
class Widget2D;
}
class Widget2D : public QWidget {
    Q_OBJECT
public:
    enum SliceOrientation
    {
        Axial,
        Coronal,
        Sagittal
    };
    Widget2D(QWidget *parent = nullptr);
    void setSliceOrientation(SliceOrientation ori);
    void setInputImage(vtkImageData *image);
    void resetView();
    void renderImage();

protected:

private:
    vtkSmartPointer<vtkEventQtSlotConnect> m_scrollConnection = nullptr;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_renderwindow;
    Ui::Widget2D *ui;
    double m_sagittalMatrix[16] = {0, 0, 1, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 1};
    double m_coronalMatrix[16] = {1, 0, 0, 0, 0, 0, 1, 0, 0, -1, 0, 0, 0, 0, 0, 1};
    double m_axialMatrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    vtkSmartPointer<vtkMatrix4x4> m_resliceAxes;
    vtkSmartPointer<vtkImageResliceToColors> m_reslice;
    vtkSmartPointer<vtkLookupTable> m_colorTable;
    vtkSmartPointer<vtkImageActor> m_imgActor;
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkNew<vtkImageResliceMapper> m_mapper;
};
