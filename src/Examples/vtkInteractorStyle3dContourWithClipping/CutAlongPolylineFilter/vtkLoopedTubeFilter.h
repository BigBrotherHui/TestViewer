/*
Author:		Wong, Matthew Lun
Date:		05, October 2016
Occupation:	Chinese University of Hong Kong,
			Department of Imaging and Inteventional Radiology,
			Junior Research Assistant

FileName: 	vtkLoopedTubeFilter.h

Descriptions:
	This class inherits vtkTubeFilter and creat a looped tube.


Wong Matthew Lun
Copyright (C) 2016
*/

#include <vtkObjectFactory.h>
#include <vtkTubeFilter.h>

class vtkLoopedTubeFilter : public vtkTubeFilter
{
public:
	vtkTypeMacro(vtkLoopedTubeFilter, vtkTubeFilter);
	static vtkLoopedTubeFilter* New();

	virtual int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*);

protected:
	vtkLoopedTubeFilter() {};
	~vtkLoopedTubeFilter() {};

	void GenerateStripsForLoop(vtkIdType offset, vtkIdType npts,
		const vtkIdType* vtkNotUsed(pts),
		vtkIdType inCellId,
		vtkCellData *cd, vtkCellData *outCD,
		vtkCellArray *newStrips);

	int  GeneratePointsForLoop(vtkIdType offset,
		vtkIdType npts, const vtkIdType *pts,
		vtkPoints *inPts, vtkPoints *newPts,
		vtkPointData *pd, vtkPointData *outPD,
		vtkFloatArray *newNormals,
		vtkDataArray *inScalars, double range[2],
		vtkDataArray *inVectors, double maxSpeed,
		vtkDataArray *inNormals);

private:
};
 