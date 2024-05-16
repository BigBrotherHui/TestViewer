#pragma once

#include <qglobal.h>
#include <qmath.h>
#include <thread>
#include <vtkBox.h>
#include <vtkImplicitPolyDataDistance.h>
#include "vtkPolyDataAlgorithm.h"
#include "vtkContourValues.h"
#include <vtkDataSetAttributes.h>
#include <QtCore/QList>
#include "vtkImageData.h"
#include "vtkMatrix4x4.h"


class NTBooleanOperationFilter : public vtkPolyDataAlgorithm
{
public:
	static NTBooleanOperationFilter* New();
vtkTypeMacro(NTBooleanOperationFilter, vtkPolyDataAlgorithm)

	NTBooleanOperationFilter(const NTBooleanOperationFilter&) = delete;
	// void operator=(const NTBooleanOperationFilter&) = delete;

	vtkGetMacro(Radius, double)
	vtkSetVector3Macro(Center, double)
	vtkGetVectorMacro(Center, double, 3)

	enum BoneType
	{
		Femur,
		Tibia
	};

	void SetBoneImageAndRadius(vtkImageData* input0, double offset[3], vtkPolyData* implant,
	                           BoneType type = Tibia, double BallRadius = 2.5);
	void GetExactSurface(vtkPolyData* outSurface);
	void GetExactImageData(vtkImageData* outImageData);

	// struct ConcurrentData
	// {
	// 	float dis;
	// 	vtkNew<vtkPolyData> mesh;
	// 	vtkNew<vtkImplicitPolyDataDistance> implantFilter;
	// 	vtkNew<vtkImplicitPolyDataDistance> remainBoneFilter;
	//
	//
	// 	// vtkNew<vtkPolyData> implant;
	// 	// vtkNew<vtkPolyData> remainBone;
	// };
	// QList<ConcurrentData> dataList;

protected:
	NTBooleanOperationFilter();
	int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
	double Radius = 2.5;
	double Center[3] = {VTK_DOUBLE_MAX, VTK_DOUBLE_MAX, VTK_DOUBLE_MAX};
	double Spacing[3];
	double Origin[3];
	int CutExtents[6]; //Í¼Ïñ·¶Î§

	BoneType boneType = Tibia;

	const uchar red[3] = {255, 0, 0};
	const uchar green[3] = {0, 255, 0};
	const uchar white[3] = {255, 255, 255};

	bool isCanAddColor = false;

	vtkNew<vtkImageData> holdImage;
	vtkNew<vtkPolyData> holdPolyData;
	vtkNew<vtkPolyData> patchPolyData;
	vtkNew<vtkPolyData> midPolyData;
	vtkNew<vtkImplicitPolyDataDistance> implantFilter;
	// vtkNew<vtkImplicitPolyDataDistance> implantFilter_copy;
	vtkNew<vtkImplicitPolyDataDistance> remainBoneFilter;
	// vtkNew<vtkImplicitPolyDataDistance> remainBoneFilter_copy;

	bool ErasePixel();
	void Reconstruction();
	void UpdatePatchPolyData();
	// void UpdatePatchPolyData2();
	void UpdateMidPolyData();
	void AddColor(vtkPolyData* mesh);
	void SetUpColorFilter(vtkImageData* boneImage, vtkPolyData* implant);
	void CreateSurface(vtkImageData* vtkImage, vtkPolyData* outSurface);
	void CreateSurfaceFast(vtkImageData* vtkImage, vtkPolyData* outSurface);
	void GetROIExtents(vtkImageData* input, int (&ROIExtents)[6]);

	// void LargestRegionFilter(vtkImageData* input);
	// void GetHaveValueExtents(vtkImageData* input, int (&CutExtents)[6]);
};
