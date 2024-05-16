#include "NTBooleanOperationFilter.h"
#include <qvector3d.h>
#include <vtkAppendPolyData.h>
#include <vtkCleanPolyData.h>
#include <vtkClipPolyData.h>
#include <vtkBox.h>
#include <vtkDecimatePro.h>
#include <vtkDoubleArray.h>
#include <vtkExtractVOI.h>
#include <vtkFloatArray.h>
#include <vtkGenericCell.h>
#include <vtkImageChangeInformation.h>
#include <vtkImplicitPolyDataDistance.h>
#include <vtkMarchingCubes.h>
#include <vtkPointData.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkSphereSource.h>
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMarchingCubesTriangleCases.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkImageData.h"
#include "vtkCellData.h"
#include "vtkUnsignedCharArray.h"
#include "vtkCommand.h"
#include "QTime"
#include "vtkKdTree.h"
#include <vtkSTLWriter.h>
#include <vtkDiscreteFlyingEdges3D.h>

vtkStandardNewMacro(NTBooleanOperationFilter);

NTBooleanOperationFilter::NTBooleanOperationFilter()
{
	this->SetNumberOfInputPorts(0);
	this->SetNumberOfOutputPorts(1);
}


void NTBooleanOperationFilter::SetBoneImageAndRadius(vtkImageData* input0, double offset[3], vtkPolyData* implant,
                                                     BoneType type, double BallRadius)
{
	QTime time0;
	time0.start();

	//重置参数
	Center[0] = VTK_DOUBLE_MAX;
	Center[1] = VTK_DOUBLE_MAX;
	Center[2] = VTK_DOUBLE_MAX;

	vtkNew<vtkImageChangeInformation> changeFilter;
	changeFilter->SetInputData(input0);
	changeFilter->SetOutputOrigin(offset);
	changeFilter->Update();
        vtkImageData* input = changeFilter->GetOutput();

	this->Radius = BallRadius;
	this->boneType = type;

	input->GetSpacing(Spacing);
	input->GetOrigin(Origin);

	GetROIExtents(input, CutExtents);

	vtkNew<vtkExtractVOI> extractVOI;
	extractVOI->SetInputData(input);
	extractVOI->SetVOI(CutExtents);
	extractVOI->Update();

	holdImage->ShallowCopy(extractVOI->GetOutput());
	CreateSurfaceFast(holdImage, holdPolyData);
	SetUpColorFilter(holdImage, implant);
	AddColor(holdPolyData);

	this->Modified();

	cout << "SetBoneImageAndRadius Time: " << time0.elapsed() << endl;
}

void NTBooleanOperationFilter::GetExactSurface(vtkPolyData* outSurface)
{
	CreateSurface(holdImage, outSurface);
	AddColor(outSurface);
}

void NTBooleanOperationFilter::GetExactImageData(vtkImageData* outImageData)
{
	outImageData->ShallowCopy(holdImage);
}


void NTBooleanOperationFilter::SetUpColorFilter(vtkImageData* boneImage, vtkPolyData* implant)
{
	if (boneImage == nullptr || implant == nullptr)
	{
		isCanAddColor = false;
		return;
	}

	vtkNew<vtkImageData> image;
	image->DeepCopy(boneImage);

	implantFilter->SetInput(implant);
	// implantFilter_copy->SetInput(implant);

	double Bounds[6];
	implant->GetBounds(Bounds);

	int Extents[6];
	Extents[0] = std::max(static_cast<int>(floor((Bounds[0] - Origin[0]) / Spacing[0])), CutExtents[0]);
	Extents[1] = std::min(static_cast<int>(ceil((Bounds[1] - Origin[0]) / Spacing[0])), CutExtents[1]);
	Extents[2] = std::max(static_cast<int>(floor((Bounds[2] - Origin[1]) / Spacing[1])), CutExtents[2]);
	Extents[3] = std::min(static_cast<int>(ceil((Bounds[3] - Origin[1]) / Spacing[1])), CutExtents[3]);
	Extents[4] = std::max(static_cast<int>(floor((Bounds[4] - Origin[2]) / Spacing[2])), CutExtents[4]);
	Extents[5] = std::min(static_cast<int>(ceil((Bounds[5] - Origin[2]) / Spacing[2])), CutExtents[5]);

	std::vector<std::vector<int>> vector;
	for (int i = Extents[0]; i <= Extents[1]; i++)
	{
		for (int j = Extents[2]; j <= Extents[3]; j++)
		{
			for (int k = Extents[4]; k <= Extents[5]; k++)
			{
				double pixelPos[3] = {
					i * Spacing[0] + Spacing[0] / 2 + Origin[0], j * Spacing[1] + Spacing[1] / 2 + Origin[1],
					k * Spacing[2] + Spacing[2] / 2 + Origin[2]
				};
				const double dis = implantFilter->EvaluateFunction(pixelPos);
				auto* pixel = static_cast<uchar*>(image->GetScalarPointer(i, j, k));
				if (dis < 0 && *pixel != 0)
				{
					std::vector<int> pos = {i, j, k};
					vector.push_back(pos);
				}
			}
		}
	}

	if (vector.size() < 10)
	{
		std::cout << "bone and implant no Intersect" << endl;
		isCanAddColor = false;
		return;
	}
	isCanAddColor = true;

	for (auto pos : vector)
	{
		auto* pixel = static_cast<uchar*>(image->GetScalarPointer(pos[0], pos[1], pos[2]));
		*pixel = 0;
	}

	vtkNew<vtkPolyData> remainBone;
	CreateSurfaceFast(image, remainBone);
	remainBoneFilter->SetInput(remainBone);
	// remainBoneFilter_copy->SetInput(remainBone);
}


