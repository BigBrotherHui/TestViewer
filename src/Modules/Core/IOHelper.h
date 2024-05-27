#pragma once
#include <vtkSTLWriter.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <mitkDataNode.h>
#include <QmitkIOUtil.h>
#include "CoreExports.h"
class Core_EXPORT IOHelper {
public:
    static void writeToStl(vtkPolyData *p, const std::string &name);
    static void writeToNii(vtkImageData *image,const std::string &name);
    static void saveNode(mitk::DataNode *node,const std::string &name);
    static void saveData(mitk::BaseData *data, const std::string &name);
    mitk::DataStorage::SetOfObjects::Pointer load(const QString &path);
};