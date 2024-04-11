#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <QHBoxLayout>
#include <QMap>
#include <itkImage.h>
#include "ct_image.h"
#include <QtCore/qobjectdefs.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void About();
protected:
    void blockAllSignals(bool block);
    void showImage(CT_Image* img,bool generateJpg=1);
    void loadRecentImages();
    void seriesToTable(QStringList items);
    void openCase();
    bool eventFilter(QObject* watched, QEvent* event) override;
protected slots:
    void on_pushButton_scan_clicked();
    void on_pushButton_scanNifti_clicked();
    void on_pushButton_exit_clicked();
    void on_pushButton_imageProcess_clicked();
    void slot_volumeRenderingForegroundImage();
private:
    Ui::MainWindow *ui;


private slots:
private:
    vtkSmartPointer<vtkImageData> m_currentImageData{ nullptr };
    itk::Image<CT_Image::PixelType, 3>::Pointer mImageItk;
    QHBoxLayout* mContainer;
    QMap<QString, QString> mMap;
    bool isShowMax{ false };
    vtkSmartPointer<vtkImageData> m_foregroundImageData{ nullptr };
};

#endif // MAINWINDOW_H
