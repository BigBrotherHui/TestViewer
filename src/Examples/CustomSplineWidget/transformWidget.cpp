#include <vtkActor.h>
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkCommand.h>
#include <vtkObjectFactory.h>
#include <vtkPropCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkWidgetCallbackMapper.h>
#include <vtkWidgetEvent.h>

#include "transformWidget.h"
#include <vtkWidgetEventTranslator.h>
#include "vtkWidgetRepresentation.h"
#include <vtkRendererCollection.h>
#include "vtkHandleRepresentation.h"
#include <vtkRegularPolygonSource.h>
#include <vtkCaptionRepresentation.h>
#include <vtkCaptionActor2D.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkTriangle.h>
#include <vtkPointHandleRepresentation3D.h>
#include <QString>
#include <vtkTriangleFilter.h>
#include <vtkParametricSpline.h>
#include <vtkKochanekSpline.h>
#include <vtkParametricFunctionSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkMassProperties.h>
#include <vtkStripper.h>
#include <vtkDelaunay2D.h>
#include <vtkCleanPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkThinPlateSplineTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTransform.h>
#include "vtkAddonMathUtilities.h"
#include "vtkMessageCollection.h"
vtkStandardNewMacro(transformRepresentation);

vtkSmartPointer<vtkPolyData> CreateArrow()
{
    vtkSmartPointer<vtkPolyData> ret = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    double leftSide[3], rightSide[3];
    int offset = 5;
    double startPoint[3]{20, 0, 0};
    leftSide[0] = 0;
    leftSide[1] = startPoint[1] - offset;
    leftSide[2] = 0;
    rightSide[0] = 0;
    rightSide[1] = startPoint[1] + offset;
    rightSide[2] = 0;

    points->InsertNextPoint(startPoint);
    points->InsertNextPoint(leftSide);
    points->InsertNextPoint(rightSide);
    vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkTriangle> tri = vtkSmartPointer<vtkTriangle>::New();
    tri->GetPointIds()->SetId(0, 0);
    tri->GetPointIds()->SetId(1, 1);
    tri->GetPointIds()->SetId(2, 2);
    ret->SetPoints(points);
    cells->InsertNextCell(tri);
    ret->SetPolys(cells);
    return ret;
}

transformRepresentation::transformRepresentation()
{
}

void transformRepresentation::PlaceWidget(double bounds[6])
{
}

void transformRepresentation::StartWidgetInteraction(double e[2])
{
}

void transformRepresentation::WidgetInteraction(double e[2])
{
}

int transformRepresentation::ComputeInteractionState(int X, int Y, int modify)
{
    return outside;
}

void transformRepresentation::Highlight(int highlight)
{
}

void transformRepresentation::BuildRepresentation()
{
}

void transformRepresentation::GetActors(vtkPropCollection *pc)
{
}

vtkStandardNewMacro(transformWidget);

transformWidget::transformWidget()
{
    this->state_ = WIDGETSTATE::pickPoint;

    // define widget events
    {
        this->CallbackMapper->SetCallbackMethod(
            vtkCommand::LeftButtonPressEvent, vtkWidgetEvent::Select, this,
            transformWidget::SelectAction);

        this->CallbackMapper->SetCallbackMethod(
            vtkCommand::LeftButtonReleaseEvent, vtkWidgetEvent::EndSelect, this,
            transformWidget::EndSelectAction);
        this->CallbackMapper->SetCallbackMethod(vtkCommand::MiddleButtonPressEvent, vtkWidgetEvent::Translate, this,
                                                transformWidget::MoveSpline);
        this->CallbackMapper->SetCallbackMethod(vtkCommand::MiddleButtonReleaseEvent, vtkWidgetEvent::EndTranslate, this, transformWidget::MoveSplineDone);

        this->CallbackMapper->SetCallbackMethod(vtkCommand::MouseMoveEvent,
                                                vtkWidgetEvent::Move, this,
                                                transformWidget::MoveAction);

        this->keyEventCallbackCommand_ = vtkCallbackCommand::New();
        this->keyEventCallbackCommand_->SetClientData(this);
        this->keyEventCallbackCommand_->SetCallback(
            transformWidget::ProcessKeyEvents);
    }
}

