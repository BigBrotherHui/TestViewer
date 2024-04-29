#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QVTKOpenGLNativeWidget.h>
#include <QVBoxLayout>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkSphereSource.h>
#include <vtkRenderer.h>
#include <vtkInteractorStyleRubberBand3D.h>
#include <vtkSTLReader.h>
#include <vtkInteractorStyleDrawPolygon.h>
#include <vtkProperty.h>
#include <vtkDataSetMapper.h>
#include <vtkCellPicker.h>
#include <vtkRendererCollection.h>
#include <vtkPolyLine.h>
#include <vtkPointPicker.h>
#define VTK_CREATE(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()
class PointPickerInteractorStyle1 : public vtkInteractorStyleTrackballCamera {
public:
    static PointPickerInteractorStyle1 *New();
    vtkTypeMacro(PointPickerInteractorStyle1, vtkInteractorStyleTrackballCamera);
    virtual void OnLeftButtonDown()
    {
        isLeftMousePressed = 1;
        m_points = vtkSmartPointer<vtkPoints>::New();
        cells = vtkSmartPointer<vtkCellArray>::New();
        // ע��ʰȡ�㺯��
        this->Interactor->GetPicker()->Pick(
            this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1],
            0,  //��Ҫ��ȡ���Ƕ�άƽ������ص�����
            this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()  //��ȡvtkrender����
        );
        screenPoints.push_back(vtkVector2i(this->Interactor->GetEventPosition()));
        // ��ӡʰȡ��ռ�λ��
        double picked[3];
        this->Interactor->GetPicker()->GetPickPosition(picked);
        m_points->InsertNextPoint(picked[0], picked[1], picked[2]);
        polyLine->GetPointIds()->SetNumberOfIds(m_points->GetNumberOfPoints());
        for (int i = 0; i < m_points->GetNumberOfPoints(); i++) {
            polyLine->GetPointIds()->SetId(i, i);
        };
        cells->InsertNextCell(polyLine);
        outputVector->SetPoints(m_points);
        outputVector->SetLines(cells);
        vtkSmartPointer<vtkPolyDataMapper> LineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        LineMapper->SetInputData(outputVector);
        vtkSmartPointer<vtkActor> LineActor = vtkSmartPointer<vtkActor>::New();
        LineActor->SetMapper(LineMapper);
        LineActor->GetProperty()->SetColor(1, 0, 0);
        LineActor->GetProperty()->SetLineWidth(2);
        this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(LineActor);
        this->Interactor->Render();
        // return vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
    }
    virtual void OnLeftButtonUp()
    {
        isLeftMousePressed = false;
        InvokeEvent(vtkCommand::SelectionChangedEvent);
        screenPoints.clear();
        // return vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
    }
    virtual void OnMouseMove()
    {
        if (isLeftMousePressed) {
            this->Interactor->GetPicker()->Pick(
                this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1], 0,
                this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
            double picked[3];
            screenPoints.push_back(vtkVector2i(this->Interactor->GetEventPosition()));

            this->Interactor->GetPicker()->GetPickPosition(picked);
            m_points->InsertNextPoint(picked[0], picked[1], picked[2]);
            int numOfPoints = m_points->GetNumberOfPoints();
            polyLine->GetPointIds()->InsertNextId(numOfPoints);
            polyLine->GetPointIds()->SetId(numOfPoints - 1, numOfPoints - 1);
            m_points->Modified();
            cells->Initialize();              // reset the cells to remove the old spiral
            cells->InsertNextCell(polyLine);  // re-insert the updated spiral
            cells->Modified();                // required to update
            outputVector->Modified();
            // renderer->GetRenderWindow()->GetInteractor()->Initialize();
            //  renderer->Render();
            this->Interactor->GetRenderWindow()->Render();
            // return vtkInteractorStyleTrackballCamera::OnMouseMove();
        }
    };
    std::vector<vtkVector2i> screenPoints;

private:
    vtkSmartPointer<vtkPoints> m_points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
    vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
    vtkSmartPointer<vtkPolyData> outputVector = vtkSmartPointer<vtkPolyData>::New();
    bool isLeftMousePressed{false};
};

vtkStandardNewMacro(PointPickerInteractorStyle1) 

typedef struct {
public:
    double x, y, z;
} POLYGON_VERTEX;
class POLYGON_LIST {
public:
    std::vector<vtkVector2i> point_list;
    int point_number;
    int state;  // state 0 = not finished or not valid; state 1 =closed and valide