int NTBooleanOperationFilter::RequestData(vtkInformation*vtkNotUsed(request), vtkInformationVector** inputVector,
                                          vtkInformationVector* outputVector)
{
	QTime time;
	time.start();

	vtkInformation* outInfo = outputVector->GetInformationObject(0);
	vtkPolyData* output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

	if (!ErasePixel())
	{
		output->ShallowCopy(holdPolyData);
		return 1;
	}
	
	// Reconstruction();

	std::thread t1([&]()
	{
		// QTime time1;
		// time1.start();

		UpdatePatchPolyData();

		// cout << "Time1: " << time1.elapsed() << endl;
	});
	std::thread t2([&]()
	{
		// QTime time2;
		// time2.start();

		UpdateMidPolyData();

		// cout << "Time2: " << time2.elapsed() << endl;
	});
	t1.join();
	t2.join();

	vtkNew<vtkAppendPolyData> append;
	append->AddInputData(patchPolyData);
	append->AddInputData(midPolyData);
	append->Update();

	output->ShallowCopy(append->GetOutput());
	holdPolyData->ShallowCopy(output);

	cout << "Total Time: " << time.elapsed() << endl;
	return 1;
}

bool NTBooleanOperationFilter::ErasePixel()
{
	int Extents[6];
	Extents[0] = std::max(static_cast<int>(floor((Center[0] - Origin[0] - Radius) / Spacing[0])),
	                      CutExtents[0]);
	Extents[1] = std::min(static_cast<int>(ceil((Center[0] - Origin[0] + Radius) / Spacing[0])),
	                      CutExtents[1]);
	Extents[2] = std::max(static_cast<int>(floor((Center[1] - Origin[1] - Radius) / Spacing[1])),
	                      CutExtents[2]);
	Extents[3] = std::min(static_cast<int>(ceil((Center[1] - Origin[1] + Radius) / Spacing[1])),
	                      CutExtents[3]);
	Extents[4] = std::max(static_cast<int>(floor((Center[2] - Origin[2] - Radius) / Spacing[2])),
	                      CutExtents[4]);
	Extents[5] = std::min(static_cast<int>(ceil((Center[2] - Origin[2] + Radius) / Spacing[2])),
	                      CutExtents[5]);

	std::vector<std::vector<int>> vector;
	for (int i = Extents[0]; i <= Extents[1]; i++)
	{
		for (int j = Extents[2]; j <= Extents[3]; j++)
		{
			for (int k = Extents[4]; k <= Extents[5]; k++)
			{
				double pixelPos[3] = {
					i * Spacing[0] + Spacing[0] / 2 + Origin[0], j * Spacing[1] + Spacing[1] / 2 + Origin[1],
					k * Spacing[2] + Spacing[2] / 2 + Origin[2]
				};
				const double dis = vtkMath::Distance2BetweenPoints(pixelPos, Center);
				auto* pixel = static_cast<uchar*>(holdImage->GetScalarPointer(i, j, k));
				if (dis < Radius * Radius && *pixel != 0)
				{
					std::vector<int> pos = {i, j, k};
					vector.push_back(pos);
				}
			}
		}
	}

	if (vector.size() < 10)
	{
		return false;
	}

	for (auto pos : vector)
	{
		auto* pixel = static_cast<uchar*>(holdImage->GetScalarPointer(pos[0], pos[1], pos[2]));
		*pixel = 0;
	}
	return true;
}

/**
 * 方形截取效果比圆形好
 */

