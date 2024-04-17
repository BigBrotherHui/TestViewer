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
#include "Algorithm.h"
#include <qchart.h>
QT_CHARTS_USE_NAMESPACE

#include <QLineSeries>
#include <QChart>
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
    void showImage(CT_Image* img);
    void loadRecentImages();
    void seriesToTable(QStringList items);
    void openCase();
    bool eventFilter(QObject* watched, QEvent* event) override;
protected slots:
    void on_pushButton_scanNifti_clicked();
    void on_pushButton_exit_clicked();
    void on_pushButton_reset_clicked();
    void on_pushButton_register_clicked();
    void on_pushButton_segment_clicked();
    void slot_draw();
    void slot_register_finished();
    void slot_thresholdvalue_changed(int v);
private:
    Ui::MainWindow *ui;


private slots:
private:
    itk::Image<CT_Image::PixelType, 3>::Pointer mImageItk;
    QHBoxLayout* mContainer;
    QMap<QString, QString> mMap;
    bool isShowMax{ false };
    vtkSmartPointer<vtkImageData> m_foregroundImageData{ nullptr };
    QAction* roi_rectangle;
    QAction* roi_ellipse;
    QAction* roi_apply;
    QAction* endroi;
    ImageProcess::Algorithm algorthom_;
    QLineSeries* series;
    QChart* chart;
};

#endif // MAINWINDOW_H