    POLYGON_LIST()
    {
        this->point_list.clear();
        this->point_number = 0;
        this->state = 0;
    }
    int POLYGON_POINT_INSIDE(POLYGON_VERTEX P)
    {
        int cn = 0;  // the crossing number counter

        // loop through all edges of the polygon
        for (int i = 0; i < this->point_list.size() - 1; i++) {  // edge from V[i] to V[i+1]
            const vtkVector2i &V1 = point_list[i];
            const vtkVector2i &V2 = point_list[i + 1];
            if (((V1[1] <= P.y) && (V2[1] > P.y))        // an upward crossing
                || ((V1[1] > P.y) && (V2[1] <= P.y))) {  // a downward crossing
                                                         // compute the actual edge-ray intersect x-coordinate
                double vt = (double)(P.y - V1[1]) / (V2[1] - V1[1]);
                if (P.x < V1[0] + vt * (V2[0] - V1[0]))  // P.x < intersect
                    ++cn;                                // a valid crossing of y=P.y right of P.x
            }
        }
        // last edge
        const vtkVector2i &F = point_list[0];                 // first point
        const vtkVector2i &L = point_list[point_number - 1];  // last point

        if (((F[1] <= P.y) && (L[1] > P.y))        // an upward crossing
            || ((F[1] > P.y) && (L[1] <= P.y))) {  // a downward crossing
                                                   // compute the actual edge-ray intersect x-coordinate
            double vt = (double)(P.y - F[1]) / (L[1] - F[1]);
            if (P.x < F[0] + vt * (L[0] - F[0]))  // P.x < intersect
                ++cn;                             // a valid crossing of y=P.y right of P.x
        }
        return (cn & 1);  // 0 if even (out), and 1 if odd (in)
    }
    void SetPointList(std::vector<vtkVector2i> points)
    {
        this->point_list = points;
        this->point_number = (int)points.size();
        this->state = 0;
        this->state = this->Polygon_valid();
    }

