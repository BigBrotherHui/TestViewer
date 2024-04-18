#pragma once
#include "AlgorithmExports.h"
#include <vector>
#include <Eigen/Eigen>
#include <vtkPoints.h>
Algorithm_EXPORT void calculateTranslateX(double &x, double &y, double angle);
Algorithm_EXPORT void calculateTranslateY(double &x, double &y, double angle);
Algorithm_EXPORT int calculateCursorState(double angle);
Algorithm_EXPORT void sortVtkPoints(vtkPoints *vtkpoints);
Algorithm_EXPORT Eigen::MatrixX3d projectPoints(const Eigen::MatrixX3d &points, const Eigen::Vector4d &parameters);
Algorithm_EXPORT void ProjectToPlane(const double x[3], const double origin[3], const double normal[3],
                                     double xproj[3]);
Algorithm_EXPORT Eigen::MatrixX3d svdTransform(const Eigen::MatrixX3d &points);