transformWidget::~transformWidget()
{
    this->keyEventCallbackCommand_->Delete();
}

void transformWidget::SetEnabled(int enabling)
{
    const int enabled = this->Enabled;

    // We do this step first because it sets the CurrentRenderer
    vtkAbstractWidget::SetEnabled(enabling);
    if (enabling && !enabled)
    {
        this->CreateDefaultRepresentation();

        if (this->Parent)
        {
            this->Parent->AddObserver(vtkCommand::KeyPressEvent,
                                      this->keyEventCallbackCommand_,
                                      this->Priority);
            this->Parent->AddObserver(vtkCommand::KeyReleaseEvent,
                                      this->keyEventCallbackCommand_,
                                      this->Priority);
        }
        else
        {
            this->Interactor->AddObserver(vtkCommand::KeyPressEvent,
                                          this->keyEventCallbackCommand_,
                                          this->Priority);
            this->Interactor->AddObserver(vtkCommand::KeyReleaseEvent,
                                          this->keyEventCallbackCommand_,
                                          this->Priority);
        }

        // Add the actor
        {
            
        }
    }
    else if (!enabling && enabled)
    {
        if (this->Parent)
        {
            this->Parent->RemoveObserver(this->keyEventCallbackCommand_);
        }
        else
        {
            this->Interactor->RemoveObserver(this->keyEventCallbackCommand_);
        }

        // Remove the actor
        {
            
            
        }
    }
    vtkSmartPointer<vtkCaptionRepresentation> rep = vtkSmartPointer<vtkCaptionRepresentation>::New();
    captionWidget->SetRepresentation(rep);
    auto pro = vtkSmartPointer<vtkTextProperty>::New();
    pro->SetFontFamily(VTK_FONT_FILE);
    pro->SetFontFile("C:/Windows/Fonts/simhei.ttf");
    pro->SetJustificationToCentered();
    pro->SetVerticalJustificationToCentered();
    pro->SetFontSize(16);
    pro->SetColor(0.0, 1.0, 0.7);
    rep->GetCaptionActor2D()->SetCaptionTextProperty(pro);
    //rep->GetCaptionActor2D()->BorderOff();
    rep->GetCaptionActor2D()->SetCaption("");
    rep->GetCaptionActor2D()->SetLeaderGlyphData(CreateArrow());
    rep->GetAnchorRepresentation()->SetHandleSize(0);
    this->Interactor->Render();
}

void transformWidget::SetInteractor(vtkRenderWindowInteractor* iren)
{
    vtkAbstractWidget::SetInteractor(iren);
    captionWidget->SetInteractor(iren);
}

void transformWidget::CreateDefaultRepresentation()
{
    if (!this->WidgetRep) this->WidgetRep = transformRepresentation::New();
}

float transformWidget::calculateArea(vtkPoints* vp)
{
    if (!vp || vp->GetNumberOfPoints()<3) return 0.0;
    vtkSmartPointer<vtkPolyData> surface = vtkSmartPointer<vtkPolyData>::New();
    bool success = fitSurfaceProjectWarp(vp, surface);
    if (!success) {
        return 0.0;
    }
    if (surface->GetNumberOfPolys() == 0) return 0.0;
    vtkNew<vtkMassProperties> metrics;
    metrics->SetInputData(surface);
    double result= metrics->GetSurfaceArea();
    return result;
}