    void Polygon_init()
    {
        this->point_number = 0;
        this->state = 0;
    }
    int Poly_cross(double x1, double y1, double x2, double y2, double u1, double u2, double v1, double v2)
    {
        double b1;
        double b2;
        double xi, yi;
        double a1, a2;
        if ((x2 - x1 == 0) || (u2 - u1 == 0)) {
            return 0;
        }  // Cases were one edge is vertical are avoided...(A and B exceptions)

        b1 = (y2 - y1) / (x2 - x1);  //(A)
        b2 = (v2 - v1) / (u2 - u1);  // (B)

        if (b2 - b1 == 0) {
            return 0;
        }  // Cases were two edges are parallel are avoided. (C exception)

        a1 = y1 - b1 * x1;
        a2 = v1 - b2 * u1;
        xi = -(a1 - a2) / (b1 - b2);  //(C)
        yi = a1 + b1 * xi;
        if (((x1 - xi) * (xi - x2) >= 0) && ((u1 - xi) * (xi - u2) >= 0) && ((y1 - yi) * (yi - y2) >= 0) &&
            ((v1 - yi) * (yi - v2) >= 0)) {
            return 1;
        }
        else {
            return 0;
        }
    }
    int Polygon_valid()  // Allocate Memory For Each Object
    {
        int x1, x2, y1, y2, u1, u2, v1, v2;
        // And Defines points

        int valid = 1;

        if (this->point_number < 3) {
            return 0;
        }
        else {  // return 1;
            if (this->point_number == 3) {
                const vtkVector2i &A = point_list[0];
                const vtkVector2i &B = point_list[1];
                const vtkVector2i &C = point_list[2];

                x1 = (int)A[0];
                x2 = (int)B[0];
                y1 = (int)A[1];
                y2 = (int)B[1];
                u1 = (int)B[0];
                u2 = (int)C[0];
                v1 = (int)B[1];
                v2 = (int)C[1];
                if (Poly_cross((double)x1, (double)y1, (double)x2, (double)y2, (double)u1, (double)u2, (double)v1,
                               (double)v2)) {
                    valid = 0;
                }
                // test if paralleles
            }
            else {
                int i;
                for (i = 0; i < point_number - 1; i++) {
                    const vtkVector2i &V1 = point_list[i];
                    const vtkVector2i &V2 = point_list[i + 1];
                    x1 = (int)V1[0];
                    x2 = (int)V2[0];
                    y1 = (int)V1[1];
                    y2 = (int)V2[1];
                    for (int j = i + 2; j < point_number - 1; j++) {
                        const vtkVector2i &U1 = point_list[j];
                        const vtkVector2i &U2 = point_list[j + 1];
                        // fprintf(stderr, "Segment %d - %d\n", i + 1, j + 1);
                        u1 = (int)U1[0];
                        u2 = (int)U2[0];
                        v1 = (int)U1[1];
                        v2 = (int)U2[1];

                        if (Poly_cross((double)x1, (double)y1, (double)x2, (double)y2, (double)u1, (double)u2,
                                       (double)v1, (double)v2)) {
                            cout << "Segment " << j + 1 << "-" << j + 2 << ": U1[0]" << U1[0] << ", U1[1]" << U1[1]
                                 << ", U2[0]" << U2[0] << "U2[1]" << U1[1] << endl;
                            cout << "Vs Segment " << i + 1 << "-" << i + 2 << ": V1[0]" << V1[0] << ", V1[1]" << V1[1]
                                 << ", V2[0]" << V2[0] << "V2[1]" << V1[1] << endl;

                            valid = 0;
                        }
                    }
                }
                // Last edge
                const vtkVector2i &F = point_list[0];                 // first point
                const vtkVector2i &L = point_list[point_number - 1];  // last point
                x1 = (int)L[0];
                x2 = (int)F[0];
                y1 = (int)L[1];
                y2 = (int)F[1];

                for (i = 1; i < point_number - 2; i++) {
                    const vtkVector2i &V1 = point_list[i];
                    const vtkVector2i &V2 = point_list[i + 1];

                    u1 = (int)V1[0];
                    u2 = (int)V2[0];
                    v1 = (int)V1[1];
                    v2 = (int)V2[1];
                    // fprintf(stderr, "Segment %d - %d\n", point_number, i + 1);

                    if (Poly_cross((double)x1, (double)y1, (double)x2, (double)y2, (double)u1, (double)u2, (double)v1,
                                   (double)v2)) {
                        cout << "Last " << point_number << "-" << 1 << ": F[0]" << F[0] << ", F[1]" << F[1] << ", L[0]"
                             << L[0] << "L[1]" << L[1] << endl;
                        cout << "Vs Segment" << i + 1 << "-" << i + 2 << ": V1[0]" << V1[0] << ", V1[1]" << V1[1]
                             << ", V2[0]" << V2[0] << "V2[1]" << V1[1] << endl;

                        valid = 0;
                    }
                }
            }
        }
        return valid;
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mwidget = new QVTKOpenGLNativeWidget(ui->widget);
    QVBoxLayout *l = new QVBoxLayout(ui->widget);
    l->addWidget(mwidget);
    mwidget->setRenderWindow(rw);
    rw->AddRenderer(mrenderer);
    vtkNew<vtkSTLReader> reader;
    reader->SetFileName("D:\\THA\\build\\bin\\x64\\Release\\prosthesisdata\\MeshSTL\\THA_Pelvis.stl");
    reader->Update();
    
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputData(reader->GetOutput());
    actor->SetMapper(mapper);
    actor->GetProperty()->SetSpecular(0.);
    actor->GetProperty()->SetAmbient(0.);
    mrenderer->AddActor(actor);

    vtkNew<vtkCellPicker> picker;
    mwidget->interactor()->SetPicker(picker);
    //mrenderer->SetLightFollowCamera(false);
    //mrenderer->UseDepthPeelingOff();  // �ر���Ȱ���
    //mrenderer->PreserveDepthBufferOff();  // �ر��������

    //actor->GetProperty()->ShadingOff();
}
#include <vtkMatrix4x4.h>
#include <vtkPointData.h>
#include <vtkThreshold.h>
#include <vtkMaskFields.h>
#include <vtkGeometryFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkTransform.h>
#include <QDebug>
void transformPoint(vtkMatrix4x4 *matrix, double pointin[3], double pointout[3])
{
    double pointPred[4];
    double pointNew[4] = {0, 0, 0, 0};
    pointPred[0] = pointin[0];
    pointPred[1] = pointin[1];
    pointPred[2] = pointin[2];
    pointPred[3] = 1;

    matrix->MultiplyPoint(pointPred, pointNew);
    pointout[0] = pointNew[0];
    pointout[1] = pointNew[1];
    pointout[2] = pointNew[2];
}

void getWorldToDisplay(vtkRenderer *vtkRenderer,double x, double y, double z, double displayPt[3])
{
    vtkRenderer->SetWorldPoint(x, y, z, 1.0);
    vtkRenderer->WorldToDisplay();
    vtkRenderer->GetDisplayPoint(displayPt);
}
#include <vtkRendererCollection.h>
void MainWindow::lassoselect(vtkObject *caller, long unsigned int vtkNotUsed(eventId), void *clientData,
                             void *callData)
{
    vtkInteractorStyleDrawPolygon *style=vtkInteractorStyleDrawPolygon::SafeDownCast(caller);
    vtkRenderWindow *rw = style->GetInteractor()->GetRenderWindow();
    vtkRenderer *mrenderer = rw->GetRenderers()->GetFirstRenderer();
    vtkActor *actor = (vtkActor *)clientData;
    POLYGON_LIST poly;
    poly.SetPointList(style->GetPolygonPoints());
    cout << "Poly valide: " << poly.state << endl;
    if (poly.state == 1)
    {
        vtkPolyDataMapper *mymapper = vtkPolyDataMapper::SafeDownCast(actor->GetMapper());
        if (mymapper != NULL && vtkPolyData::SafeDownCast(mymapper->GetInput()) != NULL) {
            vtkPolyData *myPD = vtkPolyData::SafeDownCast(mymapper->GetInput());
            vtkSmartPointer<vtkIntArray> newCuts = vtkSmartPointer<vtkIntArray>::New();

            newCuts->SetNumberOfComponents(1);  // 3d normals (ie x,y,z)
            newCuts->SetNumberOfTuples(myPD->GetNumberOfPoints());
            double ve_init_pos[3];
            ;
            double ve_final_pos[3];
            double ve_proj_screen[3];
            vtkSmartPointer<vtkMatrix4x4> Mat = actor->GetMatrix();
            POLYGON_VERTEX proj_screen;
            int proj_is_inside;
            for (vtkIdType i = 0; i < myPD->GetNumberOfPoints(); i++) {
                // for every triangle
                myPD->GetPoint(i, ve_init_pos);
                transformPoint(Mat, ve_init_pos, ve_final_pos);
                getWorldToDisplay(mrenderer, ve_final_pos[0], ve_final_pos[1], ve_final_pos[2], ve_proj_screen);
                if (i < 10) {
                    cout << "ve_proj_screen " << i << "=" << ve_proj_screen[0] << "," << ve_proj_screen[1] << ","
                         << ve_proj_screen[2] << endl;
                }
                proj_screen.x = ve_proj_screen[0];
                proj_screen.y = ve_proj_screen[1];
                proj_is_inside = poly.POLYGON_POINT_INSIDE(proj_screen);

                if ((ve_proj_screen[2] > -1.0) && ve_proj_screen[2] < 1.0 && (proj_is_inside == 1)) {
                    newCuts->InsertTuple1(i, 1);
                }
            }
            newCuts->SetName("Cuts");
            myPD->GetPointData()->AddArray(newCuts);
            myPD->GetPointData()->SetActiveScalars("Cuts");

            vtkSmartPointer<vtkThreshold> selector = vtkSmartPointer<vtkThreshold>::New();
            vtkSmartPointer<vtkMaskFields> scalarsOff = vtkSmartPointer<vtkMaskFields>::New();
            vtkSmartPointer<vtkGeometryFilter> geometry = vtkSmartPointer<vtkGeometryFilter>::New();
            selector->SetInputData((vtkPolyData *)myPD);
            /*selector->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS,
                                             vtkDataSetAttributes::SCALARS);*/
            selector->SetAllScalars(1);  // was g_tag_extraction_criterion_all
            selector->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, "Cuts");
            // selector->ThresholdBetween(0.9, 1.1);
            selector->SetLowerThreshold(0.9);
            selector->SetUpperThreshold(1.1);
            selector->Update();
            std::cout << "\nSelector new Number of points:" << selector->GetOutput()->GetNumberOfPoints() << std::endl;
            std::cout << "\nSelector new Number of cells:" << selector->GetOutput()->GetNumberOfCells() << std::endl;

            scalarsOff->SetInputData(selector->GetOutput());
            scalarsOff->CopyAttributeOff(vtkMaskFields::POINT_DATA, vtkDataSetAttributes::SCALARS);
            scalarsOff->CopyAttributeOff(vtkMaskFields::CELL_DATA, vtkDataSetAttributes::SCALARS);
            scalarsOff->Update();
            geometry->SetInputData(scalarsOff->GetOutput());
            geometry->Update();

            vtkSmartPointer<vtkPolyData> MyObj = vtkSmartPointer<vtkPolyData>::New();

            MyObj = geometry->GetOutput();
            myPD->GetPointData()->RemoveArray("Cuts");

            std::cout << "\nLasso new Number of points:" << MyObj->GetNumberOfPoints() << std::endl;
            std::cout << "\nLasso  new Number of cells:" << MyObj->GetNumberOfCells() << std::endl;

            if (MyObj->GetNumberOfPoints() > 10) {
                VTK_CREATE(vtkActor, newactor);

                // VTK_CREATE(vtkDataSetMapper, newmapper);
                VTK_CREATE(vtkPolyDataMapper, newmapper);
                // VTK_CREATE(vtkSmartPointer<vtkDataSetMapper>
                // newmapper->ImmediateModeRenderingOn();
                //newmapper->SetColorModeToDefault();
                const double units0 = -.1;
                newmapper->SetResolveCoincidentTopologyToPolygonOffset();
                newmapper->SetRelativeCoincidentTopologyLineOffsetParameters(0, units0);
                newmapper->SetRelativeCoincidentTopologyPolygonOffsetParameters(0, units0);
                newmapper->SetRelativeCoincidentTopologyPointOffsetParameter(units0);

                /*newmapper->ScalarVisibilityOn();*/

                cout << "cut object" << MyObj->GetNumberOfPoints() << endl;
                // newmapper->SetInputConnection(delaunay3D->GetOutputPort());
                newmapper->SetInputData(MyObj);

                // VTK_CREATE(vtkActor, actor);

                int num = 2;

                vtkSmartPointer<vtkMatrix4x4> Mat = actor->GetMatrix();
                vtkTransform *newTransform = vtkTransform::New();
                newTransform->PostMultiply();

                newTransform->SetMatrix(Mat);
                newactor->SetPosition(newTransform->GetPosition());
                newactor->SetScale(newTransform->GetScale());
                newactor->SetOrientation(newTransform->GetOrientation());
                newactor->GetProperty()->SetColor(1,0,0);
                newactor->GetProperty()->SetSpecular(0);
                newactor->GetProperty()->SetAmbient(0);
                //newactor->GetProperty()->ShadingOff();
                newTransform->Delete();
                newactor->SetMapper(newmapper);
                //mymapper->SetInputData(MyObj);
                mrenderer->AddActor(newactor);

                /*vtkNew<vtkDataSetMapper> dsmapper;
                vtkNew<vtkActor> dsactor;
                dsactor->SetMapper(dsmapper);
                dsmapper->SetInputData(selector->GetOutput());
                mrenderer->AddActor(dsactor);
                actor->SetVisibility(0);
                newactor->SetVisibility(0);*/
            }
        }
    }
    rw->Render();
}