void NTBooleanOperationFilter::UpdatePatchPolyData()
{
	double BallExtents[6];
	BallExtents[0] = Center[0] - 1.5 * Radius;
	BallExtents[1] = Center[0] + 1.5 * Radius;
	BallExtents[2] = Center[1] - 1.5 * Radius;
	BallExtents[3] = Center[1] + 1.5 * Radius;
	BallExtents[4] = Center[2] - 1.5 * Radius;
	BallExtents[5] = Center[2] + 1.5 * Radius;

	vtkNew<vtkBox> box;
	box->SetBounds(BallExtents);


	/*QTime time3;
	time3.start();

	vtkNew<NTClipPolyData> clipper1;
	std::thread t1([&]()
	{
		clipper1->SetInputData(holdPolyData);
		clipper1->SetClipFunction(box);
		clipper1->SetBeginAndEndPos(0, static_cast<int>(holdPolyData->GetNumberOfCells() / 2));
		clipper1->Update();
	});

	vtkNew<NTClipPolyData> clipper2;
	std::thread t2([&]()
	{
		vtkNew<vtkPolyData> copyPolyDta;
		copyPolyDta->DeepCopy(holdPolyData);
		clipper2->SetInputData(copyPolyDta);
		clipper2->SetClipFunction(box);
		clipper2->SetBeginAndEndPos(static_cast<int>(holdPolyData->GetNumberOfCells() / 2) + 1,
		                            holdPolyData->GetNumberOfCells());
		clipper2->Update();
	});
	t1.join();
	t2.join();

	vtkNew<vtkAppendPolyData> append;
	append->AddInputData(clipper1->GetOutput());
	append->AddInputData(clipper2->GetOutput());
	append->Update();

	cout << "Time3: " << time3.elapsed() << endl;*/


	// QTime time7;
	// time7.start();

	vtkNew<vtkClipPolyData> clipper3;
	clipper3->SetInputData(holdPolyData);
	clipper3->SetClipFunction(box);
	clipper3->Update();
	// cout << "Time7: " << time7.elapsed() << endl;

	patchPolyData->ShallowCopy(clipper3->GetOutput());
}


void NTBooleanOperationFilter::UpdateMidPolyData()
{
	double BallExtents[6];
	BallExtents[0] = Center[0] - 1.5 * Radius;
	BallExtents[1] = Center[0] + 1.5 * Radius;
	BallExtents[2] = Center[1] - 1.5 * Radius;
	BallExtents[3] = Center[1] + 1.5 * Radius;
	BallExtents[4] = Center[2] - 1.5 * Radius;
	BallExtents[5] = Center[2] + 1.5 * Radius;

	int CutExtents0[6];
	const int relax = 3;
	CutExtents0[0] = std::max(static_cast<int>((BallExtents[0] - Origin[0]) / Spacing[0] - relax), CutExtents[0]);
	CutExtents0[1] = std::min(static_cast<int>((BallExtents[1] - Origin[0]) / Spacing[0] + relax), CutExtents[1]);
	CutExtents0[2] = std::max(static_cast<int>((BallExtents[2] - Origin[1]) / Spacing[1] - relax), CutExtents[2]);
	CutExtents0[3] = std::min(static_cast<int>((BallExtents[3] - Origin[1]) / Spacing[1] + relax), CutExtents[3]);
	CutExtents0[4] = std::max(static_cast<int>((BallExtents[4] - Origin[2]) / Spacing[2] - relax), CutExtents[4]);
	CutExtents0[5] = std::min(static_cast<int>((BallExtents[5] - Origin[2]) / Spacing[2] + relax), CutExtents[5]);

	vtkNew<vtkExtractVOI> extractVOI;
	extractVOI->SetInputData(holdImage);
	extractVOI->SetVOI(CutExtents0);
	extractVOI->Update();

	CreateSurfaceFast(extractVOI->GetOutput(), midPolyData);

	double BigBallExtents[6];
	BigBallExtents[0] = BallExtents[0] - 0.1;
	BigBallExtents[1] = BallExtents[1] + 0.1;
	BigBallExtents[2] = BallExtents[2] - 0.1;
	BigBallExtents[3] = BallExtents[3] + 0.1;
	BigBallExtents[4] = BallExtents[4] - 0.1;
	BigBallExtents[5] = BallExtents[5] + 0.1;

	vtkNew<vtkBox> box;
	box->SetBounds(BigBallExtents);
	vtkNew<vtkClipPolyData> clipper2;
	clipper2->SetInputData(midPolyData);
	clipper2->SetClipFunction(box);
	clipper2->SetInsideOut(true);
	clipper2->Update();

	// vtkNew<vtkPolyDataConnectivityFilter> connecter;
	// connecter->SetInputData(clipper2->GetOutput());
	// connecter->SetExtractionModeToLargestRegion();
	// connecter->Update();
	midPolyData->ShallowCopy(clipper2->GetOutput());
	AddColor(midPolyData);
}