bool transformWidget::fitSurfaceProjectWarp(vtkPoints *curvePoints,
                                            vtkPolyData *surface,vtkIdType numberOfInternalGridPoints)
{
    if (!surface) {
        vtkGenericWarningMacro("FitSurfaceProjectWarp failed: invalid surface");
        return false;
    }

    if (!curvePoints) {
        vtkGenericWarningMacro("FitSurfaceProjectWarp failed: invalid curvePoints");
        surface->Initialize();
        return false;
    }

    // The triangulator needs a polygon, where the first and last points are different.
    // However, in the curve points, the first and last points are the same, therefore we remove the last point
    // by setting number of points to n-1.
    vtkIdType numberOfCurvePoints = curvePoints->GetNumberOfPoints() - 1;
    if (numberOfCurvePoints < 3) {
        // less than 3 points means that the surface is empty
        surface->Initialize();
        return true;
    }

    // Create a polydata containing a single polygon of the curve points
    vtkNew<vtkPolyData> inputSurface;
    inputSurface->SetPoints(curvePoints);
    vtkNew<vtkCellArray> polys;
    polys->InsertNextCell(numberOfCurvePoints);
    for (int i = 0; i < numberOfCurvePoints; i++) {
        polys->InsertCellPoint(i);
    }
    polys->Modified();
    inputSurface->SetPolys(polys);

    // Remove duplicate points (it would confuse the triangulator)
    vtkNew<vtkCleanPolyData> cleaner;
    cleaner->SetInputData(inputSurface);
    cleaner->Update();
    inputSurface->DeepCopy(cleaner->GetOutput());
    vtkNew<vtkPoints> cleanedCurvePoints;
    cleanedCurvePoints->DeepCopy(inputSurface->GetPoints());
    numberOfCurvePoints = cleanedCurvePoints->GetNumberOfPoints();

    // The triangulator requires all points to be on the XY plane
    vtkNew<vtkMatrix4x4> transformToBestFitPlaneMatrix;
    if (!vtkAddonMathUtilities::FitPlaneToPoints(inputSurface->GetPoints(), transformToBestFitPlaneMatrix)) {
        surface->Initialize();
        return false;
    }
    vtkNew<vtkTransform> transformToXYPlane;
    transformToXYPlane->SetMatrix(transformToBestFitPlaneMatrix);  // set XY plane -> best-fit plane
    transformToXYPlane->Inverse();  // // change the transform to: set best-fit plane -> XY plane
    vtkNew<vtkPoints> pointsOnPlane;
    transformToXYPlane->TransformPoints(cleanedCurvePoints, pointsOnPlane);
    inputSurface->SetPoints(pointsOnPlane);
    for (vtkIdType i = 0; i < numberOfCurvePoints; i++) {
        double *pt = pointsOnPlane->GetPoint(i);
        pointsOnPlane->SetPoint(i, pt[0], pt[1], 0.0);
    }

    // Ensure points are in counter-clockwise direction
    // (that indicates to Delaunay2D that it is a polygon to be
    // filled in and not a hole).
    vtkNew<vtkIdList> cleanedCurvePointIds;
    polys->GetCell(0, cleanedCurvePointIds);
    if (IsPolygonClockwise(pointsOnPlane, cleanedCurvePointIds)) {
        vtkIdType numberOfCleanedCurvePointIds = cleanedCurvePointIds->GetNumberOfIds();
        vtkNew<vtkCellArray> reversePolys;
        reversePolys->InsertNextCell(numberOfCleanedCurvePointIds);
        for (vtkIdType i = 0; i < numberOfCleanedCurvePointIds; i++) {
            reversePolys->InsertCellPoint(cleanedCurvePointIds->GetId(numberOfCleanedCurvePointIds - 1 - i));
        }
        reversePolys->Modified();
        inputSurface->SetPolys(reversePolys);
    }

    // Add set of internal points to improve triangulation quality.
    // We already have many points on the boundary but no points inside the polygon.
    // If we passed these points to the triangulator then opposite points on the curve
    // would be connected by triangles, so we would end up with many very skinny triangles,
    // and not smooth surface after warping.
    // By adding random points, the triangulator can create evenly sized triangles.
    double bounds[6] = {0.0};
    pointsOnPlane->GetBounds(bounds);
    double width = bounds[1] - bounds[0];
    double height = bounds[3] - bounds[2];
    // Compute the number of grid points along the two axes from these:
    // 1.  rows * cols = numberOfInternalGridPoints
    // 2.  rows/cols = height/width
    vtkIdType rows = 1;
    if (height > width / numberOfInternalGridPoints) {
        rows = static_cast<vtkIdType>(sqrt(numberOfInternalGridPoints * height / width));
        if (rows > numberOfInternalGridPoints) {
            rows = numberOfInternalGridPoints;
        }
    }
    vtkIdType cols = numberOfInternalGridPoints / rows;
    if (cols < 1) {
        cols = 1;
    }
    double colSpacing = width / cols;
    double rowSpacing = height / rows;
    double colStart = bounds[0] + 0.5 * colSpacing;
    double rowStart = bounds[2] + 0.5 * rowSpacing;
    for (vtkIdType row = 0; row < rows; row++) {
        for (vtkIdType col = 0; col < cols; col++) {
            pointsOnPlane->InsertNextPoint(colStart + colSpacing * col, rowStart + rowSpacing * row, 0.0);
        }
    }

    vtkNew<vtkDelaunay2D> triangulator;
    triangulator->SetInputData(inputSurface);
    triangulator->SetSourceData(inputSurface);

    vtkNew<vtkMessageCollection> messages;
    messages->SetObservedObject(triangulator);
    triangulator->Update();

    bool errorFound = false;
    bool warningFound = false;
    std::string messageStr = messages->GetAllMessagesAsString(&errorFound, &warningFound);
    if (errorFound || warningFound) {
        vtkGenericWarningMacro(
            "FitSurfaceProjectWarp failed: error triangulating the surface area of the closed curve. Details: "
            << messageStr);
        surface->Initialize();
        return false;
    }

    vtkPolyData *triangulatedSurface = triangulator->GetOutput();
    vtkPoints *triangulatedSurfacePoints = triangulatedSurface->GetPoints();

    vtkNew<vtkPoints> sourceLandmarkPoints;  // points on the triangulated surface
    vtkNew<vtkPoints> targetLandmarkPoints;  // points on the curve
    // Use only the transformed curve points (first numberOfCurvePoints points)
    // We use only every 3rd boundary point as warping transform control point for simpler and faster warping.
    int step = 3;
    vtkIdType numberOfRegistrationLandmarkPoints = numberOfCurvePoints / step;
    sourceLandmarkPoints->SetNumberOfPoints(numberOfRegistrationLandmarkPoints);
    targetLandmarkPoints->SetNumberOfPoints(numberOfRegistrationLandmarkPoints);
    for (vtkIdType landmarkPointIndex = 0; landmarkPointIndex < numberOfRegistrationLandmarkPoints;
         landmarkPointIndex++) {
        sourceLandmarkPoints->SetPoint(landmarkPointIndex,
                                       triangulatedSurfacePoints->GetPoint(landmarkPointIndex * step));
        targetLandmarkPoints->SetPoint(landmarkPointIndex, cleanedCurvePoints->GetPoint(landmarkPointIndex * step));
    }

    vtkNew<vtkThinPlateSplineTransform> landmarkTransform;
    // Disable regularization to make sure transformation is correct even if source or target points are coplanar
    landmarkTransform->SetRegularizeBulkTransform(false);
    landmarkTransform->SetBasisToR();
    landmarkTransform->SetSourceLandmarks(sourceLandmarkPoints);
    landmarkTransform->SetTargetLandmarks(targetLandmarkPoints);

    vtkNew<vtkTransformPolyDataFilter> polyTransformToCurve;
    polyTransformToCurve->SetTransform(landmarkTransform);
    polyTransformToCurve->SetInputData(triangulatedSurface);

    vtkNew<vtkPolyDataNormals> polyDataNormals;
    polyDataNormals->SetInputConnection(polyTransformToCurve->GetOutputPort());
    polyDataNormals->SplittingOff();
    polyDataNormals->Update();

    surface->DeepCopy(polyDataNormals->GetOutput());
    return true;
}

