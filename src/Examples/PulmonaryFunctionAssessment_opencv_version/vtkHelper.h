#pragma once
#ifndef VTKHELPER_H
#define VTKHELPER_H

#include <vtkRenderer.h>
#include <QString>

QString DICOMToJPEG(char* imgpath);
void setHeader(vtkRenderer* ren, int axis);

#endif