void NTBooleanOperationFilter::Reconstruction()
{
	double BallExtents[6];
	BallExtents[0] = Center[0] - 1.5 * Radius;
	BallExtents[1] = Center[0] + 1.5 * Radius;
	BallExtents[2] = Center[1] - 1.5 * Radius;
	BallExtents[3] = Center[1] + 1.5 * Radius;
	BallExtents[4] = Center[2] - 1.5 * Radius;
	BallExtents[5] = Center[2] + 1.5 * Radius;

	vtkNew<vtkBox> box;
	box->SetBounds(BallExtents);

	vtkNew<vtkClipPolyData> clipper1;
	clipper1->SetInputData(holdPolyData);
	clipper1->SetClipFunction(box);
	clipper1->GenerateClippedOutputOff();
	clipper1->Update();
	patchPolyData->ShallowCopy(clipper1->GetOutput());

	int CutExtents0[6];
	const int relax = 3;
	CutExtents0[0] = std::max(static_cast<int>((BallExtents[0] - Origin[0]) / Spacing[0] - relax), CutExtents[0]);
	CutExtents0[1] = std::min(static_cast<int>((BallExtents[1] - Origin[0]) / Spacing[0] + relax), CutExtents[1]);
	CutExtents0[2] = std::max(static_cast<int>((BallExtents[2] - Origin[1]) / Spacing[1] - relax), CutExtents[2]);
	CutExtents0[3] = std::min(static_cast<int>((BallExtents[3] - Origin[1]) / Spacing[1] + relax), CutExtents[3]);
	CutExtents0[4] = std::max(static_cast<int>((BallExtents[4] - Origin[2]) / Spacing[2] - relax), CutExtents[4]);
	CutExtents0[5] = std::min(static_cast<int>((BallExtents[5] - Origin[2]) / Spacing[2] + relax), CutExtents[5]);

	vtkNew<vtkExtractVOI> extractVOI;
	extractVOI->SetInputData(holdImage);
	extractVOI->SetVOI(CutExtents0);
	extractVOI->Update();
	CreateSurfaceFast(extractVOI->GetOutput(), midPolyData);

	double BigBallExtents[6];
	BigBallExtents[0] = BallExtents[0] - 0.1;
	BigBallExtents[1] = BallExtents[1] + 0.1;
	BigBallExtents[2] = BallExtents[2] - 0.1;
	BigBallExtents[3] = BallExtents[3] + 0.1;
	BigBallExtents[4] = BallExtents[4] - 0.1;
	BigBallExtents[5] = BallExtents[5] + 0.1;

	box->SetBounds(BigBallExtents);
	vtkNew<vtkClipPolyData> clipper2;
	clipper2->SetInputData(midPolyData);
	clipper2->SetClipFunction(box);
	clipper2->GenerateClippedOutputOn();
	clipper2->Update();
	midPolyData->ShallowCopy(clipper2->GetOutput(1));
	AddColor(midPolyData);
}

void NTBooleanOperationFilter::AddColor(vtkPolyData* mesh)
{
	if (!isCanAddColor)
	{
		return;
	}

	vtkNew<vtkUnsignedCharArray> cellColors;
	cellColors->SetNumberOfComponents(3);
	cellColors->SetNumberOfValues(mesh->GetNumberOfCells());

	// QTime time4;
	// time4.start();

	// std::thread t1([&]()
	// {
	for (int i = 0; i < mesh->GetNumberOfCells(); ++i)
	{
		vtkCell* cell = mesh->GetCell(i);
		int subId;
		double pcoords[3], x[3], weights[256];
		cell->GetParametricCenter(pcoords);
		cell->EvaluateLocation(subId, pcoords, x, weights);

		if (implantFilter->EvaluateFunction(x) < 0)
		{
			cellColors->InsertTypedTuple(i, green);
		}
		else if (remainBoneFilter->EvaluateFunction(x) < -0.5)
		{
			cellColors->InsertTypedTuple(i, red);
		}
		else
		{
			cellColors->InsertTypedTuple(i, white);
		}
	}
	// });


	// std::thread t2([&]()
	// {
	// 	vtkNew<vtkPolyData> copyPolyDta;
	// 	copyPolyDta->DeepCopy(mesh);
	//
	// 	for (int i = static_cast<int>(mesh->GetNumberOfCells() / 2) + 1; i < mesh->GetNumberOfCells(); ++i)
	// 	{
	// 		vtkCell* cell = copyPolyDta->GetCell(i);
	// 		int subId;
	// 		double pcoords[3], x[3], weights[256];
	// 		cell->GetParametricCenter(pcoords);
	// 		cell->EvaluateLocation(subId, pcoords, x, weights);
	//
	// 		if (implantFilter_copy->EvaluateFunction(x) < 0)
	// 		{
	// 			cellColors->InsertTypedTuple(i, green);
	// 		}
	// 		else if (remainBoneFilter_copy->EvaluateFunction(x) < -0.5)
	// 		{
	// 			cellColors->InsertTypedTuple(i, red);
	// 		}
	// 		else
	// 		{
	// 			cellColors->InsertTypedTuple(i, white);
	// 		}
	// 	}
	// });

	// t1.join();
	// t2.join();

	mesh->GetCellData()->SetScalars(cellColors);

	// cout << "Time4: " << time4.elapsed() << endl;
}