bool transformWidget::IsPolygonClockwise(vtkPoints* points, vtkIdList* pointIds)
{
    if (!points) {
        return false;
    }
    vtkIdType numberOfPoints = (pointIds != nullptr ? pointIds->GetNumberOfIds() : points->GetNumberOfPoints());
    if (numberOfPoints < 3) {
        return false;
    }

    // Find the bottom-left point (it is on the convex hull) of the polygon,
    // and check sign of cross-product of the edges before and after that point.
    // (https://en.wikipedia.org/wiki/Curve_orientation#Orientation_of_a_simple_polygon)

    double *point0 = points->GetPoint(0);
    double minX = point0[0];
    double minY = point0[1];
    vtkIdType cornerPointIndex = 0;
    for (vtkIdType i = 1; i < numberOfPoints; i++) {
        vtkIdType pointId = (pointIds != nullptr ? pointIds->GetId(i) : i);
        double *p = points->GetPoint(pointId);
        if ((p[1] < minY) || ((p[1] == minY) && (p[0] < minX))) {
            cornerPointIndex = i;
            minX = p[0];
            minY = p[1];
        }
    }

    double p1[3];
    double p2[3];
    double p3[3];
    if (pointIds != nullptr) {
        points->GetPoint(pointIds->GetId((cornerPointIndex - 1 + numberOfPoints) % numberOfPoints), p1);
        points->GetPoint(pointIds->GetId(cornerPointIndex), p2);
        points->GetPoint(pointIds->GetId((cornerPointIndex + 1) % numberOfPoints), p3);
    }
    else {
        points->GetPoint((cornerPointIndex - 1 + numberOfPoints) % numberOfPoints, p1);
        points->GetPoint(cornerPointIndex, p2);
        points->GetPoint((cornerPointIndex + 1) % numberOfPoints, p3);
    }
    double det = p2[0] * p3[1] - p2[1] * p3[0] - p1[0] * p3[1] + p1[0] * p2[1] + p1[1] * p3[0] - p1[1] * p2[0];
    return (det < 0);
}

