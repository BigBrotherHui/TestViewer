#include <vtkInteractorStyleImage.h>
#include <vtkImageViewer2.h>
#include <vtkCornerAnnotation.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindowInteractor.h>
#include <sstream>
class CT_2d_Widget;
class StatusMessage
{
public:
    static std::string Format(int slice, int maxSlice)
    {
        std::stringstream tmp;
        tmp << "Slice Number:" << slice + 1 << "/" << maxSlice + 1;
        return tmp.str();
    }
};

class myInteractorStyleImage : public vtkInteractorStyleImage
{
public:
    static myInteractorStyleImage* New();
    vtkTypeMacro(myInteractorStyleImage, vtkInteractorStyleImage);

protected:
    vtkImageViewer2* _ImageViewer;
    vtkTextMapper* _StatusMapper;
    int _Slice;
    int _MinSlice;
    int _MaxSlice;
    CT_2d_Widget* widget{nullptr};
public:
    void SetImageViewer(CT_2d_Widget* imageViewer);

    void SetStatusMapper(vtkTextMapper* statusMapper);
    void initSlice();
    void MoveSliceForward();

    void MoveSliceBackward();
protected:

    virtual void OnKeyDown();

    virtual void OnMouseWheelForward();

    virtual void OnMouseWheelBackward();
};