void NTBooleanOperationFilter::CreateSurface(vtkImageData* vtkImage, vtkPolyData* outSurface)
{
	vtkNew<vtkMarchingCubes> skinExtractor;
	skinExtractor->ComputeScalarsOff();
	skinExtractor->SetInputData(vtkImage);
	skinExtractor->SetValue(0, 1.0);
	skinExtractor->Update();

	vtkNew<vtkPolyDataConnectivityFilter> connecter;
	connecter->SetInputData(skinExtractor->GetOutput());
	connecter->SetExtractionModeToLargestRegion();
	connecter->Update();

	vtkNew<vtkSmoothPolyDataFilter> smoother;
	smoother->SetInputData(connecter->GetOutput());
	smoother->SetNumberOfIterations(50);
	smoother->SetRelaxationFactor(0.1);
	smoother->SetFeatureAngle(60);
	smoother->FeatureEdgeSmoothingOff();
	smoother->BoundarySmoothingOff();
	smoother->SetConvergence(0);
	smoother->Update();

	vtkNew<vtkDecimatePro> decimate;
	decimate->SplittingOff();
	decimate->SetErrorIsAbsolute(5);
	decimate->SetFeatureAngle(30);
	decimate->PreserveTopologyOn();
	decimate->BoundaryVertexDeletionOff();
	decimate->SetDegree(10); // std-value is 25!
	decimate->SetInputData(smoother->GetOutput()); // RC++
	decimate->SetTargetReduction(0);
	decimate->SetMaximumError(0.002);
	decimate->Update();

	vtkNew<vtkPolyDataNormals> normalsGenerator;
	normalsGenerator->SetInputData(decimate->GetOutput());
	// normalsGenerator->FlipNormalsOn();
	normalsGenerator->Update();

	vtkNew<vtkCleanPolyData> cleaner;
	cleaner->SetInputData(normalsGenerator->GetOutput());
	cleaner->PieceInvariantOff();
	cleaner->ConvertLinesToPointsOff();
	cleaner->ConvertPolysToLinesOff();
	cleaner->ConvertStripsToPolysOff();
	cleaner->PointMergingOn();
	cleaner->Update();

	outSurface->ShallowCopy(cleaner->GetOutput());
}


void NTBooleanOperationFilter::CreateSurfaceFast(vtkImageData* vtkImage, vtkPolyData* outSurface)
{
        vtkNew<vtkDiscreteFlyingEdges3D> skinExtractor;
        skinExtractor->SetComputeGradients(false);
        skinExtractor->SetComputeNormals(false);
        skinExtractor->SetComputeScalars(true);
	skinExtractor->SetInputData(vtkImage);
        skinExtractor->SetNumberOfContours(1);
	skinExtractor->SetValue(0, 1.0);
	skinExtractor->Update();

	vtkNew<vtkSmoothPolyDataFilter> smoother;
	smoother->SetInputData(skinExtractor->GetOutput());
	smoother->SetNumberOfIterations(10);
	smoother->SetRelaxationFactor(0.5);
	smoother->SetFeatureAngle(60);
	smoother->BoundarySmoothingOff();
	//smoother->SetConvergence(0);
	smoother->Update();

	vtkNew<vtkPolyDataConnectivityFilter> connecter;
	connecter->SetInputData(smoother->GetOutput());
	connecter->SetExtractionModeToLargestRegion();
	connecter->Update();

	vtkNew<vtkPolyDataNormals> normalsGenerator;
	normalsGenerator->SetInputData(connecter->GetOutput());
	// normalsGenerator->FlipNormalsOn();
	normalsGenerator->Update();

	outSurface->ShallowCopy(normalsGenerator->GetOutput());
}

void NTBooleanOperationFilter::GetROIExtents(vtkImageData* input, int (&ROIExtents)[6])
{
	int Extents[6];
	input->GetExtent(Extents);

	vtkNew<vtkPolyData> origin;

	CreateSurfaceFast(input, origin);

	double surfaceBounds[6];
	origin->GetBounds(surfaceBounds);

	const int relax = 3;
	ROIExtents[0] = std::max(static_cast<int>((surfaceBounds[0] - Origin[0]) / Spacing[0] - relax), Extents[0]);
	ROIExtents[1] = std::min(static_cast<int>((surfaceBounds[1] - Origin[0]) / Spacing[0] + relax), Extents[1]);
	ROIExtents[2] = std::max(static_cast<int>((surfaceBounds[2] - Origin[1]) / Spacing[1] - relax), Extents[2]);
	ROIExtents[3] = std::min(static_cast<int>((surfaceBounds[3] - Origin[1]) / Spacing[1] + relax), Extents[3]);
	ROIExtents[4] = std::max(static_cast<int>((surfaceBounds[4] - Origin[2]) / Spacing[2] - relax), Extents[4]);
	ROIExtents[5] = std::min(static_cast<int>((surfaceBounds[5] - Origin[2]) / Spacing[2] + relax), Extents[5]);

	const int boneLength = 60; //骨头保留长度 单位：mm

	if (this->boneType == Tibia)
	{
		// ROIExtents[4] = ROIExtents[5] - (ROIExtents[5] - ROIExtents[4]) / 2;
		ROIExtents[4] = std::max(ROIExtents[4], static_cast<int>(ROIExtents[5] - boneLength / Spacing[2]));
	}
	else if (this->boneType == Femur)
	{
		// ROIExtents[5] = ROIExtents[4] + (ROIExtents[5] - ROIExtents[4]) / 2;
		ROIExtents[5] = std::min(ROIExtents[5], static_cast<int>(ROIExtents[4] + boneLength / Spacing[2]));
	}
}

