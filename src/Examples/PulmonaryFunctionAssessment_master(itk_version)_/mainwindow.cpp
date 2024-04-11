#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "uiHelper.h"
#include "ct_image.h"
#include "vtkHelper.h"
#include "image_register.h"
//Qt
#include <QProgressDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QListWidgetItem>
#include <QDebug>
#include <QTextCodec>
#include <about.h>
#include <QMouseEvent>
#include "RegisterSegmentWidget.h"
#include "uStatus.h"
#include <vtkMarchingCubes.h>
const QSize IMAGE_SIZE(138, 138);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->horizontalHeader()->setVisible(true);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->horizontalHeader()->setHighlightSections(false);
    ui->tableWidget->horizontalHeader()->setFixedHeight(40);
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(40);

    ui->widget_coronal->setViewDirection(ViewDirection_Coronal);
    ui->widget_sagittal->setViewDirection(ViewDirection_Sagittal);
    mContainer = new QHBoxLayout(ui->scrollAreaWidgetContents);
    mContainer->setSpacing(5);

    connect(ui->tableWidget, &QTableWidget::itemDoubleClicked, this, &MainWindow::openCase);
    connect(ui->action_about, &QAction::triggered, this, &MainWindow::About);

    ui->widget_axial->installEventFilter(this);
    ui->widget_coronal->installEventFilter(this);
    ui->widget_sagittal->installEventFilter(this);
    ui->widget_3d->installEventFilter(this);
    ui->widget_3d_postProcess->installEventFilter(this);
    ui->widget_chartpie->installEventFilter(this);
    loadRecentImages();

    std::map<std::string, int> data;
    data["hello"] = 2;
    data["world"] = 10;
    ui->widget_chartpie->setData(data);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::About()
{
    AboutDialog about;
    about.exec();
}

void MainWindow::blockAllSignals(bool block)
{
    auto children=findChildren<QPushButton*>();
    for(auto c:children)
    {
        c->blockSignals(block);
    }
}

void MainWindow::showImage(CT_Image *img, bool generateJpg)
{
    //preview
    ui->widget_preview->setCTImage(img);
    //mpr
    ui->widget_axial->setCTImage(img);
    ui->widget_sagittal->setCTImage(img);
    ui->widget_coronal->setCTImage(img);
    //渲染三维
    ui->widget_3d->setCTImage(img);

    if(generateJpg)
    {
        std::vector<std::string> filenames = img->getFileNames();
        QString filepath = img->getFilePath();

        int i = 0;
        for (; i < filenames.size(); i++)
        {
            QByteArray b;
            b.append(filepath + "/" + QFileInfo(QString::fromStdString(filenames.at(i))).baseName() + ".dcm");
            char* str = b.data();
            //将DICOM格式转变为PNG格式

            QString JPGimgPath = DICOMToJPEG(str);
            QPixmap pixmap(JPGimgPath);
            QLabel* label = new QLabel;
            label->setFixedSize(IMAGE_SIZE);
            label->setPixmap(pixmap.scaled(IMAGE_SIZE));
            mContainer->addWidget(label);
        }
    }
    else
    {
        QStringList ls=QDir("./out").entryList(QStringList()<<"*.jpg");
        for(auto l:ls)
        {
	        if(!l.endsWith("jpg"))
                continue;
            QString JPGimgPath = "./out/" + l;
            QPixmap pixmap(JPGimgPath);
            QLabel* label = new QLabel;
            label->setFixedSize(IMAGE_SIZE);
            label->setPixmap(pixmap.scaled(IMAGE_SIZE));
            mContainer->addWidget(label);
        }
    }
}

