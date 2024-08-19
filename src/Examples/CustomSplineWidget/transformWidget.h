#pragma once

#include "vtkAbstractWidget.h"
#include <vtkSphereHandleRepresentation.h>
#include <vtkHandleWidget.h>
#include <vtkCaptionWidget.h>
class transformRepresentation : public vtkWidgetRepresentation {
public:
    enum INTERACTIONSTATE { outside = 0, onXRing, onYRing, onZRing, onXArrow, onYArrow, onZArrow };
    transformRepresentation();
    static transformRepresentation *New();
    vtkTypeMacro(transformRepresentation, vtkWidgetRepresentation);

    void PlaceWidget(double bounds[6]) override;
    void StartWidgetInteraction(double e[2]) override;
    void WidgetInteraction(double e[2]) override;
    int ComputeInteractionState(int X, int Y, int modify = 0) override;
    void Highlight(int highlight) override;

    void BuildRepresentation() override;

    void GetActors(vtkPropCollection *pc) override;

};
class vtkCallbackCommand;

class transformWidget : public vtkAbstractWidget
{
   public:
    static transformWidget *New();

    vtkTypeMacro(transformWidget, transformWidget);
    void PrintSelf(ostream &os, vtkIndent indent) override;

    void SetEnabled(int enabling) override;

    void SetInteractor(vtkRenderWindowInteractor* iren) override;

    void CreateDefaultRepresentation() override;

   protected:
    float calculateArea(vtkPoints *vp);
       bool fitSurfaceProjectWarp(vtkPoints *curvePoints, vtkPolyData *surface, vtkIdType numberOfInternalGridPoints=225);
    bool IsPolygonClockwise(vtkPoints *points, vtkIdList *pointIds = nullptr);
    void updateLines(bool calculate=true);
    transformWidget();
    ~transformWidget() override;

    static void SelectAction(vtkAbstractWidget *w);
    static void EndSelectAction(vtkAbstractWidget *w);
    static void MoveSpline(vtkAbstractWidget *w);
    static void MoveSplineDone(vtkAbstractWidget *w);
    static void MoveAction(vtkAbstractWidget *w);
    static void ProcessKeyEvents(vtkObject *, unsigned long, void *, void *);

   private:
    enum WIDGETSTATE
    {
        pickPoint,
        interactivePoint,
        interactiveCaption,
        dragSpline
    };
    int state_;
    vtkCallbackCommand *keyEventCallbackCommand_;

    transformWidget(const transformWidget &) = delete;
    void operator=(const transformWidget &) = delete;
    std::vector<vtkHandleWidget *> m_controlPoints;
    vtkNew<vtkActor> actor;
    bool isadded{false};
    vtkHandleWidget *m_interactiveControlPoint{nullptr};
    vtkSmartPointer<vtkCaptionWidget> captionWidget = vtkSmartPointer<vtkCaptionWidget>::New();
};
