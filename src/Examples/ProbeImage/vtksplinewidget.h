

#ifndef vtksplinewidget_h
#define vtksplinewidget_h

#include "vtk3DWidget.h"
#include "vtkInteractionWidgetsModule.h" // For export macro

class vtkActor;
class vtkCellPicker;
class vtkParametricSpline;
class vtkParametricFunctionSource;
class vtkPlaneSource;
class vtkPoints;
class vtkPolyData;
class vtkProp;
class vtkProperty;
class vtkSphereSource;
class vtkTransform;
class vtkRegularPolygonSource;
#define VTK_PROJECTION_YZ 0
#define VTK_PROJECTION_XZ 1
#define VTK_PROJECTION_XY 2
#define VTK_PROJECTION_OBLIQUE 3

class vtksplinewidget : public vtk3DWidget
{
public:
  /**
   * Instantiate the object.
   */
  static vtksplinewidget* New();

  vtkTypeMacro(vtksplinewidget, vtk3DWidget);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///@{
  /**
   * Methods that satisfy the superclass' API.
   */
  void SetEnabled(int) override;
  void PlaceWidget(double bounds[6]) override;
  void PlaceWidget() override { this->Superclass::PlaceWidget(); }
  void PlaceWidget(
    double xmin, double xmax, double ymin, double ymax, double zmin, double zmax) override
  {
    this->Superclass::PlaceWidget(xmin, xmax, ymin, ymax, zmin, zmax);
  }
  ///@}

  ///@{
  /**
   * Force the spline widget to be projected onto one of the orthogonal planes.
   * Remember that when the state changes, a ModifiedEvent is invoked.
   * This can be used to snap the spline to the plane if it is originally
   * not aligned.  The normal in SetProjectionNormal is 0,1,2 for YZ,XZ,XY
   * planes respectively and 3 for arbitrary oblique planes when the widget
   * is tied to a vtkPlaneSource.
   */
  vtkSetMacro(ProjectToPlane, vtkTypeBool);
  vtkGetMacro(ProjectToPlane, vtkTypeBool);
  vtkBooleanMacro(ProjectToPlane, vtkTypeBool);
  ///@}

  /**
   * Set up a reference to a vtkPlaneSource that could be from another widget
   * object, e.g. a vtkPolyDataSourceWidget.
   */
  void SetPlaneSource(vtkPlaneSource* plane);

  vtkSetClampMacro(ProjectionNormal, int, VTK_PROJECTION_YZ, VTK_PROJECTION_OBLIQUE);
  vtkGetMacro(ProjectionNormal, int);
  void SetProjectionNormalToXAxes() { this->SetProjectionNormal(0); }
  void SetProjectionNormalToYAxes() { this->SetProjectionNormal(1); }
  void SetProjectionNormalToZAxes() { this->SetProjectionNormal(2); }
  void SetProjectionNormalToOblique() { this->SetProjectionNormal(3); }

  ///@{
  /**
   * Set the position of spline handles and points in terms of a plane's
   * position. i.e., if ProjectionNormal is 0, all of the x-coordinate
   * values of the points are set to position. Any value can be passed (and is
   * ignored) to update the spline points when Projection normal is set to 3
   * for arbitrary plane orientations.
   */
  void SetProjectionPosition(double position);
  vtkGetMacro(ProjectionPosition, double);
  ///@}

  /**
   * Grab the polydata (including points) that defines the spline.  The
   * polydata consists of points and line segments numbering Resolution + 1
   * and Resoltuion, respectively. Points are guaranteed to be up-to-date when
   * either the InteractionEvent or EndInteraction events are invoked. The
   * user provides the vtkPolyData and the points and polyline are added to it.
   */
  void GetPolyData(vtkPolyData* pd);

  ///@{
  /**
   * Set/Get the handle properties (the spheres are the handles). The
   * properties of the handles when selected and unselected can be manipulated.
   */
  virtual void SetHandleProperty(vtkProperty*);
  vtkGetObjectMacro(HandleProperty, vtkProperty);
  virtual void SetSelectedHandleProperty(vtkProperty*);
  vtkGetObjectMacro(SelectedHandleProperty, vtkProperty);
  ///@}

  ///@{
  /**
   * Set/Get the line properties. The properties of the line when selected
   * and unselected can be manipulated.
   */
  virtual void SetLineProperty(vtkProperty*);
  vtkGetObjectMacro(LineProperty, vtkProperty);
  virtual void SetSelectedLineProperty(vtkProperty*);
  vtkGetObjectMacro(SelectedLineProperty, vtkProperty);
  ///@}

  ///@{
  /**
   * Set/Get the number of handles for this widget.
   */
  virtual void SetNumberOfHandles(int npts);
  vtkGetMacro(NumberOfHandles, int);
  ///@}

  ///@{
  /**
   * Set/Get the number of line segments representing the spline for
   * this widget.
   */
  void SetResolution(int resolution);
  vtkGetMacro(Resolution, int);
  ///@}