/*
void NTBooleanOperationFilter::UpdatePatchPolyData2()
{
	QTime time3;
	time3.start();

	double BallExtents[6];
	BallExtents[0] = Center[0] - 1.5 * Radius;
	BallExtents[1] = Center[0] + 1.5 * Radius;
	BallExtents[2] = Center[1] - 1.5 * Radius;
	BallExtents[3] = Center[1] + 1.5 * Radius;
	BallExtents[4] = Center[2] - 1.5 * Radius;
	BallExtents[5] = Center[2] + 1.5 * Radius;

	vtkNew<vtkBox> box;
	box->SetBounds(BallExtents);


	vtkNew<vtkKdTree> kDTree;
	kDTree->BuildLocatorFromPoints(holdPolyData);

	vtkNew<vtkIdList> pds;
	kDTree->FindPointsWithinRadius(2 * Radius, Center, pds);

	vtkNew<vtkIdList> cds_in;
	vtkNew<vtkIdList> cds_out;

	for (int i = 0; i < pds->GetNumberOfIds(); ++i)
	{
		vtkNew<vtkIdList> cells;
		holdPolyData->GetPointCells(pds->GetId(i), cells);
		for (int j = 0; j < cells->GetNumberOfIds(); ++j)
		{
			if (cds_in->IsId(cells->GetId(j)) == -1)
			{
				cds_in->InsertNextId(cells->GetId(j));
			}
		}
	}
	for (int i = 0; i < holdPolyData->GetNumberOfCells(); ++i)
	{
		if (cds_in->IsId(i) == -1)
		{
			cds_out->InsertNextId(i);
		}
	}

	vtkDataSetAttributes::FieldList pointFields(1);
	pointFields.InitializeFieldList(holdPolyData->GetPointData());
	vtkDataSetAttributes::FieldList cellFields(1);
	cellFields.InitializeFieldList(holdPolyData->GetCellData());

	vtkNew<vtkPolyData> holdPolyData_in;
	holdPolyData_in->Allocate(holdPolyData);
	holdPolyData_in->GetPointData()->CopyAllocate(pointFields);
	holdPolyData_in->GetCellData()->CopyAllocate(cellFields);
	CopyCells(holdPolyData, holdPolyData_in, 0, pointFields, cellFields, cds_in, false);

	vtkNew<vtkPolyData> holdPolyData_out;
	holdPolyData_out->Allocate(holdPolyData);
	holdPolyData_out->GetPointData()->CopyAllocate(pointFields);
	holdPolyData_out->GetCellData()->CopyAllocate(cellFields);
	CopyCells(holdPolyData, holdPolyData_out, 0, pointFields, cellFields, cds_out, false);


	vtkNew<vtkClipPolyData> clipper1;
	clipper1->SetInputData(holdPolyData_in);
	clipper1->SetClipFunction(box);
	clipper1->Update();

	vtkNew<vtkAppendPolyData> append;
	append->AddInputData(clipper1->GetOutput());
	append->AddInputData(holdPolyData_out);
	append->Update();

	cout << "Time3: " << time3.elapsed() << endl;

	patchPolyData->ShallowCopy(append->GetOutput());
}
*/

