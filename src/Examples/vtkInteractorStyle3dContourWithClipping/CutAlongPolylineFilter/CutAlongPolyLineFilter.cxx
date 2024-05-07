/*
Author:		Wong, Matthew Lun
Date:		22, September 2016
Occupation:	Chinese University of Hong Kong,
			Department of Imaging and Inteventional Radiology,
			Junior Research Assistant

FileName: 	CutAlongPolyLineFilter.cxx

Descriptions:
	This class with itch away cells of the input polydata along an input polyline


Wong Matthew Lun
Copyright (C) 2016
*/

#include <vtkDemandDrivenPipeline.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkDataObject.h>
#include <vtkTriangleFilter.h>
#include <vtkClipPolyData.h>
#include <vtkImplicitPolyDataDistance.h>
#include <vtkCleanPolyData.h>

#include "vtkLoopedTubeFilter.h"
#include "CutAlongPolyLineFilter.h"

vtkStandardNewMacro(CutAlongPolyLineFilter);


void CutAlongPolyLineFilter::PrintSelf(ostream & os, vtkIndent indent)
{
	Superclass::PrintSelf(os, indent);
}

CutAlongPolyLineFilter::CutAlongPolyLineFilter()
{
	this->ClipTolerance = 0.5;
	this->SetNumberOfInputPorts(2);
	this->SetNumberOfOutputPorts(1);
}

CutAlongPolyLineFilter::~CutAlongPolyLineFilter()
{
}
#include <vtkXMLPolyDataWriter.h>
int CutAlongPolyLineFilter::RequestData(vtkInformation* vtkNotUsed(request), vtkInformationVector ** inputVector, vtkInformationVector * outputVector)
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

	vtkSmartPointer<vtkCleanPolyData> cleaner = vtkSmartPointer<vtkCleanPolyData>::New();
	cleaner->SetInputData(cutLine);
	cleaner->SetTolerance(0.01);
	cleaner->Update();

	// create a tube around the line
	vtkSmartPointer<vtkLoopedTubeFilter> tubefilter = vtkSmartPointer<vtkLoopedTubeFilter>::New();
	tubefilter->SetInputConnection(cleaner->GetOutputPort());
	tubefilter->SetCapping(0);
	tubefilter->SetNumberOfSides(10);
	tubefilter->SetRadius(this->ClipTolerance);
	tubefilter->Update();

	// Write the file
	vtkSmartPointer<vtkXMLPolyDataWriter> writer =
		vtkSmartPointer<vtkXMLPolyDataWriter>::New();
	writer->SetFileName("test.vtp");
#if VTK_MAJOR_VERSION <= 5
	writer->SetInput(polydata);
#else
	writer->SetInputData(tubefilter->GetOutput());
#endif

	// Optional - set the mode. The default is binary.
	//writer->SetDataModeToBinary();
	//writer->SetDataModeToAscii();

	writer->Write();

	// push pds through triangle filters
	vtkSmartPointer<vtkTriangleFilter> tubeTri = vtkSmartPointer<vtkTriangleFilter>::New();
	vtkSmartPointer<vtkTriangleFilter> dataTri = vtkSmartPointer<vtkTriangleFilter>::New();
	tubeTri->SetInputConnection(tubefilter->GetOutputPort());
	tubeTri->Update();
	dataTri->SetInputData(inputPD);
	dataTri->Update();


	vtkSmartPointer<vtkImplicitPolyDataDistance> clipFunc = vtkSmartPointer<vtkImplicitPolyDataDistance>::New();
	clipFunc->SetInput(tubeTri->GetOutput());
	clipFunc->SetTolerance(0.1);
	
	vtkSmartPointer<vtkClipPolyData> clipper = vtkSmartPointer<vtkClipPolyData>::New();
	clipper->SetInputConnection(dataTri->GetOutputPort());
	clipper->SetClipFunction(clipFunc);
	clipper->Update();
	
	clipper->GetOutput()->Print(cout);

	outputPD->DeepCopy(clipper->GetOutput());
	return 1;
}

//int CutAlongPolyLineFilter::FillOutputPortInformation(int vtkNotUsed(port), vtkInformation * info)
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
int CutAlongPolyLineFilter::FillInputPortInformation(int port, vtkInformation * info)
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
