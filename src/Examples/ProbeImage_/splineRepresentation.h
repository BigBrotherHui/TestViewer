
#ifndef splineRepresentation_h
#define splineRepresentation_h

#include "vtkInteractionWidgetsModule.h" // For export macro
#include "vtkWidgetRepresentation.h"
#include <QVector>

class vtkHandleRepresentation;

class splineRepresentation : public vtkWidgetRepresentation {
public:
  ///@{
  /**
   * Standard VTK methods.
   */
  vtkTypeMacro(splineRepresentation, vtkWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  ///@}

  /**
   * This representation and all subclasses must keep a distance
   * consistent with the state of the widget.
   */
  virtual double GetDistance() = 0;

  ///@{
  /**
   * Methods to Set/Get the coordinates of the two points defining
   * this representation. Note that methods are available for both
   * display and world coordinates.
   */
  virtual void GetPointWorldPosition(int no,double pos[3]) = 0;
  virtual double* GetPointWorldPosition(int no) VTK_SIZEHINT(3) = 0;
  virtual void SetPointDisplayPosition(int no,double pos[3]);
  virtual void GetPointDisplayPosition(int no,double pos[3]) = 0;
  virtual void SetPointWorldPosition(int no,double pos[3]) = 0;

  void SetHandleRepresentation(vtkHandleRepresentation* handle);
  void InstantiateHandleRepresentation();

  vtkHandleRepresentation *GetPointRepresentation(int i);

  vtkSetClampMacro(Tolerance, int, 1, 100);
  vtkGetMacro(Tolerance, int);

  vtkSetStringMacro(LabelFormat);
  vtkGetStringMacro(LabelFormat);

  vtkSetMacro(Scale, double);
  vtkGetMacro(Scale, double);

  vtkSetMacro(RulerMode, vtkTypeBool);
  vtkGetMacro(RulerMode, vtkTypeBool);
  vtkBooleanMacro(RulerMode, vtkTypeBool);

  vtkSetClampMacro(RulerDistance, double, 0, VTK_FLOAT_MAX);
  vtkGetMacro(RulerDistance, double);

  vtkSetClampMacro(NumberOfRulerTicks, int, 1, VTK_INT_MAX);
  vtkGetMacro(NumberOfRulerTicks, int);

  enum
  {
    Outside = 0,
    Near
  };

  void BuildRepresentation() override;
  int ComputeInteractionState(int X, int Y, int modify = 0) override;
  void StartWidgetInteraction(double e[2]) override;
  void WidgetInteraction(double e[2]) override;
  void StartComplexInteraction(vtkRenderWindowInteractor* iren, vtkAbstractWidget* widget,
    unsigned long event, void* calldata) override;
  void ComplexInteraction(vtkRenderWindowInteractor* iren, vtkAbstractWidget* widget,
    unsigned long event, void* calldata) override;
  int ComputeComplexInteractionState(vtkRenderWindowInteractor* iren, vtkAbstractWidget* widget,
    unsigned long event, void* calldata, int modify = 0) override;
  ///@}

protected:
  void addPointRepresentation();
  splineRepresentation();
  ~splineRepresentation() override;

  // The handle and the rep used to close the handles
  vtkHandleRepresentation* HandleRepresentation;
  QVector<vtkHandleRepresentation*> PointRepresentation;

  // Selection tolerance for the handles
  int Tolerance;

  // Format for printing the distance
  char* LabelFormat;

  // Scale to change from the VTK world coordinates to the desired coordinate
  // system.
  double Scale;

  // Ruler related stuff
  vtkTypeBool RulerMode;
  double RulerDistance;
  int NumberOfRulerTicks;

private:
  splineRepresentation(const splineRepresentation&) = delete;
  void operator=(const splineRepresentation&) = delete;
};

#endif
