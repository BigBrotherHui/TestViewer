#include "vtkAlgorithm.h"
#include <math.h>
#define M_PI 3.1415926
#define VTK_CURSOR_DEFAULT 0
#define VTK_CURSOR_ARROW 1
#define VTK_CURSOR_SIZENE 2
#define VTK_CURSOR_SIZENW 3
#define VTK_CURSOR_SIZESW 4
#define VTK_CURSOR_SIZESE 5
#define VTK_CURSOR_SIZENS 6
#define VTK_CURSOR_SIZEWE 7
#define VTK_CURSOR_SIZEALL 8
#define VTK_CURSOR_HAND 9
#define VTK_CURSOR_CROSSHAIR 10
#define VTK_CURSOR_CUSTOM 11
void calculateTranslateX(double &x, double &y, double angle)
{
    double nx = x * cos(angle) + y * sin(angle);
    x = nx * cos(angle);
    y = nx * sin(angle);
}
void calculateTranslateY(double &x, double &y, double angle)
{
    double ny = x * -sin(angle) + y * cos(angle);
    x = -ny * sin(angle);
    y = ny * cos(angle);
}

int calculateCursorState(double pangle)
{
    double angle = fmod(pangle, 180);
    if ((fabs(angle) > 180.0 - 22.5) || (fabs(angle) < 22.5)) {
        return VTK_CURSOR_SIZENS;
    }
    if (fabs(fabs(angle) - 90.0) < 22.5) {
        return VTK_CURSOR_SIZEWE;
    }
    if ((fabs(angle - 135.0) < 22.5) || (fabs(angle + 45.0) < 22.5)) {
        return VTK_CURSOR_SIZENE;
    }
    return VTK_CURSOR_SIZENW;
}