void transformWidget::updateLines(bool calculate)
{
    if (m_controlPoints.size() < 2) return;
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    for (int i = 0; i < m_controlPoints.size(); ++i) {
        double pos[3];
        static_cast<vtkHandleRepresentation *>(m_controlPoints[i]->GetRepresentation())->GetWorldPosition(pos);
        points->InsertNextPoint(pos);
    }
    if (m_controlPoints.size() > 2) {
        double pos[3];
        static_cast<vtkHandleRepresentation *>(m_controlPoints[0]->GetRepresentation())->GetWorldPosition(pos);
        points->InsertNextPoint(pos);
    }
    vtkNew<vtkKochanekSpline> xSpline;
    vtkNew<vtkKochanekSpline> ySpline;
    vtkNew<vtkKochanekSpline> zSpline;

    vtkNew<vtkParametricSpline> spline;
    spline->SetXSpline(xSpline);
    spline->SetYSpline(ySpline);
    spline->SetZSpline(zSpline);
    spline->SetPoints(points);

    int numberOfPoints = 10;

    vtkNew<vtkParametricFunctionSource> functionSource;
    functionSource->SetParametricFunction(spline);
    functionSource->SetUResolution(50 * numberOfPoints);
    functionSource->SetVResolution(50 * numberOfPoints);
    functionSource->SetWResolution(50 * numberOfPoints);
    functionSource->Update();
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(functionSource->GetOutput());
    if (points->GetNumberOfPoints()==4) {
        double *pos = functionSource->GetOutput()->GetCenter();
        auto captionRepresentation = captionWidget->GetRepresentation();
        static_cast<vtkCaptionRepresentation *>(captionRepresentation)->SetAnchorPosition(pos);
        captionWidget->SetEnabled(1);
    }
    if (points->GetNumberOfPoints() > 2 && calculate) {
        float area=calculateArea(points);
        auto pts = functionSource->GetOutput()->GetPoints();
        int npts = pts->GetNumberOfPoints();
        double a[3];
        double b[3];
        double sum = 0.0;
        int i = 0;
        pts->GetPoint(i, a);
        int imax = (npts % 2 == 0) ? npts - 2 : npts - 1;

        while (i < imax) {
            pts->GetPoint(i + 1, b);
            sum += sqrt(vtkMath::Distance2BetweenPoints(a, b));
            i = i + 2;
            pts->GetPoint(i, a);
            sum = sum + sqrt(vtkMath::Distance2BetweenPoints(a, b));
        }

        if (npts % 2 == 0) {
            pts->GetPoint(i + 1, b);
            sum += sqrt(vtkMath::Distance2BetweenPoints(a, b));
        }
        static_cast<vtkCaptionRepresentation *>(captionWidget->GetRepresentation())
            ->GetCaptionActor2D()
            ->SetCaption(QString::fromLocal8Bit("面积：%1\n长度：%2").arg(area).arg(sum).toUtf8());
    }

    if (!isadded)
    {
        isadded = 1;
        actor->SetMapper(mapper);
        actor->GetProperty()->SetLineWidth(1.0);
        this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(actor);    
    }
    else {
        static_cast<vtkPolyDataMapper *>(actor->GetMapper())->SetInputData(functionSource->GetOutput());
    }
}