/*
void NTBooleanOperationFilter::Reconstruction()
{
	const bool isInXCut = Center[0] - 1.5 * Radius > BallExtents[0] && Center[0] + 1.5 * Radius < BallExtents[1];
	const bool isInYCut = Center[1] - 1.5 * Radius > BallExtents[1] && Center[1] + 1.5 * Radius < BallExtents[1];
	const bool isInZCut = Center[2] - 1.5 * Radius > BallExtents[2] && Center[2] + 1.5 * Radius < BallExtents[2];
	if (!(isInXCut && isInYCut && isInZCut))
	{
		BallExtents[0] = Center[0] - 3 * Radius;
		BallExtents[1] = Center[0] + 3 * Radius;
		BallExtents[2] = Center[1] - 3 * Radius;
		BallExtents[3] = Center[1] + 3 * Radius;
		BallExtents[4] = Center[2] - 3 * Radius;
		BallExtents[5] = Center[2] + 3 * Radius;

		vtkNew<vtkBox> box;
		box->SetBounds(BallExtents);
		vtkNew<vtkClipPolyData> clipPolyData;
		clipPolyData->SetInputData(holdPolyData);
		clipPolyData->SetClipFunction(box);
		clipPolyData->GenerateClippedOutputOff();
		clipPolyData->Update();
		patchPolyData->ShallowCopy(clipPolyData->GetOutput());
	}

	int CutExtents0[6];
	CutExtents0[0] = std::max(static_cast<int>((BallExtents[0] - Origin[0]) / Spacing[0] - relax), CutExtents[0]);
	CutExtents0[1] = std::min(static_cast<int>((BallExtents[1] - Origin[0]) / Spacing[0] + relax), CutExtents[1]);
	CutExtents0[2] = std::max(static_cast<int>((BallExtents[2] - Origin[1]) / Spacing[1] - relax), CutExtents[2]);
	CutExtents0[3] = std::min(static_cast<int>((BallExtents[3] - Origin[1]) / Spacing[1] + relax), CutExtents[3]);
	CutExtents0[4] = std::max(static_cast<int>((BallExtents[4] - Origin[2]) / Spacing[2] - relax), CutExtents[4]);
	CutExtents0[5] = std::min(static_cast<int>((BallExtents[5] - Origin[2]) / Spacing[2] + relax), CutExtents[5]);

	vtkNew<vtkExtractVOI> extractVOI;
	extractVOI->SetInputData(holdImage);
	extractVOI->SetVOI(CutExtents0);
	extractVOI->Update();
	CreateSurfaceFast(extractVOI->GetOutput(), midPolyData);

	double BallBigExtents[6];
	BallBigExtents[0] = BallExtents[0] - 0.05;
	BallBigExtents[1] = BallExtents[1] + 0.05;
	BallBigExtents[2] = BallExtents[2] - 0.05;
	BallBigExtents[3] = BallExtents[3] + 0.05;
	BallBigExtents[4] = BallExtents[4] - 0.05;
	BallBigExtents[5] = BallExtents[5] + 0.05;
	vtkNew<vtkBox> box;
	box->SetBounds(BallBigExtents);
	vtkNew<vtkClipPolyData> clipPolyData;
	clipPolyData->SetInputData(midPolyData);
	clipPolyData->SetClipFunction(box);
	clipPolyData->GenerateClippedOutputOn();
	clipPolyData->Update();
	midPolyData->ShallowCopy(clipPolyData->GetOutput(1));
}

void NTBooleanOperationFilter::Reconstruction2()
{
	vtkNew<vtkSphere> ball;
	ball->SetCenter(Center);
	ball->SetRadius(Radius + 0.4);

	vtkNew<vtkClipPolyData> clipPolyData;
	clipPolyData->SetInputData(holdPolyData);
	clipPolyData->SetClipFunction(ball);
	clipPolyData->GenerateClippedOutputOff();
	clipPolyData->Update();
	patchPolyData->ShallowCopy(clipPolyData->GetOutput());

	int CutExtents0[6];
	CutExtents0[0] = std::max(static_cast<int>((Center[0] - Origin[0]) / Spacing[0] - 2 * nPixel - relax),
	                          CutExtents[0]);
	CutExtents0[1] = std::min(static_cast<int>((Center[0] - Origin[0]) / Spacing[0] + 2 * nPixel + relax),
	                          CutExtents[1]);
	CutExtents0[2] = std::max(static_cast<int>((Center[1] - Origin[1]) / Spacing[1] - 2 * nPixel - relax),
	                          CutExtents[2]);
	CutExtents0[3] = std::min(static_cast<int>((Center[1] - Origin[1]) / Spacing[1] + 2 * nPixel + relax),
	                          CutExtents[3]);
	CutExtents0[4] = std::max(static_cast<int>((Center[2] - Origin[2]) / Spacing[2] - 2 * nPixel - relax),
	                          CutExtents[4]);
	CutExtents0[5] = std::min(static_cast<int>((Center[2] - Origin[2]) / Spacing[2] + 2 * nPixel + relax),
	                          CutExtents[5]);

	vtkNew<vtkExtractVOI> extractVOI;
	extractVOI->SetInputData(holdImage);
	extractVOI->SetVOI(CutExtents0);
	extractVOI->Update();
	CreateSurfaceFast(extractVOI->GetOutput(), midPolyData);

	ball->SetRadius(Radius + 0.5);
	vtkNew<vtkClipPolyData> clipPolyData2;
	clipPolyData2->SetInputData(midPolyData);
	clipPolyData2->SetClipFunction(ball);
	clipPolyData2->GenerateClippedOutputOn();
	clipPolyData2->Update();
	midPolyData->ShallowCopy(clipPolyData2->GetOutput(1));
}*/


