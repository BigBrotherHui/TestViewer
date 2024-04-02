
#ifndef VVIMAGE_H
#define VVIMAGE_H

#include <iostream>
#include <vector>
#include <itkObjectFactory.h>
#include <itkProcessObject.h>

#define VTK_EXCLUDE_STRSTREAM_HEADERS
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include "CoreExports.h"
class vtkImageData;

//------------------------------------------------------------------------------
class Core_EXPORT vvImage : public itk::LightObject {
public :
  typedef vvImage Self;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::ProcessObject::Pointer ConverterPointer;
  itkNewMacro(Self);

  void Init();
  void Reset();
  template<class TItkImageType> void AddItkImage(TItkImageType *input);
  void AddVtkImage(vtkImageData* input);
  const std::vector<vtkImageData*>& GetVTKImages();
  vtkImageData* GetFirstVTKImageData();
  int GetNumberOfDimensions() const;
  int GetNumberOfSpatialDimensions();
  void GetScalarRange(double* range);
  unsigned long GetActualMemorySize();
  std::vector<double> GetSpacing();
  std::vector<double> GetOrigin() const;
  std::vector<int> GetSize();
  std::string GetScalarTypeAsITKString();
  int GetNumberOfScalarComponents();
  int GetScalarSize();
  bool IsTimeSequence() const;
  bool IsScalarTypeInteger();
  bool IsScalarTypeInteger(int t);
  const std::vector< vtkSmartPointer<vtkTransform> >& GetTransform();
  void SetTimeSpacing(double s) { mTimeSpacing = s; }
  void SetTimeOrigin(double o) { mTimeOrigin = o; }
  bool HaveSameSizeAndSpacingThan(vvImage * other);
  //META DATA
  itk::MetaDataDictionary* GetFirstMetaDataDictionary();

private:
  vvImage();
  ~vvImage();

  std::vector< ConverterPointer > mItkToVtkConverters;
  std::vector< vtkImageData* > mVtkImages;
  std::vector< vtkSmartPointer<vtkTransform> > mTransform;
  //META DATA
  std::vector< itk::MetaDataDictionary* > mDictionary;

  double mTimeOrigin;
  double mTimeSpacing;
  unsigned int mImageDimension;
};
//------------------------------------------------------------------------------

#include "vvImage.txx"

#endif
