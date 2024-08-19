
#ifndef splineWidget_h
#define splineWidget_h

#include "vtkAbstractWidget.h"
#include "vtkInteractionWidgetsModule.h" // For export macro
#include <QVector>
#include "splineRepresentation.h"

class vtkHandleWidget;
class splineWidgetCallback;

class splineWidget : public vtkAbstractWidget
{
public:
  /**
   * Instantiate this class.
   */
  static splineWidget* New();

  ///@{
  /**
   * Standard methods for a VTK class.
   */
  vtkTypeMacro(splineWidget, vtkAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  ///@}

  /**
   * The method for activating and deactivating this widget. This method
   * must be overridden because it is a composite widget and does more than
   * its superclasses' vtkAbstractWidget::SetEnabled() method.
   */
  void SetEnabled(int) override;

  /**
   * Specify an instance of vtkWidgetRepresentation used to represent this
   * widget in the scene. Note that the representation is a subclass of vtkProp
   * so it can be added to the renderer independent of the widget.
   */
  void SetRepresentation(splineRepresentation* r)
  {
    this->Superclass::SetWidgetRepresentation(reinterpret_cast<vtkWidgetRepresentation*>(r));
  }

  splineRepresentation* GetDistanceRepresentation()
  { return reinterpret_cast<splineRepresentation*>(this->WidgetRep);
  }

  /**
   * Create the default widget representation if one is not set.
   */
  void CreateDefaultRepresentation() override;

  /**
   * Methods to change the whether the widget responds to interaction.
   * Overridden to pass the state to component widgets.
   */
  void SetProcessEvents(vtkTypeBool) override;

  /**
   * Description:
   * Enum defining the state of the widget. By default the widget is in Start mode,
   * and expects to be interactively placed. While placing the points the widget
   * transitions to Define state. Once placed, the widget enters the Manipulate state.
   */

  enum
  {
    Start = 0,
    Define,
    Manipulate
  };

  ///@{
  /**
   * Set the state of the widget. If the state is set to "Manipulate" then it
   * is assumed that the widget and its representation will be initialized
   * programmatically and is not interactively placed. Initially the widget
   * state is set to "Start" which means nothing will appear and the user
   * must interactively place the widget with repeated mouse selections. Set
   * the state to "Start" if you want interactive placement. Generally state
   * changes must be followed by a Render() for things to visually take
   * effect.
   */
  virtual void SetWidgetStateToStart();
  virtual void SetWidgetStateToManipulate();
  ///@}

  /**
   * Return the current widget state.
   */
  virtual int GetWidgetState() { return this->WidgetState; }

protected:
  splineWidget();
  ~splineWidget() override;

  // The state of the widget
  int WidgetState;
  int CurrentHandle;

  // Callback interface to capture events when
  // placing the widget.
  static void AddPointAction(vtkAbstractWidget*);
  static void MoveAction(vtkAbstractWidget*);
  static void EndSelectAction(vtkAbstractWidget*);
  static void AddPointAction3D(vtkAbstractWidget*);
  static void MoveAction3D(vtkAbstractWidget*);
  static void EndSelectAction3D(vtkAbstractWidget*);
  void addHandle();
  // The positioning handle widgets
  QVector<vtkHandleWidget*> PointWidget;
  QVector<splineWidgetCallback*> DistanceWidgetCallback;

  // Methods invoked when the handles at the
  // end points of the widget are manipulated
  void StartDistanceInteraction(int handleNum);
  void DistanceInteraction(int handleNum);
  void EndDistanceInteraction(int handleNum);

  friend class splineWidgetCallback;

private:
  splineWidget(const splineWidget&) = delete;
  void operator=(const splineWidget&) = delete;
};

#endif
