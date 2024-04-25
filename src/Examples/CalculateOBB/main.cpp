

#include <vtkAxesActor.h>
#include <vtkColorTransferFunction.h>
#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPiecewiseFunction.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkStructuredPoints.h>
#include <vtkStructuredPointsReader.h>
#include <vtkVolumeProperty.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkDicomImageReader.h>
#include <vtkSmartPointer.h>
#include <vtkAutoInit.h>
#include <vtkAnnotatedCubeActor.h>
#include <vtkImageFlip.h>
#include <vtkDICOMReader.h>
#include <iostream>


#include <itkImageFileReader.h>
#include <itkImageSeriesReader.h>
#include <itkGDCMImageIO.h>
#include <itkGDCMSeriesFileNames.h>
#include <itkImageToVTKImageFilter.h>

#include <QDebug>
#include <vtkDICOMSorter.h>
#include <vtkStringArray.h>
#include <vtkMatrix3x3.h>
#include <vtkMatrix4x4.h>
#include <QDir>
#include <vtkOBBTree.h>
#include <vtkPolyDataMapper.h>

VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);
#include <QVTKOpenGLNativeWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkSTLReader.h>
#include <vtkProperty.h>
#include <QApplication>
#include <vtkOutlineFilter.h>
#include <vtkClipClosedSurface.h>
#include <vtkPlaneCollection.h>
#include <vtkPlane.h>
#include <vtkBox.h>
#include <vtkClipPolyData.h>
vtkSmartPointer<vtkPolyData> generateSideFemur(vtkPolyData *out,bool left=true)
{
    vtkSmartPointer<vtkPolyData> ret = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkBox> box = vtkSmartPointer<vtkBox>::New();
    double* b = out->GetBounds();
    double bounds[6];
    memcpy(bounds, b, sizeof(double) * 6);
    bounds[4] = (bounds[4] + bounds[5]) / 2;
    box->SetBounds(bounds);
    vtkSmartPointer<vtkClipPolyData> clipper = vtkSmartPointer<vtkClipPolyData>::New();
    clipper->SetInputData(out);
    clipper->SetClipFunction(box);
    clipper->Update();
    out = clipper->GetOutput();
    b = out->GetBounds();
    memcpy(bounds, b, sizeof(double) * 6);
    if (left)
        bounds[0] = (bounds[0] + bounds[1]) / 2;
    else
        bounds[1] = (bounds[0] + bounds[1]) / 2;
    box->SetBounds(bounds);
    vtkSmartPointer<vtkClipPolyData> clipper2 = vtkSmartPointer<vtkClipPolyData>::New();
    clipper2->SetInputData(out);
    clipper2->SetClipFunction(box);
    clipper2->Update();
    ret->DeepCopy(clipper2->GetOutput());
    return ret;
}
int main(int argc, char* argv[])
{
    QApplication ac(argc, argv);
    QVTKOpenGLNativeWidget widget;
    auto renderwindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    widget.setRenderWindow(renderwindow);
    auto renderer = vtkSmartPointer<vtkRenderer>::New();
    renderwindow->AddRenderer(renderer);
    vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
    reader->SetFileName("C:\\XJT_KASystem\\CaseMgt\\Planning\\2024-04-24_09-30-43_\\caseFemur.stl");
    reader->Update();
    vtkPolyData* out = reader->GetOutput();
    vtkSmartPointer<vtkPolyData> ret=generateSideFemur(out,false);
    int maxLevel = 5;
    auto obbTree = vtkSmartPointer<vtkOBBTree>::New();
    obbTree->SetDataSet(ret);
    obbTree->SetMaxLevel(maxLevel);
    obbTree->BuildLocator();

    auto polydata = vtkSmartPointer<vtkPolyData>::New();
    obbTree->GenerateRepresentation(0, polydata);
    auto obbtreeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    obbtreeMapper->SetInputData(polydata);

    auto obbtreeActor = vtkSmartPointer<vtkActor>::New();
    obbtreeActor->SetMapper(obbtreeMapper);
    obbtreeActor->GetProperty()->SetInterpolationToFlat();
    obbtreeActor->GetProperty()->SetOpacity(.5);
    obbtreeActor->GetProperty()->EdgeVisibilityOn();
    obbtreeActor->GetProperty()->SetColor(0., 1., 0.);
    renderer->AddActor(obbtreeActor);

    vtkSmartPointer<vtkActor> femur=vtkSmartPointer<vtkActor>::New();
    vtkSmartPointer<vtkPolyDataMapper> femurmapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    femur->SetMapper(femurmapper);
    femurmapper->SetInputData(ret);
    renderer->AddActor(femur);

    vtkSmartPointer<vtkOutlineFilter> outlinefilter = vtkSmartPointer<vtkOutlineFilter>::New();
    outlinefilter->SetInputData(ret);
    outlinefilter->Update();
    vtkSmartPointer<vtkActor> outline = vtkSmartPointer<vtkActor>::New();
    vtkSmartPointer<vtkPolyDataMapper> outlinemapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    outline->SetMapper(outlinemapper);
    outlinemapper->SetInputData(outlinefilter->GetOutput());
    renderer->AddActor(outline);

    widget.show();
    return ac.exec();
}