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

#ifndef CUT_ALONG_POLYLINE_FILTER_H
#define CUT_ALONG_POLYLINE_FILTER_H

#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkInformation.h>

class CutAlongPolyLineFilter : public vtkPolyDataAlgorithm
{
public:
	static CutAlongPolyLineFilter* New();
	vtkTypeMacro(CutAlongPolyLineFilter, vtkPolyDataAlgorithm);
	virtual void PrintSelf(ostream& os, vtkIndent indent);

	vtkSetMacro(ClipTolerance, double);
	vtkGetMacro(ClipTolerance, double);


protected:
	CutAlongPolyLineFilter();
	~CutAlongPolyLineFilter();

	int RequestData(
		vtkInformation* request,
		vtkInformationVector** inputVector,
		vtkInformationVector* outputVector);

	//int FillOutputPortInformation(int port, vtkInformation* info);
	int FillInputPortInformation(int port, vtkInformation* info);

private:
	double ClipTolerance;

	CutAlongPolyLineFilter(const CutAlongPolyLineFilter&); // Not implemented.
	void operator = (const CutAlongPolyLineFilter&);  // Not implemented.

};

#endif