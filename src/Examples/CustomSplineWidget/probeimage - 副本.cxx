#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkImageMapToColors.h>
#include <vtkImageMapper3D.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Factory.h>
#include <vtkInteractorStyleImage.h>
#include <vtkLookupTable.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkDICOMImageReader.h>
#include <vtkAutoInit.h>
#include <vtkImagePlaneWidget.h>
#include <vtkWindowLevelLookupTable.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <QDebug>
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);
int main(int argc, char* argv[])
{
    vtkNew<vtkNamedColors> colors;

    // Verify input arguments

    // Read the image
    vtkSmartPointer<vtkDICOMImageReader> reader=vtkSmartPointer<vtkDICOMImageReader>::New();
    reader->SetFileName("D:/Images/THAIMAGE/Patient_002_0155.dcm");
    reader->Update();

    vtkImageData* image = reader->GetOutput();

    // Create a mask - half of the image should be transparent and the other half
    // opaque
    //vtkNew<vtkImageData> maskImage;
    //int extent[6];
    //image->GetExtent(extent);
    //maskImage->SetExtent(extent);
    //maskImage->AllocateScalars(VTK_DOUBLE, 1);

    //for (int y = extent[2]; y < extent[3]; y++) {
    //    for (int x = extent[0]; x < extent[1]; x++) {
    //        double* pixel = static_cast<double*>(maskImage->GetScalarPointer(x, y, 0));
    //        if (y > (extent[3] - extent[2]) / 2.0) {
    //            pixel[0] = 0.0;
    //        }
    //        else {
    //            pixel[0] = 1.0;
    //        }
    //    }
    //}
    double min_val = image->GetScalarRange()[0];
    double max_val = image->GetScalarRange()[1];
    double window_width = max_val - min_val;
    double window_level = (max_val + min_val) / 2.0;
    vtkNew<vtkImageMapToWindowLevelColors> lookupTable;
    int level = 78;
    int window = 2203;
    lookupTable->SetLevel(level);
    lookupTable->SetWindow(/*window_width*/window);
    lookupTable->SetInputData(image);
    lookupTable->Update();
    //lookupTable->GetOutput()->Print(std::cout);
    auto scalars = lookupTable->GetOutput()->GetPointData()->GetScalars();//GetArray("DICOMImage")
    //int to{0};
    auto img = lookupTable->GetOutput();
    int dims[3];
    img->GetDimensions(dims);

    for (int k = 0; k < dims[2]; ++k) {
        for (int j = 0; j < dims[1]; ++j) {
            for (int i = 0; i < dims[0]; ++i) {
                vtkIdType index = k * dims[0] * dims[1] + j * dims[0] + i;
                double* tuple = scalars->GetTuple4(index);
                ushort v = *((ushort*)img->GetScalarPointer(i,j,k));
                if (v < level - window/2 || v > level + window/2)
                    scalars->SetComponent(index, 3, 255);
                else
                    scalars->SetComponent(index, 3, 0);
            }
        }
    }
    //scalars->Modified();
    //lookupTable->GetOutput()->GetPointData()->Modified();
    //lookupTable->GetOutput()->Modified();
    //vtkNew<vtkImageMapToColors> mapTransparency;
    //mapTransparency->SetLookupTable(lookupTable);
    ////mapTransparency->PassAlphaToOutputOn();
    //mapTransparency->SetInputData(image);

    // Create actors
    //vtkNew<vtkImageActor> imageActor;
    //imageActor->GetMapper()->SetInputData(image);
    //qDebug() << "*(**" << image->GetScalarRange()[1];
    vtkNew<vtkImageActor> maskActor;
    maskActor->GetMapper()->SetInputData(lookupTable->GetOutput());
    //qDebug() << "*****"<< maskActor->GetInput()->GetScalarRange()[1];
    // Visualize
    vtkNew<vtkRenderer> renderer;
    //renderer->AddActor(imageActor);
    renderer->AddActor(maskActor);
    renderer->SetBackground(colors->GetColor3d("CornflowerBlue").GetData());
    renderer->ResetCamera();

    vtkNew<vtkRenderWindow> renderWindow;
    renderWindow->AddRenderer(renderer);
    renderWindow->SetWindowName("Transparency");

    vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
    vtkNew<vtkInteractorStyleImage> style;

    //renderWindowInteractor->SetInteractorStyle(style);

    renderWindowInteractor->SetRenderWindow(renderWindow);
    renderWindow->Render();
    renderWindowInteractor->Initialize();
    renderWindowInteractor->Start();

    return EXIT_SUCCESS;
}