void MainWindow::loadRecentImages()
{
    ui->tableWidget->clear();
    int colCount = 5;
    ui->tableWidget->setColumnCount(colCount);//name sex age date
    QTextCodec* codec = QTextCodec::codecForName("utf-8");

    ui->tableWidget->setHorizontalHeaderLabels(QStringList()
        << codec->toUnicode("姓名")
        << codec->toUnicode("性别")
        << codec->toUnicode("年龄 ")
        << codec->toUnicode("时间")
        << codec->toUnicode("影像路径")
    );
    if (!QDir("./cache/").exists()) {
        return;
    }
    QStringList recentImageNameList = QDir("./cache/imageInfo").entryList(QDir::Files);
    QStringList ls;
    for (QString fileName : recentImageNameList) {
        QString completePath = "./cache/imageInfo/" + fileName;
        ImageRegister im(completePath); // will be deleted in RecentImageListWidgetItem
        auto map = im.getMap();
        QString filePath = map["filePath"];
        if(QFileInfo(filePath).isDir() && !QDir(filePath).exists())
        {
            QFile::remove(completePath);
            continue;
        }else if (QFileInfo(filePath).isFile() && !QFile(filePath).exists())
        {
            QFile::remove(completePath);
            continue;
        }
        ls.append(map["Patient Name"]);
        ls.append(map["Patient's Sex"]);
        ls.append(map["Patient's Age"]);
        ls.append(map["Instance Creation Date"]);
        ls.append(filePath);
    }
    seriesToTable(ls);
}

void MainWindow::seriesToTable(QStringList items)
{
    //ui->tableWidget->clear();
    //ui->tableWidget->setRowCount(0);
    int colCount = 5;
    int rowCount = ui->tableWidget->rowCount();
    for (size_t i = 0; i < items.size()/ colCount; i++)
    {
        ui->tableWidget->insertRow(rowCount+i);
        for(int j=0;j< colCount;++j)
        {
            QTableWidgetItem* item = new QTableWidgetItem(items[i * colCount + j]);
            item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            ui->tableWidget->setItem(i, j, item);
        }
    }
}

