#include "myInteractorStyleImage.h"
#include <vtkTextMapper.h>
#include "ct_2d_widget.h"
vtkStandardNewMacro(myInteractorStyleImage);
void myInteractorStyleImage::SetImageViewer(CT_2d_Widget* widget)
{
    _ImageViewer = widget->getImageViewer2();
    this->widget = widget;
    _MinSlice = _ImageViewer->GetSliceMin();
    _MaxSlice = _ImageViewer->GetSliceMax();
    _Slice = _MinSlice;
}

void myInteractorStyleImage::SetStatusMapper(vtkTextMapper* statusMapper)
{
    _StatusMapper = statusMapper;
}

void myInteractorStyleImage::initSlice()
{
    _Slice = 1;
    _ImageViewer->SetSlice(_Slice);
    std::string msg = StatusMessage::Format(_Slice, _MaxSlice);
    _StatusMapper->SetInput(msg.c_str());
    if (widget)
        widget->Render();
}

void myInteractorStyleImage::MoveSliceForward()
{
    if (_Slice < _MaxSlice)
    {
        _Slice += 1;
        _ImageViewer->SetSlice(_Slice);
        std::string msg = StatusMessage::Format(_Slice, _MaxSlice);
        _StatusMapper->SetInput(msg.c_str());
        //_ImageViewer->Render();
        if (widget)
            widget->Render();
    }
}

void myInteractorStyleImage::MoveSliceBackward()
{
    if (_Slice > _MinSlice)
    {
        _Slice -= 1;
        _ImageViewer->SetSlice(_Slice);
        std::string msg = StatusMessage::Format(_Slice, _MaxSlice);
        _StatusMapper->SetInput(msg.c_str());
        //_ImageViewer->Render();
        if (widget)
            widget->Render();
    }
}

void myInteractorStyleImage::OnKeyDown()
{
    std::string key = this->GetInteractor()->GetKeySym();
    if (key.compare("Up") == 0)
    {
        // cout << "Up arrow key was pressed." << endl;
        MoveSliceForward();
    }
    else if (key.compare("Down") == 0)
    {
        // cout << "Down arrow key was pressed." << endl;
        MoveSliceBackward();
    }
    // forward event
    vtkInteractorStyleImage::OnKeyDown();
}

void myInteractorStyleImage::OnMouseWheelForward()
{
    MoveSliceForward();
}

void myInteractorStyleImage::OnMouseWheelBackward()
{
    if (_Slice > _MinSlice)
    {
        MoveSliceBackward();
    }
}
