/*=========================================================================

Program:   Insight Segmentation & Registration Toolkit
Module:    getDRRSiddonJacobsRayTracing.cxx
Language:  C++
Date:      2010/12/15
Version:   1.0
Author:    Jian Wu (eewujian@hotmail.com)
           Univerisity of Florida
		   Virginia Commonwealth University

This program compute digitally reconstructed radiograph (DRR) by projecting
a 3D CT image into a 2D image plane using Siddon-Jacob ray-tracing algorithm.

This program was modified from the ITK application--GenerateProjection.cxx

-------------------------------------------------------------------------
References:

R. L. Siddon, "Fast calculation of the exact radiological path for a
threedimensional CT array," Medical Physics 12, 252-55 (1985).

F. Jacobs, E. Sundermann, B. De Sutter, M. Christiaens, and I. Lemahieu,
"A fast algorithm to calculate the exact radiological path through a pixel
or voxel space," Journal of Computing and Information Technology ?
CIT 6, 89-94 (1998).

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif


// This example illustrates the use of the ResampleImageFilter and 
// SiddonJacobsRayCastInterpolateImageFunction to generate digitally 
// reconstructed radiographs (DRRs) from a 3D CT image volume.

// The program attempts to generate the simulated x-ray images that can 
// be acquired when an imager is attached to a linear accelerator.

#include "itkTimeProbesCollectorBase.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkResampleImageFilter.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkFlipImageFilter.h"

#include "itkEuler3DTransform.h"
#include "itkSiddonJacobsRayCastInterpolateImageFunction.h"
#include "itkIntensityWindowingImageFilter.h"


void usage()
{
	std::cerr << "\n";
	std::cerr << "Usage: getDRRSiddonJacobsRayTracing <options> [input]\n";
	std::cerr << "       calculates the Digitally Reconstructed Radiograph from  \n";
	std::cerr << "       a CT image using Siddon-Jacob ray-tracing algorithm. \n\n";
	std::cerr << "   where <options> is one or more of the following:\n\n";
	std::cerr << "       <-h>                     Display (this) usage information\n";
	std::cerr << "       <-v>                     Verbose output [default: no]\n";
	std::cerr << "       <-res float float>       DRR Pixel spacing in isocenter plane in mm [default: 0.51mm 0.51mm]  \n";
	std::cerr << "       <-size int int>          Size of DRR in number of pixels [default: 512x512]  \n";
	std::cerr << "       <-scd float>             Source to isocenter (i.e., 3D image center) distance in mm [default: 1000mm]\n";
	std::cerr << "       <-t float float float>   CT volume translation in x, y, and z direction in mm \n";
	std::cerr << "       <-rx float>              CT volume rotation about x axis in degrees \n";
	std::cerr << "       <-ry float>              CT volume rotation about y axis in degrees \n";
	std::cerr << "       <-rz float>              CT volume rotation about z axis in degrees \n";
	std::cerr << "       <-2dcx float float>      Central axis position of DRR in continuous indices \n";
	std::cerr << "       <-iso float float float> Continous voxel indices of CT isocenter (center of rotation and projection center)\n";
	std::cerr << "       <-rp float>              Projection angle in degrees";
	std::cerr << "       <-threshold float>       CT intensity threshold, below which are ignored [default: 0]\n";
	std::cerr << "       <-o file>                Output image filename\n\n";
	std::cerr << "                                by  Jian Wu (eewujian@hotmail.com)\n\n";
	exit(1);
}


itk::Image<unsigned char,3>::Pointer drr(itk::Image<float,3>::Pointer inputImage,std::string outputFileName, float scd = 1000, float rx = 0, float ry = 0, float rz = 0,
	float tx = 0, float ty = 0, float tz = 0,float threshold=0,bool verbose = false, bool customized_iso = false,bool customized_2DCX=false,bool rprojection=0)
{
	const char *output_name = outputFileName.c_str();

	bool ok;
	//bool customized_iso = false;  // Flag for customized 3D image isocenter positions
	//bool customized_2DCX = false; // Flag for customized 2D image central axis positions

	//float rprojection = 0.; // Projection angle in degrees

	// CT volume rotation around isocenter along x,y,z axis in degrees
	/*float rx = 0.;
	float ry = 0.;
	float rz = 0.;*/

	// Translation parameter of the isocenter in mm
	//float tx = 0.;
	//float ty = 0.;
	//float tz = 0.;

	// The pixel indices of the isocenter
	float cx = 0.;
	float cy = 0.;
	float cz = 0.;

	//float scd = 1000; // Source to isocenter distance in mm

	// Default pixel spacing in the iso-center plane in mm
	float im_sx = 0.51;
	float im_sy = 0.51;

	// Size of the output image in number of pixels
	int dx = 512;
	int dy = 512;

	// The central axis positions of the 2D images in continuous indices
	float o2Dx;
	float o2Dy;


	// Create a timer to record calculation time.
	//itk::TimeProbesCollectorBase timer;

	if (verbose) 
	{
		std::cout << "Output image: " << output_name << std::endl;
	}

	// Although we generate a 2D projection of the 3D volume for the
	// purposes of the interpolator both images must be three dimensional.

	const     unsigned int   Dimension = 3;
	typedef   float  InputPixelType;
	typedef   unsigned char OutputPixelType;

	typedef itk::Image< InputPixelType,  Dimension >   InputImageType;
	typedef itk::Image< OutputPixelType, Dimension >   OutputImageType;

	//InputImageType::Pointer image;

	//// For the purposes of this example we assume the input volume has
	//// been loaded into an {itk::Image image}.

	//if (input_name) 
	//{

	//	timer.Start("Loading Input Image");
	//	using SeriesFileNamesType = itk::GDCMSeriesFileNames;
	//	using ImageType = itk::Image<signed short, 3>;
	//	using ReaderType = itk::ImageSeriesReader<ImageType>;
	//	using ImageIOType = itk::GDCMImageIO;
	//	ImageIOType::Pointer m_gdcmImageIO= ImageIOType::New();
	//	SeriesFileNamesType::Pointer m_gdcmSeriesFileNames = SeriesFileNamesType::New();
	//	m_gdcmSeriesFileNames->SetUseSeriesDetails(true);
	//	// m_gdcmSeriesFileNames->AddSeriesRestriction("0008|0021");
	//	m_gdcmSeriesFileNames->SetDirectory(input_name);
	//	using SeriesIdContainer = std::vector<std::string>;
	//	const SeriesIdContainer& seriesUID = m_gdcmSeriesFileNames->GetSeriesUIDs();
	//	const std::string& seriesUIDStr = seriesUID.at(0);
	//	using FileNamesContainer = std::vector<std::string>;
	//	FileNamesContainer fileNames = m_gdcmSeriesFileNames->GetFileNames(seriesUIDStr);
	//	ReaderType::Pointer reader = ReaderType::New();
	//	reader->SetImageIO(m_gdcmImageIO);
	//	reader->SetFileNames(fileNames);
	//	reader->ForceOrthogonalDirectionOff();

	//	try { 
	//		reader->Update();
	//	} 

	//	catch( itk::ExceptionObject & err ) 
	//	{ 
	//		std::cerr << "ERROR: ExceptionObject caught !" << std::endl; 
	//		std::cerr << err << std::endl; 
	//		return EXIT_FAILURE;
	//	} 

	//	image = reader->GetOutput();
	//	timer.Stop("Loading Input Image");
	//}
	//else 
	//{
	//	std::cerr << "Input image file missing !" << std::endl;
	//	return EXIT_FAILURE;
	//}

	// To simply Siddon-Jacob's fast ray-tracing algorithm, we force the origin of the CT image
	// to be (0,0,0). Because we align the CT isocenter with the central axis, the projection
	// geometry is fully defined. The origin of the CT image becomes irrelavent.
	InputImageType::PointType ctOrigin;
	ctOrigin[0] = 0.0;
	ctOrigin[1] = 0.0;
	ctOrigin[2] = 0.0;
	inputImage->SetOrigin(ctOrigin);

	// Print out the details of the input volume

	if (verbose) 
	{
		unsigned int i;
		const itk::Vector<double, 3> spacing = inputImage->GetSpacing();
		std::cout << std::endl << "Input ";

		InputImageType::RegionType region = inputImage->GetBufferedRegion();
		region.Print(std::cout);

		std::cout << "  Resolution: [";
		for (i=0; i<Dimension; i++) 
		{
			std::cout << spacing[i];
			if (i < Dimension-1) std::cout << ", ";
		}
		std::cout << "]" << std::endl;

		const itk::Point<double, 3> origin = inputImage->GetOrigin();
		std::cout << "  Origin: [";
		for (i=0; i<Dimension; i++) 
		{
			std::cout << origin[i];
			if (i < Dimension-1) std::cout << ", ";
		}
		std::cout << "]" << std::endl<< std::endl;
	}

	// Creation of a {ResampleImageFilter} enables coordinates for
	// each of the pixels in the DRR image to be generated. These
	// coordinates are used by the {RayCastInterpolateImageFunction}
	// to determine the equation of each corresponding ray which is cast
	// through the input volume.

	typedef itk::ResampleImageFilter<InputImageType, InputImageType > FilterType;

	FilterType::Pointer filter = FilterType::New();

	filter->SetInput(inputImage);
	filter->SetDefaultPixelValue( 0 );

	// An Euler transformation is defined to position the input volume.

	typedef itk::Euler3DTransform< double >  TransformType;

	TransformType::Pointer transform = TransformType::New();

	transform->SetComputeZYX(true);

	TransformType::OutputVectorType translation;

	translation[0] = tx;
	translation[1] = ty;
	translation[2] = tz;

	// constant for converting degrees into radians
	const double dtr = ( atan(1.0) * 4.0 ) / 180.0;

	transform->SetTranslation( translation );
	transform->SetRotation( dtr*rx, dtr*ry, dtr*rz );

	InputImageType::PointType   imOrigin = inputImage->GetOrigin();
	InputImageType::SpacingType imRes    = inputImage->GetSpacing();

	typedef InputImageType::RegionType     InputImageRegionType;
	typedef InputImageRegionType::SizeType InputImageSizeType;

	InputImageRegionType imRegion = inputImage->GetBufferedRegion();
	InputImageSizeType   imSize   = imRegion.GetSize();

	TransformType::InputPointType isocenter;
	if (customized_iso)
	{
		// Isocenter location given by the user.
		isocenter[0] = imOrigin[0] + imRes[0] * cx; 
		isocenter[1] = imOrigin[1] + imRes[1] * cy; 
		isocenter[2] = imOrigin[2] + imRes[2] * cz;
	}
	else
	{
		// Set the center of the image as the isocenter.
		isocenter[0] = imOrigin[0] + imRes[0] * static_cast<double>( imSize[0] ) / 2.0; 
		isocenter[1] = imOrigin[1] + imRes[1] * static_cast<double>( imSize[1] ) / 2.0; 
		isocenter[2] = imOrigin[2] + imRes[2] * static_cast<double>( imSize[2] ) / 2.0;
	}
	transform->SetCenter(isocenter);

	if (verbose) 
	{
		std::cout << "Image size: "
			<< imSize[0] << ", " << imSize[1] << ", " << imSize[2] << std::endl
			<< "   resolution: "
			<< imRes[0] << ", " << imRes[1] << ", " << imRes[2] << std::endl
			<< "   origin: "
			<< imOrigin[0] << ", " << imOrigin[1] << ", " << imOrigin[2] << std::endl
			<< "   isocenter: "
			<< isocenter[0] << ", " << isocenter[1] << ", " << isocenter[2] << std::endl
			<< "Transform: " << transform << std::endl;
	}

	typedef itk::SiddonJacobsRayCastInterpolateImageFunction<InputImageType,double> InterpolatorType;
	InterpolatorType::Pointer interpolator = InterpolatorType::New();

	interpolator->SetProjectionAngle( dtr * rprojection ); // Set angle between projection central axis and -z axis
	interpolator->SetFocalPointToIsocenterDistance(scd); // Set source to isocenter distance
	interpolator->SetThreshold(threshold); // Set intensity threshold, below which are ignored.
	interpolator->SetTransform(transform);

	interpolator->Initialize();

	filter->SetInterpolator( interpolator );



	// The size and resolution of the output DRR image is specified via the filter.

	// setup the scene
	InputImageType::SizeType   size;
	double spacing[ Dimension ];

	size[0] = dx;  // number of pixels along X of the 2D DRR image
	size[1] = dy;   // number of pixels along X of the 2D DRR image
	size[2] = 1;   // only one slice
	spacing[0] = im_sx;  // pixel spacing along X of the 2D DRR image [mm]
	spacing[1] = im_sy;  // pixel spacing along Y of the 2D DRR image [mm]
	spacing[2] = 1.0; // slice thickness of the 2D DRR image [mm]

	filter->SetSize( size );

	filter->SetOutputSpacing( spacing );

	if (verbose) 
	{
		std::cout << "Output image size: " 
			<< size[0] << ", " 
			<< size[1] << ", " 
			<< size[2] << std::endl;

		std::cout << "Output image spacing: " 
			<< spacing[0] << ", " 
			<< spacing[1] << ", " 
			<< spacing[2] << std::endl;
	}

	double origin[ Dimension ];

	if (!customized_2DCX)
	{ // Central axis positions are not given by the user. Use the image centers 
		// as the central axis position.
		o2Dx = ((double) dx - 1.)/2.;
		o2Dy = ((double) dy - 1.)/2.;
	}
	
	// Compute the origin (in mm) of the 2D Image
	origin[0] = - im_sx * o2Dx; 
	origin[1] = - im_sy * o2Dy;
	origin[2] = - 200;

	filter->SetOutputOrigin( origin );

	//timer.Start("DRR generation");
	filter->Update();
	//timer.Stop("DRR generation");

	if (verbose)
	{
		std::cout << "Output image origin: "
			<< origin[0] << ", " 
			<< origin[1] << ", " 
			<< origin[2] << std::endl;
	}

	typedef itk::IntensityWindowingImageFilter <InputImageType, OutputImageType> IntensityWindowingImageFilterType;
	IntensityWindowingImageFilterType::Pointer intensityFilter = IntensityWindowingImageFilterType::New();
	intensityFilter->SetInput(filter->GetOutput());
	intensityFilter->SetWindowLevel(1000, 400);//window,level//180,90
	intensityFilter->SetOutputMinimum(0);
	intensityFilter->SetOutputMaximum(255);
	intensityFilter->Update();
	/*typedef itk::RescaleIntensityImageFilter< 
		InputImageType, OutputImageType > RescaleFilterType;
	RescaleFilterType::Pointer rescaler = RescaleFilterType::New();
	rescaler->SetOutputMinimum(   0 );
	rescaler->SetOutputMaximum( 255 );
	rescaler->SetInput( filter->GetOutput() );*/

	//timer.Start("DRR post-processing");
	//rescaler->Update();

	/* Out of some reason, the computed projection is upsided-down.
	 Here we use a FilpImageFilter to flip the images in y direction.*/
	typedef itk::FlipImageFilter< OutputImageType > FlipFilterType;
	FlipFilterType::Pointer flipFilter = FlipFilterType::New();

	typedef FlipFilterType::FlipAxesArrayType FlipAxesArrayType;
	FlipAxesArrayType flipArray;
	flipArray[0] = 0;
	flipArray[1] = 1;

	flipFilter->SetFlipAxes( flipArray );
	flipFilter->SetInput(intensityFilter->GetOutput() );
	flipFilter->Update();

//timer.Stop("DRR post-processing");
	// create writer

	//if (output_name) 
	//{

		// The output of the filter can then be passed to a writer to
		// save the DRR image to a file.

		//typedef itk::ImageFileWriter< OutputImageType >  WriterType;
		//WriterType::Pointer writer = WriterType::New();
		//writer->SetImageIO(itk::GDCMImageIO::New());
		
		// Now we are ready to write the projection image.
		//writer->SetFileName( output_name );
		//writer->SetInput( flipFilter->GetOutput() );

		//try 
		//{ 
		//	std::cout << "Writing image: " << output_name << std::endl;
		//	writer->Update();
		//} 
		//catch( itk::ExceptionObject & err ) 
		//{ 
		//	std::cerr << "ERROR: ExceptionObject caught !" << std::endl; 
		//	std::cerr << err << std::endl; 
		//} 
	//}
	//else 
	//{
	//	filter->Update();
	//}

	//timer.Report();

	return flipFilter->GetOutput();
}

