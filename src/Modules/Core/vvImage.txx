#include <itkImageToVTKImageFilter.h>

//--------------------------------------------------------------------
template<class TItkImageType>
void vvImage::AddItkImage(TItkImageType *input)
{
  // Update input before conversion to enable exceptions thrown by the ITK pipeline.
  // Otherwise, vtkImageImport catches the exception for us.
  input->Update();
  // Convert from ITK object to VTK object
  mImageDimension = TItkImageType::ImageDimension;
  typedef itk::ImageToVTKImageFilter <TItkImageType> ConverterType;
  typename ConverterType::Pointer converter = ConverterType::New();
  mItkToVtkConverters.push_back(dynamic_cast< itk::ProcessObject *>(converter.GetPointer()));
  converter->SetInput(input);
  converter->Update();
  mVtkImages.push_back( converter->GetOutput() );
  // Account for direction in transform. The offset is already accounted for
  // in the VTK image coordinates, no need to put it in the transform.
  vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
  matrix->Identity();
  for(unsigned int i=0; i<input->GetImageDimension(); i++) {
    for(unsigned int j=0; j<input->GetImageDimension(); j++) {
      matrix->SetElement(i,j,input->GetDirection()[i][j]);
      // Direction is used around the image origin in ITK
      matrix->SetElement(i,3,matrix->GetElement(i,3) - matrix->GetElement(i,j) * input->GetOrigin()[j]);
    }
    matrix->SetElement(i,3,matrix->GetElement(i,3) + input->GetOrigin()[i]);
  }

  // GetDirection provides the forward transform, vtkImageReslice wants the inverse
  matrix->Invert();

  mTransform.push_back(vtkSmartPointer<vtkTransform>::New());
  mTransform.back()->SetMatrix(matrix);
  //META DATA
  mDictionary.push_back(&(input->GetMetaDataDictionary()));
}
//--------------------------------------------------------------------

