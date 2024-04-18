
#include <vtkImplicitPolyDataDistance.h>


#include "AlgorithmExports.h"

#include <Eigen/Dense>
#ifndef ImplicitPolyDataDistance_h
#define ImplicitPolyDataDistance_h

class Algorithm_EXPORT ImplicitPolyDataDistance : public vtkImplicitPolyDataDistance {
public:
    static ImplicitPolyDataDistance* New() { return new ImplicitPolyDataDistance; }
    ImplicitPolyDataDistance();
    ~ImplicitPolyDataDistance() override;

    // x:输入的点   g:输出的法向量（反方向） return:距离安全墙的距离（正：安全墙外面  负：在安全墙里面）
    // 注：构造以后要SetInput()
    double EvaluateFunctionDistance(double x[3], double g[3], double closestPoint[3]);
    double EvaluateFunctionDistance(Eigen::Vector3d x, Eigen::Vector3d& g, Eigen::Vector3d& closestPoint);

    /**
     * @brief 包围线上点距离和临近的其中一个点
     * @param double x[3]  目标点
     * @param double closestPoint[3]  面片上其中一个点
     * @return double  距离
     */
    double FindClosestPoint(double x[3], double closestPoint[3]);

private:
    ImplicitPolyDataDistance(const vtkImplicitPolyDataDistance&) = delete;
};

#endif
