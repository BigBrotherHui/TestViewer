
#include "ImplicitPolyDataDistance.h"

#include <vtkCellData.h>
#include <vtkCellLocator.h>
#include <vtkGenericCell.h>
#include <vtkMath.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolygon.h>
#include <vtkSmartPointer.h>

//------------------------------------------------------------------------------
ImplicitPolyDataDistance::ImplicitPolyDataDistance()
{
}

//------------------------------------------------------------------------------
ImplicitPolyDataDistance::~ImplicitPolyDataDistance()
{
}

//------------------------------------------------------------------------------
double ImplicitPolyDataDistance::EvaluateFunctionDistance(double x[3], double g[3], double closestPoint[3])
{
    return SharedEvaluate(x, g, closestPoint);
}

double ImplicitPolyDataDistance::EvaluateFunctionDistance(Eigen::Vector3d x, Eigen::Vector3d& g,
                                                          Eigen::Vector3d& closestPoint)
{
    double dx[3] = {x.x(), x.y(), x.z()};

    double dg[3], dclosetPoint[3];

    double ret = EvaluateFunctionDistance(dx, dg, dclosetPoint);

    g << dg[0], dg[1], dg[2];
    closestPoint << dclosetPoint[0], dclosetPoint[1], dclosetPoint[2];

    return ret;
}

double ImplicitPolyDataDistance::FindClosestPoint(double x[3], double closestPoint[3])
{
    double g[3];
    // Set defaults
    double ret = this->NoValue;

    for (int i = 0; i < 3; i++) {
        g[i] = this->NoGradient[i];
    }

    for (int i = 0; i < 3; i++) {
        closestPoint[i] = this->NoClosestPoint[i];
    }

    // See if data set with polygons has been specified
    if (this->Input == nullptr || Input->GetNumberOfCells() == 0) {
        vtkErrorMacro(<< "No polygons to evaluate function!");
        return ret;
    }

    double p[3];
    vtkIdType cellId;
    int subId;
    double vlen2;

    vtkDataArray* cnorms = nullptr;
    if (this->Input->GetCellData() && this->Input->GetCellData()->GetNormals()) {
        cnorms = this->Input->GetCellData()->GetNormals();
    }

    // Get point id of closest point in data set.
    vtkSmartPointer<vtkGenericCell> cell = vtkSmartPointer<vtkGenericCell>::New();
    this->Locator->FindClosestPoint(x, p, cell, cellId, subId, vlen2);

    if (cellId != -1)  // point located
    {
        // dist = | point - x |
        ret = sqrt(vlen2);
        // grad = (point - x) / dist
        for (int i = 0; i < 3; i++) {
            g[i] = (p[i] - x[i]) / (ret == 0. ? 1. : ret);
        }

        double dist2, weights[3], pcoords[3], awnorm[3] = {0, 0, 0};
        cell->EvaluatePosition(p, closestPoint, subId, pcoords, dist2, weights);

        //原数据上的点
        if (cell->GetNumberOfPoints() > 0) {
            double* data = cell->GetPoints()->GetPoint(0);
            for (int i = 0; i < 3; i++) {
                closestPoint[i] = data[i];
            }
        }

        vtkIdList* idList = vtkIdList::New();
        int count = 0;
        for (int i = 0; i < 3; i++) {
            count += (fabs(weights[i]) < this->Tolerance ? 1 : 0);
        }
        // Face case - weights contains no 0s
        if (count == 0) {
            // Compute face normal.
            if (cnorms) {
                cnorms->GetTuple(cellId, awnorm);
            }
            else {
                vtkPolygon::ComputeNormal(cell->Points, awnorm);
            }
        }
        // Edge case - weights contain one 0
        else if (count == 1) {
            // ... edge ... get two adjacent faces, compute average normal
            int a = -1, b = -1;
            for (int edge = 0; edge < 3; edge++) {
                if (fabs(weights[edge]) < this->Tolerance) {
                    a = cell->PointIds->GetId((edge + 1) % 3);
                    b = cell->PointIds->GetId((edge + 2) % 3);
                    break;
                }
            }

            if (a == -1) {
                vtkErrorMacro(<< "Could not find edge when closest point is "
                              << "expected to be on an edge.");
                return this->NoValue;
            }

            // The first argument is the cell ID. We pass a bogus cell ID so that
            // all face IDs attached to the edge are returned in the idList.
            this->Input->GetCellEdgeNeighbors(VTK_ID_MAX, a, b, idList);
            for (int i = 0; i < idList->GetNumberOfIds(); i++) {
                double norm[3];
                if (cnorms) {
                    cnorms->GetTuple(idList->GetId(i), norm);
                }
                else {
                    vtkPolygon::ComputeNormal(this->Input->GetCell(idList->GetId(i))->GetPoints(), norm);
                }
                awnorm[0] += norm[0];
                awnorm[1] += norm[1];
                awnorm[2] += norm[2];
            }
            vtkMath::Normalize(awnorm);
        }

        // Vertex case - weights contain two 0s
        else if (count == 2) {
            // ... vertex ... this is the expensive case, get all adjacent
            // faces and compute sum(a_i * n_i) Angle-Weighted Pseudo
            // Normals, J. Andreas Baerentzen and Henrik Aanaes
            int a = -1;
            for (int i = 0; i < 3; i++) {
                if (fabs(weights[i]) > this->Tolerance) {
                    a = cell->PointIds->GetId(i);
                }
            }

            if (a == -1) {
                vtkErrorMacro(<< "Could not find point when closest point is "
                              << "expected to be a point.");
                return this->NoValue;
            }

            this->Input->GetPointCells(a, idList);
            for (int i = 0; i < idList->GetNumberOfIds(); i++) {
                double norm[3];
                if (cnorms) {
                    cnorms->GetTuple(idList->GetId(i), norm);
                }
                else {
                    vtkPolygon::ComputeNormal(this->Input->GetCell(idList->GetId(i))->GetPoints(), norm);
                }

                // Compute angle at point a
                int b = this->Input->GetCell(idList->GetId(i))->GetPointId(0);
                int c = this->Input->GetCell(idList->GetId(i))->GetPointId(1);
                if (a == b) {
                    b = this->Input->GetCell(idList->GetId(i))->GetPointId(2);
                }
                else if (a == c) {
                    c = this->Input->GetCell(idList->GetId(i))->GetPointId(2);
                }
                double pa[3], pb[3], pc[3];
                this->Input->GetPoint(a, pa);
                this->Input->GetPoint(b, pb);
                this->Input->GetPoint(c, pc);
                for (int j = 0; j < 3; j++) {
                    pb[j] -= pa[j];
                    pc[j] -= pa[j];
                }
                vtkMath::Normalize(pb);
                vtkMath::Normalize(pc);
                double alpha = acos(vtkMath::Dot(pb, pc));
                awnorm[0] += alpha * norm[0];
                awnorm[1] += alpha * norm[1];
                awnorm[2] += alpha * norm[2];
            }
            vtkMath::Normalize(awnorm);
        }
        idList->Delete();

        // sign(dist) = dot(grad, cell normal)
        if (ret == 0) {
            for (int i = 0; i < 3; i++) {
                g[i] = awnorm[i];
            }
        }
        ret *= (vtkMath::Dot(g, awnorm) < 0.) ? 1. : -1.;

        if (ret > 0.) {
            for (int i = 0; i < 3; i++) {
                g[i] = -g[i];
            }
        }
    }

    return ret;
}