  ///@{
  /**
   * Set the parametric spline object. Through vtkParametricSpline's API, the
   * user can supply and configure one of currently two types of spline:
   * vtkCardinalSpline, vtkKochanekSpline. The widget controls the open
   * or closed configuration of the spline.
   * WARNING: The widget does not enforce internal consistency so that all
   * three are of the same type.
   */
  virtual void SetParametricSpline(vtkParametricSpline*);
  vtkGetObjectMacro(ParametricSpline, vtkParametricSpline);
  ///@}

  ///@{
  /**
   * Set/Get the position of the spline handles. Call GetNumberOfHandles
   * to determine the valid range of handle indices.
   */
  void SetHandlePosition(int handle, double x, double y, double z);
  void SetHandlePosition(int handle, double xyz[3]);
  void GetHandlePosition(int handle, double xyz[3]);
  double* GetHandlePosition(int handle) VTK_SIZEHINT(3);
  ///@}

  ///@{
  /**
   * Control whether the spline is open or closed. A closed spline forms
   * a continuous loop: the first and last points are the same, and
   * derivatives are continuous.  A minimum of 3 handles are required to
   * form a closed loop.  This method enforces consistency with
   * user supplied subclasses of vtkSpline.
   */
  void SetClosed(vtkTypeBool closed);
  vtkGetMacro(Closed, vtkTypeBool);
  vtkBooleanMacro(Closed, vtkTypeBool);
  ///@}

  /**
   * Convenience method to determine whether the spline is
   * closed in a geometric sense.  The widget may be set "closed" but still
   * be geometrically open (e.g., a straight line).
   */
  int IsClosed();

  /**
   * Get the approximate vs. the true arc length of the spline. Calculated as
   * the summed lengths of the individual straight line segments. Use
   * SetResolution to control the accuracy.
   */
  double GetSummedLength();

  /**
   * Convenience method to allocate and set the handles from a vtkPoints
   * instance.  If the first and last points are the same, the spline sets
   * Closed to the on state and disregards the last point, otherwise Closed
   * remains unchanged.
   */
  void InitializeHandles(vtkPoints* points);

  ///@{
  /**
   * Turn on / off event processing for this widget. If off, the widget will
   * not respond to user interaction
   */
  vtkSetClampMacro(ProcessEvents, vtkTypeBool, 0, 1);
  vtkGetMacro(ProcessEvents, vtkTypeBool);
  vtkBooleanMacro(ProcessEvents, vtkTypeBool);
  ///@}

protected:
  vtksplinewidget();
  ~vtksplinewidget() override;

  // Manage the state of the widget
  int State;
  enum WidgetState
  {
    Start = 0,
    Moving,
    Scaling,
    Spinning,
    Inserting,
    Erasing,
    Outside
  };

  // handles the events
  static void ProcessEventsHandler(
    vtkObject* object, unsigned long event, void* clientdata, void* calldata);

  // ProcessEventsHandler() dispatches to these methods.
  void OnLeftButtonDown();
  void OnLeftButtonUp();
  void OnMiddleButtonDown();
  void OnMiddleButtonUp();
  void OnRightButtonDown();
  void OnRightButtonUp();
  void OnMouseMove();

  // Controlling vars
  int ProjectionNormal;
  double ProjectionPosition;
  vtkTypeBool ProjectToPlane;
  vtkPlaneSource* PlaneSource;

  // Projection capabilities
  void ProjectPointsToPlane();
  void ProjectPointsToOrthoPlane();
  void ProjectPointsToObliquePlane();

  // The spline
  vtkParametricSpline* ParametricSpline;
  vtkParametricFunctionSource* ParametricFunctionSource;
  int NumberOfHandles;
  vtkTypeBool Closed;
  void BuildRepresentation();

  // The line segments
  vtkActor* LineActor;
  void HighlightLine(int highlight);
  int Resolution;

  // Glyphs representing hot spots (e.g., handles)
  vtkActor** Handle;

  vtkRegularPolygonSource** HandleGeometry;
  void Initialize();
  int HighlightHandle(vtkProp* prop); // returns handle index or -1 on fail
  void SizeHandles() override;
  void InsertHandleOnLine(double* pos);
  void EraseHandle(const int&);

  // Do the picking
  vtkCellPicker* HandlePicker;
  vtkCellPicker* LinePicker;
  vtkActor* CurrentHandle;
  int CurrentHandleIndex;

  // Register internal Pickers within PickingManager
  void RegisterPickers() override;

  // Methods to manipulate the spline.
  void MovePoint(double* p1, double* p2);
  void Scale(double* p1, double* p2, int X, int Y);
  void Translate(double* p1, double* p2);
  void Spin(double* p1, double* p2, double* vpn);

  // Transform the control points (used for spinning)
  vtkTransform* Transform;

  // Properties used to control the appearance of selected objects and
  // the manipulator in general.
  vtkProperty* HandleProperty;
  vtkProperty* SelectedHandleProperty;
  vtkProperty* LineProperty;
  vtkProperty* SelectedLineProperty;
  void CreateDefaultProperties();

  // For efficient spinning
  double Centroid[3];
  void CalculateCentroid();
  vtkTypeBool ProcessEvents;

private:
  vtksplinewidget(const vtksplinewidget&) = delete;
  void operator=(const vtksplinewidget&) = delete;
};

#endif