/*
void NTBooleanOperationFilter8::LargestRegionFilter(vtkImageData* input)
{
	int Dims[3];
	input->GetDimensions(Dims);

	typedef itk::Image<unsigned char, 3> UCharImageType;

	UCharImageType::Pointer image = UCharImageType::New();
	itk::Size<3> size{static_cast<ulong>(Dims[0]), static_cast<ulong>(Dims[1]), static_cast<ulong>(Dims[2])};
	itk::ImageRegion<3> region(size);
	image->SetRegions(region);
	image->Allocate();
	image->FillBuffer(0);

	for (int i = 0; i < Dims[0]; ++i)
	{
		for (int j = 0; j < Dims[1]; ++j)
		{
			for (int k = 0; k < Dims[2]; ++k)
			{
				auto* pixel0 = static_cast<unsigned char*>(input->GetScalarPointer(i, j, k));
				itk::Index<3> pixel1{i, j, k};
				if (*pixel0 != 0)
				{
					image->SetPixel(pixel1, 1);
				}
				else
				{
					image->SetPixel(pixel1, 0);
				}
			}
		}
	}

	typedef itk::ConnectedComponentImageFilter<UCharImageType, UCharImageType> ConnectedComponentFilterType;
	ConnectedComponentFilterType::Pointer connectedComponentFilter = ConnectedComponentFilterType::New();
	connectedComponentFilter->SetInput(image);
	connectedComponentFilter->Update();

	typedef itk::LabelShapeKeepNObjectsImageFilter<UCharImageType> LabelShapeKeepNObjectsFilterType;
	LabelShapeKeepNObjectsFilterType::Pointer labelShapeKeepNObjectFilter = LabelShapeKeepNObjectsFilterType::New();
	labelShapeKeepNObjectFilter->SetInput(connectedComponentFilter->GetOutput());
	labelShapeKeepNObjectFilter->SetBackgroundValue(0);
	labelShapeKeepNObjectFilter->SetNumberOfObjects(1);
	labelShapeKeepNObjectFilter->SetAttribute(LabelShapeKeepNObjectsFilterType::LabelObjectType::NUMBER_OF_PIXELS);
	labelShapeKeepNObjectFilter->Update();

	itk::ImageRegionIterator<UCharImageType> itLargestComponent(labelShapeKeepNObjectFilter->GetOutput(),
																labelShapeKeepNObjectFilter->GetOutput()->
																GetLargestPossibleRegion());
	while (!itLargestComponent.IsAtEnd())
	{
		if (itLargestComponent.Get())
		{
			itLargestComponent.Set(1);
		}
		itLargestComponent.operator++();
	}

	typedef itk::CastImageFilter<UCharImageType, UCharImageType> CastFilterType;
	CastFilterType::Pointer caster = CastFilterType::New();
	caster->SetInput(labelShapeKeepNObjectFilter->GetOutput());
	caster->Update();

	UCharImageType::Pointer largestImage = caster->GetOutput();

	for (int i = 0; i < Dims[0]; ++i)
	{
		for (int j = 0; j < Dims[1]; ++j)
		{
			for (int k = 0; k < Dims[2]; ++k)
			{
				auto* pixel0 = static_cast<unsigned char*>(input->GetScalarPointer(i, j, k));
				itk::Index<3> pixel1{i, j, k};

				if (largestImage->GetPixel(pixel1) != 0)
				{
					*pixel0 = 255;
				}
				else
				{
					*pixel0 = 0;
				}
			}
		}
	}
}

void NTBooleanOperationFilter8::GetHaveValueExtents(vtkImageData* input, int (&CutExtents)[6])
{
	int Dims[3];
	input->GetDimensions(Dims);

	CutExtents[0] = Dims[0];
	CutExtents[1] = 0;
	CutExtents[2] = Dims[1];
	CutExtents[3] = 0;
	CutExtents[4] = Dims[2];
	CutExtents[5] = 0;
	for (int i = 0; i < Dims[0]; i++)
	{
		for (int j = 0; j < Dims[1]; j++)
		{
			for (int k = 0; k < Dims[2]; k++)
			{
				auto* pixel = static_cast<unsigned char*>(input->GetScalarPointer(i, j, k));
				if (*pixel != 0)
				{
					if (i <= CutExtents[0])
					{
						CutExtents[0] = i;
					}
					if (i >= CutExtents[1])
					{
						CutExtents[1] = i;
					}
					if (j <= CutExtents[2])
					{
						CutExtents[2] = j;
					}
					if (j >= CutExtents[3])
					{
						CutExtents[3] = j;
					}
					if (k <= CutExtents[4])
					{
						CutExtents[4] = k;
					}
					if (k >= CutExtents[5])
					{
						CutExtents[5] = k;
					}
				}
			}
		}
	}
	const int relax = 5;
	CutExtents[0] = std::max(0, CutExtents[0] - relax);
	CutExtents[1] = std::min(Dims[0] - 1, CutExtents[1] + relax);
	CutExtents[2] = std::max(0, CutExtents[2] - relax);
	CutExtents[3] = std::min(Dims[1] - 1, CutExtents[3] + relax);
	CutExtents[4] = std::max(0, CutExtents[4] - relax);
	CutExtents[5] = std::min(Dims[2] - 1, CutExtents[5] + relax);
}
*/
