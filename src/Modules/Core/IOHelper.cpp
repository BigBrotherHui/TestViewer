#include "IOHelper.h"
#include <vtkNIFTIImageWriter.h>
#include <mitkIOUtil.h>
#include <mitkStandaloneDataStorage.h>
void IOHelper::writeToStl(vtkPolyData* p, const std::string& name)
{
    if (!p) return;
    vtkSmartPointer<vtkSTLWriter> w = vtkSmartPointer<vtkSTLWriter>::New();
    w->SetFileName(name.c_str());
    w->SetInputData(p);
    w->Write();
}

void IOHelper::writeToNii(vtkImageData* image, const std::string& name)
{
    if (!image) return;
    vtkSmartPointer<vtkNIFTIImageWriter> writer = vtkSmartPointer<vtkNIFTIImageWriter>::New();
    writer->SetInputData(image);
    writer->SetFileName(name.c_str());
    writer->Write();
}

void IOHelper::saveData(mitk::BaseData* data, const std::string& name)
{
    if (!data) return;
    mitk::IOUtil::Save(data, name);
}

mitk::DataStorage::SetOfObjects::Pointer IOHelper::load(const QString& path)
{
    mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
    return QmitkIOUtil::Load(path, *ds);
}

void IOHelper::saveNode(mitk::DataNode* node, const std::string& name)
{
    if (!node) return;
    saveData(node->GetData(),name);
}
