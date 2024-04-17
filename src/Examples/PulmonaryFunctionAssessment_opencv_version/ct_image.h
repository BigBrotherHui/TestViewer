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
#include <vtkTransform.h>

class CT_Image : public QObject
{ 
    Q_OBJECT;
public:
    static const int Dimension = 4;
    using PixelType = short;
    typedef itk::Image<PixelType, Dimension> InputImageType;

    CT_Image();
    ~CT_Image();
    bool loadNiftiFromFilePath(QString path, QProgressDialog* dialog);
    vtkSmartPointer<vtkImageData> getCTImageDataVtk(int index);
    vtkSmartPointer<vtkImageData> getCTImageDataVtk_Registered(int index);
    vtkSmartPointer<vtkImageData> getCTImageDataVtk_Segmented(int index);
    vtkSmartPointer<vtkImageData> getCTImageDataVtk_Segmented_Src(int index);
    int getNumberOfImages();
    QMap<QString, QString> getMetaInfo();
    bool checkLoadSuccess();
    QString getFilePath();
    void saveImageData(QString diretoryPath, QProgressDialog* dialog);
    itk::Image<PixelType, Dimension>::Pointer getCTImageDataItk();
    template<class TItkImageType> bool AddItkImage(TItkImageType* input);
    void setRegisteredImages(std::vector<vtkSmartPointer<vtkImageData>> images);
    std::map<int, float>& getAverageMap();
public slots:

signals:
    

private:
    void createMetaInfo(itk::SmartPointer<itk::GDCMImageIO> dicomIO);
    void createMetaInfo(itk::SmartPointer<itk::NiftiImageIO> dicomIO);
    void writeSeries(QString path,itk::MetaDataDictionary dict);
    vtkSmartPointer<vtkImageAccumulate> ctImageAccumulate;
    QMap<QString, QString> metaInfo;
    bool load_succeed{ false };
    QString path;
    int mImageDimension;
    std::vector<vtkSmartPointer<vtkImageData>> mVtkImages;
    std::vector<vtkSmartPointer<vtkImageData>> mVtkImages_registered;
    std::vector<vtkSmartPointer<vtkImageData>> mVtkImages_segmented_src;//与配准结果一致，若选取ROI，则发生变化
    std::vector<vtkSmartPointer<vtkImageData>> mVtkImages_segmented;
    std::vector< vtkSmartPointer<vtkTransform> > mTransform;
    InputImageType::Pointer mImage;
    std::map<int, float> mAverageMap;
};