void MainWindow::openCase()
{
    QList<QTableWidgetItem*> items = ui->tableWidget->selectedItems();
    if (items.size() == 0)
    {
        return;
    }
    QString path = items.at(4)->text();
    QProgressDialog* progressDialog = createProgressDialog(tr("CT Loading"), tr("Loading CT, Please Wait..."), 101);
    progressDialog->setWindowFlag(Qt::WindowStaysOnTopHint);
    if (QFileInfo(path).isDir())
        uStatus::g_ctimage.loadDicomFromDirectory(path, progressDialog);
    else
        uStatus::g_ctimage.loadNiftiFromFilePath(path, progressDialog);
    progressDialog->deleteLater();

    if (!uStatus::g_ctimage.checkLoadSuccess()) {
        progressDialog->close();
        QMessageBox msgBox;
        msgBox.setText("No Dicom Files in the directory or Loading Failed!");
        msgBox.exec();

        return;
    }
    m_currentImageData = uStatus::g_ctimage.getCTImageDataVtk();
    mImageItk = uStatus::g_ctimage.getCTImageDataItk();
    blockAllSignals(false);
    showImage(&uStatus::g_ctimage, !uStatus::g_ctimage.isNifti);
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
    if(event->type()==QEvent::MouseButtonDblClick)
    {
        QMouseEvent* ev = static_cast<QMouseEvent*>(event);
        if (ev->button() != Qt::LeftButton)
            return false;
        if(watched==ui->widget_3d)
        {
            if(isShowMax)
            {
                ui->gridLayout->removeWidget(ui->widget_3d);
                ui->widget_coronal->show();
                ui->widget_axial->show();
                ui->widget_sagittal->show();
                ui->widget_3d_postProcess->show();
                ui->widget_chartpie->show();
                ui->gridLayout->addWidget(ui->widget_3d, 1, 0);
            }else
            {
                ui->widget_coronal->hide();
                ui->widget_axial->hide();
                ui->widget_sagittal->hide();
                ui->widget_3d_postProcess->hide();
                ui->widget_chartpie->hide();
                ui->gridLayout->addWidget(ui->widget_3d, 0, 0, 3, 3);
            }
            isShowMax = !isShowMax;
        }
        else if (watched == ui->widget_sagittal)
        {
            if (isShowMax)
            {
                ui->gridLayout->removeWidget(ui->widget_sagittal);
                ui->widget_coronal->show();
                ui->widget_axial->show();
                ui->widget_3d->show();
                ui->widget_3d_postProcess->show();
                ui->widget_chartpie->show();
                ui->gridLayout->addWidget(ui->widget_sagittal, 0, 2);
            }
            else
            {
                ui->widget_coronal->hide();
                ui->widget_axial->hide();
                ui->widget_3d->hide();
                ui->widget_3d_postProcess->hide();
                ui->widget_chartpie->hide();
                ui->gridLayout->addWidget(ui->widget_sagittal, 0, 0, 3, 3);
            }
            isShowMax = !isShowMax;
        }
        else if (watched == ui->widget_coronal)
        {
            if (isShowMax)
            {
                ui->gridLayout->removeWidget(ui->widget_coronal);
                ui->widget_3d->show();
                ui->widget_axial->show();
                ui->widget_sagittal->show();
                ui->widget_3d_postProcess->show();
                ui->widget_chartpie->show();
                ui->gridLayout->addWidget(ui->widget_coronal, 0, 1);
            }
            else
            {
                ui->widget_3d->hide();
                ui->widget_axial->hide();
                ui->widget_sagittal->hide();
                ui->widget_3d_postProcess->hide();
                ui->widget_chartpie->hide();
                ui->gridLayout->addWidget(ui->widget_coronal, 0, 0, 3, 3);
            }
            isShowMax = !isShowMax;
        }
        else if (watched == ui->widget_axial)
        {
            if (isShowMax)
            {
                ui->gridLayout->removeWidget(ui->widget_axial);
                ui->widget_coronal->show();
                ui->widget_3d->show();
                ui->widget_sagittal->show();
                ui->widget_3d_postProcess->show();
                ui->widget_chartpie->show();
                ui->gridLayout->addWidget(ui->widget_axial, 0, 0);
            }
            else
            {
                ui->widget_coronal->hide();
                ui->widget_3d->hide();
                ui->widget_sagittal->hide();
                ui->widget_3d_postProcess->hide();
                ui->widget_chartpie->hide();
                ui->gridLayout->addWidget(ui->widget_axial, 0, 0, 3, 3);
            }
            isShowMax = !isShowMax;
        }
        else if (watched == ui->widget_3d_postProcess)
        {
            if (isShowMax)
            {
                ui->gridLayout->removeWidget(ui->widget_3d_postProcess);
                ui->widget_coronal->show();
                ui->widget_3d->show();
                ui->widget_sagittal->show();
                ui->widget_axial->show();
                ui->widget_chartpie->show();
                ui->gridLayout->addWidget(ui->widget_3d_postProcess, 1, 1);
            }
            else
            {
                ui->widget_coronal->hide();
                ui->widget_3d->hide();
                ui->widget_sagittal->hide();
                ui->widget_axial->hide();
                ui->widget_chartpie->hide();
                ui->gridLayout->addWidget(ui->widget_3d_postProcess, 0, 0, 3, 3);
            }
            isShowMax = !isShowMax;
        }
        else if (watched == ui->widget_chartpie)
        {
        if (isShowMax)
        {
            ui->gridLayout->removeWidget(ui->widget_chartpie);
            ui->widget_coronal->show();
            ui->widget_3d->show();
            ui->widget_sagittal->show();
            ui->widget_axial->show();
            ui->widget_3d_postProcess->show();
            ui->gridLayout->addWidget(ui->widget_chartpie, 1, 2);
        }
        else
        {
            ui->widget_coronal->hide();
            ui->widget_3d->hide();
            ui->widget_sagittal->hide();
            ui->widget_axial->hide();
            ui->widget_3d_postProcess->hide();
            ui->gridLayout->addWidget(ui->widget_chartpie, 0, 0, 3, 3);
        }
        isShowMax = !isShowMax;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::on_pushButton_scan_clicked()
{
    blockAllSignals(true);
    QString filename = QFileDialog::getExistingDirectory(this, "Image directory", "../");
    QProgressDialog* progressDialog = createProgressDialog(tr("Image Loading"), tr("Loading Image, Please Wait..."), 101);
    progressDialog->setWindowFlag(Qt::WindowStaysOnTopHint);
    uStatus::g_ctimage.loadDicomFromDirectory(filename, progressDialog);
    progressDialog->deleteLater();

    // check the path is valid
    if (!uStatus::g_ctimage.checkLoadSuccess()) {
        progressDialog->close();
        QMessageBox msgBox;
        msgBox.setText("No Dicom Files in the directory or Loading Failed!");
        msgBox.exec();
        
        return;
    }
    m_currentImageData = uStatus::g_ctimage.getCTImageDataVtk();
    mImageItk = uStatus::g_ctimage.getCTImageDataItk();
    blockAllSignals(false);
    showImage(&uStatus::g_ctimage);
    ImageRegister imageInfo(uStatus::g_ctimage.getMetaInfo(), uStatus::g_ctimage.getFilePath());
    imageInfo.save();
    QString path = imageInfo.getSavePath();
    auto map = imageInfo.getMap();
    QStringList ls;
    ls.append(map["Patient Name"]);
    ls.append(map["Patient's Sex"]);
    ls.append(map["Patient's Age"]);
    ls.append(map["Instance Creation Date"]);
    ls.append(map["filePath"]);
    seriesToTable(ls);
}

void MainWindow::on_pushButton_scanNifti_clicked()
{
    blockAllSignals(true);
    QString filename = QFileDialog::getOpenFileName(this, "Image Path", "../","*.nii");
    QProgressDialog* progressDialog = createProgressDialog(tr("Image Loading"), tr("Loading Image, Please Wait..."), 101);
    progressDialog->setWindowFlag(Qt::WindowStaysOnTopHint);
    uStatus::g_ctimage.loadNiftiFromFilePath(filename, progressDialog);
    progressDialog->deleteLater();

    // check the path is valid
    if (!uStatus::g_ctimage.checkLoadSuccess()) {
        progressDialog->close();
        QMessageBox msgBox;
        msgBox.setText("No Dicom Files in the directory or Loading Failed!");
        msgBox.exec();

        return;
    }

    m_currentImageData = uStatus::g_ctimage.getCTImageDataVtk();
    mImageItk = uStatus::g_ctimage.getCTImageDataItk();
    blockAllSignals(false);
    showImage(&uStatus::g_ctimage,false);

    ImageRegister imageInfo(uStatus::g_ctimage.getMetaInfo(), uStatus::g_ctimage.getFilePath() + "/" + uStatus::g_ctimage.getFileName());
    imageInfo.save();
    QString path = imageInfo.getSavePath();
    auto map = imageInfo.getMap();
    QStringList ls;
    ls.append(map["Patient Name"]);
    ls.append(map["Patient's Sex"]);
    ls.append(map["Patient's Age"]);
    ls.append(map["Instance Creation Date"]);
    ls.append(map["filePath"]);
    seriesToTable(ls);
}

void MainWindow::on_pushButton_exit_clicked()
{
    qApp->quit();
}

void MainWindow::on_pushButton_imageProcess_clicked()
{
    if (!uStatus::g_ctimage.checkLoadSuccess()) {
        QMessageBox::warning(this, "Warnning", "Please Open An Image!");
        return;
    }
    RegisterSegmentWidget *d=new RegisterSegmentWidget;
    d->resize(1200, 800);

    d->setImageItk(mImageItk);
    connect(d,&RegisterSegmentWidget::signal_volumeRenderingForegroundImage,this,&MainWindow::slot_volumeRenderingForegroundImage);
    d->show();
}

void MainWindow::slot_volumeRenderingForegroundImage()
{
    RegisterSegmentWidget* dialog = qobject_cast<RegisterSegmentWidget*>(sender());
    if (!dialog)
       return;
    vtkSmartPointer<vtkImageData> segmentImage=dialog->getSegmentResult();
    vtkSmartPointer<vtkMarchingCubes> surface =
        vtkSmartPointer<vtkMarchingCubes>::New();
    surface->SetInputData(segmentImage);
    surface->ComputeNormalsOn();
    surface->SetValue(0, 1); 
    surface->Update();
    ui->widget_3d_postProcess->setMesh(surface->GetOutput());
}
