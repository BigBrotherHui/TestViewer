/*
Author:		Wong, Matthew Lun
Date:		22, September 2016
Occupation:	Chinese University of Hong Kong,
			Department of Imaging and Inteventional Radiology,
			Junior Research Assistant

FileName: 	CutAlongPolyLineFilter2.cxx

Descriptions:
	This class with itch away cells of the input polydata along an input polyline


Wong Matthew Lun
Copyright (C) 2016
*/

#include <vtkDemandDrivenPipeline.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkDataObject.h>
#include <vtkTubeFilter.h>
#include <vtkClipPolyData.h>
#include <vtkImplicitPolyDataDistance.h>
#include <vtkLineSource.h>
#include <vtkTriangleFilter.h>
#include <vtkImplicitBoolean.h>


#include "CutAlongPolyLineFilter2.h"

vtkStandardNewMacro(CutAlongPolyLineFilter2);


void CutAlongPolyLineFilter2::PrintSelf(ostream & os, vtkIndent indent)
{
	Superclass::PrintSelf(os, indent);
}

CutAlongPolyLineFilter2::CutAlongPolyLineFilter2()
{
	this->ClipTolerance = 0.5;
	this->SetNumberOfInputPorts(2);
	this->SetNumberOfOutputPorts(1);
}

CutAlongPolyLineFilter2::~CutAlongPolyLineFilter2()
{
}

int CutAlongPolyLineFilter2::RequestData(vtkInformation* vtkNotUsed(request), vtkInformationVector ** inputVector, vtkInformationVector * outputVector)
{
	vtkInformation* inInfo0 = inputVector[0]->GetInformationObject(0);
	vtkInformation* inInfo1 = inputVector[1]->GetInformationObject(0);
	vtkInformation* outPolyDataInfo = outputVector->GetInformationObject(0);
	
	vtkPolyData* inputPD = vtkPolyData::SafeDownCast(
		inInfo0->Get(vtkDataObject::DATA_OBJECT()));

	vtkPolyData* cutLine = vtkPolyData::SafeDownCast(
		inInfo1->Get(vtkDataObject::DATA_OBJECT()));

	vtkPolyData* outputPD = vtkPolyData::SafeDownCast(
		outPolyDataInfo->Get(vtkDataObject::DATA_OBJECT()));

	vtkSmartPointer<vtkImplicitBoolean> implicitBoolean =
		vtkSmartPointer<vtkImplicitBoolean>::New();
	implicitBoolean->SetOperationTypeToUnion();

	vtkPoints* points = cutLine->GetPoints();
	for (vtkIdType pointId = 1; pointId < points->GetNumberOfPoints(); ++pointId) {
		vtkSmartPointer<vtkLineSource> lineSource =
			vtkSmartPointer<vtkLineSource>::New();
		lineSource->SetPoint1(points->GetPoint(pointId - 1));
		lineSource->SetPoint2(points->GetPoint(pointId));
		lineSource->Update();
		// create a tube around the line
		vtkSmartPointer<vtkTubeFilter> tubefilter = vtkSmartPointer<vtkTubeFilter>::New();
		tubefilter->SetInputConnection(lineSource->GetOutputPort());
		tubefilter->SetCapping(0);
		tubefilter->SetNumberOfSides(3);
		tubefilter->SetRadius(this->ClipTolerance);
		tubefilter->Update();

		vtkSmartPointer<vtkImplicitPolyDataDistance> clipFunc =
			vtkSmartPointer<vtkImplicitPolyDataDistance>::New();
		clipFunc->SetInput(tubefilter->GetOutput());
		
		implicitBoolean->AddFunction(clipFunc);
	}
	// push pds through triangle filters
	vtkSmartPointer<vtkTriangleFilter> dataTri = vtkSmartPointer<vtkTriangleFilter>::New();
	dataTri->SetInputData(inputPD);
	dataTri->Update();

	
	vtkSmartPointer<vtkClipPolyData> clipper = vtkSmartPointer<vtkClipPolyData>::New();
	clipper->SetInputConnection(dataTri->GetOutputPort());
	clipper->SetClipFunction(implicitBoolean);
	clipper->Update();
	clipper->GetOutput()->Print(cout);

	outputPD->DeepCopy(clipper->GetOutput());
	return 1;
}

//int CutAlongPolyLineFilter2::FillOutputPortInformation(int vtkNotUsed(port), vtkInformation * info)
//{
//	info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
//	return 1;
//}

/*
Descriptions: 
	The input must consist of two components polydata,
for port 0 input the surface to be cut, and for port 1
input the polyline you want to cut along.
*/
int CutAlongPolyLineFilter2::FillInputPortInformation(int port, vtkInformation * info)
{
	if (!this->Superclass::FillInputPortInformation(port, info))
	{
		return 0;
	}
	if (port == 0)
	{
		// input polydata of surface to clip here
		info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
	}
	else if (port == 1)
	{
		// input polyline here
		info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
	}
	return 1;
}