void MainWindow::rectselect(vtkObject *caller, unsigned long, void *clientData, void *)
{
    vtkInteractorStyleRubberBand3D *style = vtkInteractorStyleRubberBand3D::SafeDownCast(caller);
    vtkRenderWindow *rw = style->GetInteractor()->GetRenderWindow();
    vtkRenderer *mrenderer = rw->GetRenderers()->GetFirstRenderer();
    vtkActor *actor = (vtkActor *)clientData;
    POLYGON_LIST poly;
    int mstart[2];
    int mend[2];

    style->GetStartPosition(mstart);
    style->GetEndPosition(mend);
    // std::vector<vtkVector2i>
    vtkVector2i a, b, c, d;
    a[0] = mstart[0];
    a[1] = mstart[1];

    b[0] = mstart[0];
    b[1] = mend[1];

    c[0] = mend[0];
    c[1] = mend[1];

    d[0] = mend[0];
    d[1] = mstart[1];
    cout << a[0] << "," << a[1] << endl;
    cout << b[0] << "," << b[1] << endl;
    cout << c[0] << "," << c[1] << endl;
    cout << d[0] << "," << d[1] << endl;

    std::vector<vtkVector2i> point_list;
    point_list.push_back(a);
    point_list.push_back(b);
    point_list.push_back(c);
    point_list.push_back(d);
    poly.SetPointList(point_list);
    cout << "Poly valide: " << poly.state << endl;
    if (poly.state == 1) {
        vtkPolyDataMapper *mymapper = vtkPolyDataMapper::SafeDownCast(actor->GetMapper());
        if (mymapper != NULL && vtkPolyData::SafeDownCast(mymapper->GetInput()) != NULL) {
            vtkPolyData *myPD = vtkPolyData::SafeDownCast(mymapper->GetInput());
            vtkSmartPointer<vtkIntArray> newCuts = vtkSmartPointer<vtkIntArray>::New();

            newCuts->SetNumberOfComponents(1);  // 3d normals (ie x,y,z)
            newCuts->SetNumberOfTuples(myPD->GetNumberOfPoints());
            double ve_init_pos[3];
            ;
            double ve_final_pos[3];
            double ve_proj_screen[3];
            vtkSmartPointer<vtkMatrix4x4> Mat = actor->GetMatrix();
            POLYGON_VERTEX proj_screen;
            int proj_is_inside;
            for (vtkIdType i = 0; i < myPD->GetNumberOfPoints(); i++) {
                // for every triangle
                myPD->GetPoint(i, ve_init_pos);
                transformPoint(Mat, ve_init_pos, ve_final_pos);
                getWorldToDisplay(mrenderer, ve_final_pos[0], ve_final_pos[1], ve_final_pos[2], ve_proj_screen);
                if (i < 10) {
                    cout << "ve_proj_screen " << i << "=" << ve_proj_screen[0] << "," << ve_proj_screen[1] << ","
                         << ve_proj_screen[2] << endl;
                }
                proj_screen.x = ve_proj_screen[0];
                proj_screen.y = ve_proj_screen[1];
                proj_is_inside = poly.POLYGON_POINT_INSIDE(proj_screen);

                if ((ve_proj_screen[2] > -1.0) && ve_proj_screen[2] < 1.0 && (proj_is_inside == 1)) {
                    newCuts->InsertTuple1(i, 1);
                }
            }
            newCuts->SetName("Cuts");
            myPD->GetPointData()->AddArray(newCuts);
            myPD->GetPointData()->SetActiveScalars("Cuts");

            vtkSmartPointer<vtkThreshold> selector = vtkSmartPointer<vtkThreshold>::New();
            vtkSmartPointer<vtkMaskFields> scalarsOff = vtkSmartPointer<vtkMaskFields>::New();
            vtkSmartPointer<vtkGeometryFilter> geometry = vtkSmartPointer<vtkGeometryFilter>::New();
            selector->SetInputData((vtkPolyData *)myPD);
            /*selector->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS,
                                             vtkDataSetAttributes::SCALARS);*/
            selector->SetAllScalars(1);  // was g_tag_extraction_criterion_all
            selector->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, "Cuts");
            // selector->ThresholdBetween(0.9, 1.1);
            selector->SetLowerThreshold(0.9);
            selector->SetUpperThreshold(1.1);
            selector->Update();
            std::cout << "\nSelector new Number of points:" << selector->GetOutput()->GetNumberOfPoints() << std::endl;
            std::cout << "\nSelector new Number of cells:" << selector->GetOutput()->GetNumberOfCells() << std::endl;

            scalarsOff->SetInputData(selector->GetOutput());
            scalarsOff->CopyAttributeOff(vtkMaskFields::POINT_DATA, vtkDataSetAttributes::SCALARS);
            scalarsOff->CopyAttributeOff(vtkMaskFields::CELL_DATA, vtkDataSetAttributes::SCALARS);
            scalarsOff->Update();
            geometry->SetInputData(scalarsOff->GetOutput());
            geometry->Update();

            vtkSmartPointer<vtkPolyData> MyObj = vtkSmartPointer<vtkPolyData>::New();

            MyObj = geometry->GetOutput();
            myPD->GetPointData()->RemoveArray("Cuts");

            std::cout << "\nLasso new Number of points:" << MyObj->GetNumberOfPoints() << std::endl;
            std::cout << "\nLasso  new Number of cells:" << MyObj->GetNumberOfCells() << std::endl;

            if (MyObj->GetNumberOfPoints() > 10) {
                VTK_CREATE(vtkActor, newactor);

                // VTK_CREATE(vtkDataSetMapper, newmapper);
                VTK_CREATE(vtkPolyDataMapper, newmapper);
                // VTK_CREATE(vtkSmartPointer<vtkDataSetMapper>
                // newmapper->ImmediateModeRenderingOn();
                // newmapper->SetColorModeToDefault();
                const double units0 = -.1;
                newmapper->SetResolveCoincidentTopologyToPolygonOffset();
                newmapper->SetRelativeCoincidentTopologyLineOffsetParameters(0, units0);
                newmapper->SetRelativeCoincidentTopologyPolygonOffsetParameters(0, units0);
                newmapper->SetRelativeCoincidentTopologyPointOffsetParameter(units0);

                /*newmapper->ScalarVisibilityOn();*/

                cout << "cut object" << MyObj->GetNumberOfPoints() << endl;
                // newmapper->SetInputConnection(delaunay3D->GetOutputPort());
                newmapper->SetInputData(MyObj);

                // VTK_CREATE(vtkActor, actor);

                int num = 2;

                vtkSmartPointer<vtkMatrix4x4> Mat = actor->GetMatrix();
                vtkTransform *newTransform = vtkTransform::New();
                newTransform->PostMultiply();

                newTransform->SetMatrix(Mat);
                newactor->SetPosition(newTransform->GetPosition());
                newactor->SetScale(newTransform->GetScale());
                newactor->SetOrientation(newTransform->GetOrientation());
                newactor->GetProperty()->SetColor(1, 0, 0);
                newactor->GetProperty()->SetSpecular(0);
                newactor->GetProperty()->SetAmbient(0);
                // newactor->GetProperty()->ShadingOff();
                newTransform->Delete();
                newactor->SetMapper(newmapper);
                // mymapper->SetInputData(MyObj);
                mrenderer->AddActor(newactor);

                /*vtkNew<vtkDataSetMapper> dsmapper;
                vtkNew<vtkActor> dsactor;
                dsactor->SetMapper(dsmapper);
                dsmapper->SetInputData(selector->GetOutput());
                mrenderer->AddActor(dsactor);
                actor->SetVisibility(0);
                newactor->SetVisibility(0);*/
            }
        }
    }
    rw->Render();
}