void transformWidget::PrintSelf(ostream &os, vtkIndent indent)
{
    vtkAbstractWidget::PrintSelf(os, indent);
}
#include <vtkPointHandleRepresentation3D.h>
void transformWidget::SelectAction(vtkAbstractWidget *w)
{
    auto self = reinterpret_cast<transformWidget *>(w);

    const int x = self->Interactor->GetEventPosition()[0];
    const int y = self->Interactor->GetEventPosition()[1];
    self->GrabFocus(self->EventCallbackCommand);
    for (int i = 0; i < self->m_controlPoints.size(); ++i) {
        int st=self->m_controlPoints[i]->GetRepresentation()->ComputeInteractionState(x, y);
        if (st == vtkHandleRepresentation::Nearby) {
            self->state_ = transformWidget::WIDGETSTATE::interactivePoint;
            self->m_interactiveControlPoint = self->m_controlPoints[i];
            break;
        }
    }
    if (self->state_ == transformWidget::WIDGETSTATE::interactivePoint) {
        self->ReleaseFocus();
        self->EventCallbackCommand->SetAbortFlag(1);
        return;
    }
    double eventPos[2];
    eventPos[0] = static_cast<double>(x);
    eventPos[1] = static_cast<double>(y);
    double disp[3]{eventPos[0], eventPos[1], 0};
    vtkHandleWidget *controlPoint = vtkHandleWidget::New();
    self->m_controlPoints.push_back(controlPoint);
    controlPoint->SetInteractor(self->Interactor);
    controlPoint->SetEnabled(1);
    vtkNew<vtkRegularPolygonSource> source;
    source->SetNumberOfSides(70);
    static_cast<vtkHandleRepresentation *>(controlPoint->GetRepresentation())->SetDisplayPosition(disp);
    static_cast<vtkHandleRepresentation *>(controlPoint->GetRepresentation())->SetHandleSize(5);
    self->EventCallbackCommand->SetAbortFlag(1);

    self->StartInteraction();
    self->InvokeEvent(vtkCommand::StartInteractionEvent, nullptr);
    self->updateLines();
    self->Render();
}

void transformWidget::EndSelectAction(vtkAbstractWidget *w)
{
    auto self = reinterpret_cast<transformWidget *>(w);

    // Return state to not active
    self->state_ = transformWidget::WIDGETSTATE::pickPoint;
    self->ReleaseFocus();
    self->InvokeEvent(vtkCommand::LeftButtonReleaseEvent,
                      nullptr);  // handles observe this
    self->EventCallbackCommand->SetAbortFlag(1);
    self->InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
    self->EndInteraction();
    self->Render();
}

