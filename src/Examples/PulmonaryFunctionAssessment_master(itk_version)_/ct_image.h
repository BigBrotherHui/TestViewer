#pragma once

#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkImageReslice.h>
#include <QProgressDialog>
#include <itkGDCMImageIO.h>
#include <itkSmartPointer.h>
#include <vtkImageAccumulate.h>
#include <QVector>
#include <QMap>
#include <itkImage.h>
#include <itkNiftiImageIO.h>

class CT_Image : public QObject
{ 
    Q_OBJECT;
public:
    using PixelType = signed short;
    CT_Image();
    ~CT_Image();
    bool loadDicomFromDirectory(QString path, QProgressDialog* dialog);
    bool loadNiftiFromFilePath(QString path, QProgressDialog* dialog);
    vtkSmartPointer<vtkImageData> getCTImageDataVtk();
    QMap<QString, QString> getMetaInfo();
    bool checkLoadSuccess();
    QString getFilePath();
    QString getFileName();
    void saveImageData(QString diretoryPath, QProgressDialog* dialog);
    std::vector<std::string> getFileNames();
    itk::Image<PixelType, 3>::Pointer getCTImageDataItk();
    bool isNifti{ false };
public slots:

signals:
    

private:
    void createMetaInfo(itk::SmartPointer<itk::GDCMImageIO> dicomIO);
    void createMetaInfo(itk::SmartPointer<itk::NiftiImageIO> dicomIO);
    void writeSeries(QString path, itk::Image<PixelType, 3>::Pointer image,itk::MetaDataDictionary dict,QString suffix="jpg");
    vtkSmartPointer<vtkImageAccumulate> ctImageAccumulate;
    vtkSmartPointer<vtkImageData> ctImage;
    vtkSmartPointer<vtkImageReslice> ctReslices[3];
    QMap<QString, QString> metaInfo;
    bool load_succeed{ false };
    QString path;
    QString fileName;
    double sliceCenter[3];
    double modelCenter[3];
    double modelBounds[6];
    signed short min_pixel_val;
    signed short max_pixel_val;
    int contrastThreshold[2] = {-1000, 1000};
    std::vector<std::string> mFileNames;
    itk::Image<PixelType, 3>::Pointer mImageItk;
};