void MainWindow::randomselect(vtkObject *caller, unsigned long, void *clientData, void *)
{
    PointPickerInteractorStyle1 *style = PointPickerInteractorStyle1::SafeDownCast(caller);
    vtkRenderWindow *rw = style->GetInteractor()->GetRenderWindow();
    vtkRenderer *mrenderer = rw->GetRenderers()->GetFirstRenderer();
    vtkActor *actor = (vtkActor *)clientData;
    POLYGON_LIST poly;
    int mstart[2];
    int mend[2];

    auto point_list = style->screenPoints;
    //point_list.push_back(style->screenPoints[0]);
    poly.SetPointList(point_list);
    cout << "Poly valide: " << poly.state << endl;
    if (poly.state == 1) {
        vtkPolyDataMapper *mymapper = vtkPolyDataMapper::SafeDownCast(actor->GetMapper());
        if (mymapper != NULL && vtkPolyData::SafeDownCast(mymapper->GetInput()) != NULL) {
            vtkPolyData *myPD = vtkPolyData::SafeDownCast(mymapper->GetInput());
            vtkSmartPointer<vtkIntArray> newCuts = vtkSmartPointer<vtkIntArray>::New();

            newCuts->SetNumberOfComponents(1);  // 3d normals (ie x,y,z)
            newCuts->SetNumberOfTuples(myPD->GetNumberOfPoints());
            double ve_init_pos[3];
            ;
            double ve_final_pos[3];
            double ve_proj_screen[3];
            vtkSmartPointer<vtkMatrix4x4> Mat = actor->GetMatrix();
            POLYGON_VERTEX proj_screen;
            int proj_is_inside;
            for (vtkIdType i = 0; i < myPD->GetNumberOfPoints(); i++) {
                // for every triangle
                myPD->GetPoint(i, ve_init_pos);
                transformPoint(Mat, ve_init_pos, ve_final_pos);
                getWorldToDisplay(mrenderer, ve_final_pos[0], ve_final_pos[1], ve_final_pos[2], ve_proj_screen);
                if (i < 10) {
                    cout << "ve_proj_screen " << i << "=" << ve_proj_screen[0] << "," << ve_proj_screen[1] << ","
                         << ve_proj_screen[2] << endl;
                }
                proj_screen.x = ve_proj_screen[0];
                proj_screen.y = ve_proj_screen[1];
                proj_is_inside = poly.POLYGON_POINT_INSIDE(proj_screen);

                if ((ve_proj_screen[2] > -1.0) && ve_proj_screen[2] < 1.0 && (proj_is_inside == 1)) {
                    newCuts->InsertTuple1(i, 1);
                }
            }
            newCuts->SetName("Cuts");
            myPD->GetPointData()->AddArray(newCuts);
            myPD->GetPointData()->SetActiveScalars("Cuts");

            vtkSmartPointer<vtkThreshold> selector = vtkSmartPointer<vtkThreshold>::New();
            vtkSmartPointer<vtkMaskFields> scalarsOff = vtkSmartPointer<vtkMaskFields>::New();
            vtkSmartPointer<vtkGeometryFilter> geometry = vtkSmartPointer<vtkGeometryFilter>::New();
            selector->SetInputData((vtkPolyData *)myPD);
            /*selector->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS,
                                             vtkDataSetAttributes::SCALARS);*/
            selector->SetAllScalars(1);  // was g_tag_extraction_criterion_all
            selector->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, "Cuts");
            // selector->ThresholdBetween(0.9, 1.1);
            selector->SetLowerThreshold(0.9);
            selector->SetUpperThreshold(1.1);
            selector->Update();
            std::cout << "\nSelector new Number of points:" << selector->GetOutput()->GetNumberOfPoints() << std::endl;
            std::cout << "\nSelector new Number of cells:" << selector->GetOutput()->GetNumberOfCells() << std::endl;

            scalarsOff->SetInputData(selector->GetOutput());
            scalarsOff->CopyAttributeOff(vtkMaskFields::POINT_DATA, vtkDataSetAttributes::SCALARS);
            scalarsOff->CopyAttributeOff(vtkMaskFields::CELL_DATA, vtkDataSetAttributes::SCALARS);
            scalarsOff->Update();
            geometry->SetInputData(scalarsOff->GetOutput());
            geometry->Update();

            vtkSmartPointer<vtkPolyData> MyObj = vtkSmartPointer<vtkPolyData>::New();

            MyObj = geometry->GetOutput();
            myPD->GetPointData()->RemoveArray("Cuts");

            std::cout << "\nLasso new Number of points:" << MyObj->GetNumberOfPoints() << std::endl;
            std::cout << "\nLasso  new Number of cells:" << MyObj->GetNumberOfCells() << std::endl;

            if (MyObj->GetNumberOfPoints() > 10) {
                VTK_CREATE(vtkActor, newactor);

                // VTK_CREATE(vtkDataSetMapper, newmapper);
                VTK_CREATE(vtkPolyDataMapper, newmapper);
                // VTK_CREATE(vtkSmartPointer<vtkDataSetMapper>
                // newmapper->ImmediateModeRenderingOn();
                // newmapper->SetColorModeToDefault();
                const double units0 = -.1;
                newmapper->SetResolveCoincidentTopologyToPolygonOffset();
                newmapper->SetRelativeCoincidentTopologyLineOffsetParameters(0, units0);
                newmapper->SetRelativeCoincidentTopologyPolygonOffsetParameters(0, units0);
                newmapper->SetRelativeCoincidentTopologyPointOffsetParameter(units0);

                /*newmapper->ScalarVisibilityOn();*/

                cout << "cut object" << MyObj->GetNumberOfPoints() << endl;
                // newmapper->SetInputConnection(delaunay3D->GetOutputPort());
                newmapper->SetInputData(MyObj);

                // VTK_CREATE(vtkActor, actor);

                int num = 2;

                vtkSmartPointer<vtkMatrix4x4> Mat = actor->GetMatrix();
                vtkTransform *newTransform = vtkTransform::New();
                newTransform->PostMultiply();

                newTransform->SetMatrix(Mat);
                newactor->SetPosition(newTransform->GetPosition());
                newactor->SetScale(newTransform->GetScale());
                newactor->SetOrientation(newTransform->GetOrientation());
                newactor->GetProperty()->SetColor(1, 0, 0);
                newactor->GetProperty()->SetSpecular(0);
                newactor->GetProperty()->SetAmbient(0);
                // newactor->GetProperty()->ShadingOff();
                newTransform->Delete();
                newactor->SetMapper(newmapper);
                // mymapper->SetInputData(MyObj);
                mrenderer->AddActor(newactor);

                /*vtkNew<vtkDataSetMapper> dsmapper;
                vtkNew<vtkActor> dsactor;
                dsactor->SetMapper(dsmapper);
                dsmapper->SetInputData(selector->GetOutput());
                mrenderer->AddActor(dsactor);
                actor->SetVisibility(0);
                newactor->SetVisibility(0);*/
            }
        }
    }
    rw->Render();
}

