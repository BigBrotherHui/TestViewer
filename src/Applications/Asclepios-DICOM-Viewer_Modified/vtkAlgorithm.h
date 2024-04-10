#ifdef vtkAlgorithm_EXPORT
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif
void DLLEXPORT calculateTranslateX(double &x, double &y, double angle);
void DLLEXPORT calculateTranslateY(double &x, double &y, double angle);
int DLLEXPORT calculateCursorState(double angle);