void transformWidget::MoveSpline(vtkAbstractWidget* w)
{
    auto self = reinterpret_cast<transformWidget *>(w);
    const int x = self->Interactor->GetEventPosition()[0];
    const int y = self->Interactor->GetEventPosition()[1];
    self->GrabFocus(self->EventCallbackCommand);
    if (self->captionWidget->GetEnabled() &&
        self->captionWidget->GetRepresentation()->ComputeInteractionState(x, y) == vtkCaptionRepresentation::Inside) {
        self->state_ = transformWidget::WIDGETSTATE::interactiveCaption;
        static_cast<vtkCaptionRepresentation *>(self->captionWidget->GetRepresentation())->MovingOn();
        double XF = static_cast<double>(x);
        double YF = static_cast<double>(y);
        self->CurrentRenderer->DisplayToNormalizedDisplay(XF, YF);
        self->CurrentRenderer->NormalizedDisplayToViewport(XF, YF);
        self->CurrentRenderer->ViewportToNormalizedViewport(XF, YF);
        double eventPos[2];
        eventPos[0] = XF;
        eventPos[1] = YF;
        static_cast<vtkCaptionRepresentation *>(self->captionWidget->GetRepresentation())
            ->StartWidgetInteraction(eventPos);
    }else
    {
        self->state_ = dragSpline;
            
    }
    self->ReleaseFocus();
    self->EventCallbackCommand->SetAbortFlag(1);

    self->StartInteraction();
    self->InvokeEvent(vtkCommand::StartInteractionEvent, nullptr);
    self->Render();
}

void transformWidget::MoveSplineDone(vtkAbstractWidget* w)
{
    auto self = reinterpret_cast<transformWidget *>(w);
    const int x = self->Interactor->GetEventPosition()[0];
    const int y = self->Interactor->GetEventPosition()[1];  // Return state to not active
    self->state_ = transformWidget::WIDGETSTATE::pickPoint;
    if (self->state_ == interactiveCaption) {
        static_cast<vtkCaptionRepresentation *>(self->captionWidget->GetRepresentation())->MovingOff();
    }
    self->ReleaseFocus();
    self->InvokeEvent(vtkCommand::MiddleButtonReleaseEvent,
                      nullptr);  // handles observe this
    self->EventCallbackCommand->SetAbortFlag(1);
    self->InvokeEvent(vtkCommand::EndInteractionEvent, nullptr);
    self->EndInteraction();
    self->Render();
}

void transformWidget::MoveAction(vtkAbstractWidget *w)
{
    auto self = reinterpret_cast<transformWidget *>(w);

    const int x = self->Interactor->GetEventPosition()[0];
    const int y = self->Interactor->GetEventPosition()[1];
    if (self->state_ == transformWidget::WIDGETSTATE::pickPoint)  // if is not active
    {
        self->Interactor->Disable();  // avoid extra renders
        self->Interactor->Enable();  // avoid extra renders
    }
    else  // if is active
    {
        double e[2];
        e[0] = static_cast<double>(x);
        e[1] = static_cast<double>(y);

        if (self->m_interactiveControlPoint && self->state_==interactivePoint)
        {
            double disp[3]{e[0], e[1], 0};
            static_cast<vtkHandleRepresentation *>(self->m_interactiveControlPoint->GetRepresentation())
                ->SetDisplayPosition(disp);
            self->updateLines();
    
        }
        int *lastev = self->Interactor->GetLastEventPosition();
        double diff[3]{e[0] - lastev[0], e[1] - lastev[1], 0};
        double disp[3];
        if (self->state_ == dragSpline) {
            for (int i = 0; i < self->m_controlPoints.size(); ++i) {
                static_cast<vtkHandleRepresentation *>(self->m_controlPoints[i]->GetRepresentation())->GetDisplayPosition(disp);
                for (int m = 0; m < 3; ++m) {
                    disp[m] += diff[m];
                }
                static_cast<vtkHandleRepresentation *>(self->m_controlPoints[i]->GetRepresentation())
                    ->SetDisplayPosition(disp);
            }
            self->updateLines(false);
        }
        else if (self->state_==interactiveCaption)
        {
            static_cast<vtkCaptionRepresentation *>(self->captionWidget->GetRepresentation())
                ->MovingOn();
            static_cast<vtkCaptionRepresentation *>(self->captionWidget->GetRepresentation())->WidgetInteraction(e);
        }
        self->EventCallbackCommand->SetAbortFlag(1);
        self->InvokeEvent(vtkCommand::InteractionEvent, nullptr);
        self->Render();
    }
}

void transformWidget::ProcessKeyEvents(vtkObject *, unsigned long, void *,
                                       void *)
{
}