void MainWindow::on_pushButton_rect_clicked()
{
    vtkSmartPointer<vtkInteractorStyleRubberBand3D> style = vtkSmartPointer<vtkInteractorStyleRubberBand3D>::New();
    mwidget->interactor()->SetInteractorStyle(style);
    rectselectionCallback->SetClientData(actor);
    rectselectionCallback->SetCallback(&MainWindow::rectselect);
    style->AddObserver(vtkCommand::SelectionChangedEvent, rectselectionCallback);
}

void MainWindow::on_pushButton_lasso_clicked()
{
    vtkSmartPointer<vtkInteractorStyleDrawPolygon> style = vtkSmartPointer<vtkInteractorStyleDrawPolygon>::New();
    mwidget->interactor()->SetInteractorStyle(style);
    lassoselectionCallback->SetClientData(actor);
    lassoselectionCallback->SetCallback(&MainWindow::lassoselect);
    style->AddObserver(vtkCommand::SelectionChangedEvent, lassoselectionCallback);
}

void MainWindow::on_pushButton_random_clicked()
{
    vtkSmartPointer<PointPickerInteractorStyle1> style = vtkSmartPointer<PointPickerInteractorStyle1>::New();
    mwidget->interactor()->SetInteractorStyle(style);
    randomCallback->SetClientData(actor);
    randomCallback->SetCallback(&MainWindow::randomselect);
    style->AddObserver(vtkCommand::SelectionChangedEvent, randomCallback);
}

MainWindow::~MainWindow()
{
    delete ui;
}

