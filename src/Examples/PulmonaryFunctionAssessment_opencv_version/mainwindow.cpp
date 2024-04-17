
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
#include "uStatus.h"
#include <vtkMarchingCubes.h>
#include "Algorithm.h"
#include <QValueAxis>
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

    mContainer = new QHBoxLayout(ui->scrollAreaWidgetContents);
    mContainer->setSpacing(5);

    connect(ui->tableWidget, &QTableWidget::itemDoubleClicked, this, &MainWindow::openCase);
    connect(ui->action_about, &QAction::triggered, this, &MainWindow::About);

    ui->widget_src->installEventFilter(this);
    ui->widget_imageProcess->installEventFilter(this);
    ui->widget_segment->installEventFilter(this);
    ui->widget_3d->installEventFilter(this);
    ui->widget_3d_postProcess->installEventFilter(this);
    ui->widget_chart->installEventFilter(this);
    loadRecentImages();
     
    ui->toolButton_ROI->setPopupMode(QToolButton::InstantPopup);
    QMenu* menu = new QMenu(ui->toolButton_ROI);
    roi_rectangle = new QAction("绘制矩形ROI",this);
    roi_ellipse = new QAction("绘制圆形ROI", this);
    roi_apply = new QAction("应用ROI",this);
    endroi = new QAction("结束ROI选取", this);
    connect(roi_rectangle, &QAction::triggered, this, &MainWindow::slot_draw);
    connect(roi_ellipse, &QAction::triggered, this, &MainWindow::slot_draw);
    connect(roi_apply, &QAction::triggered, this, &MainWindow::slot_draw);
    connect(endroi, &QAction::triggered, this, &MainWindow::slot_draw);
    menu->addAction(roi_rectangle);
    menu->addAction(roi_ellipse);
    menu->addAction(roi_apply);
    menu->addAction(endroi);
    ui->toolButton_ROI->setMenu(menu);

    ui->label->setStyleSheet("border-image:url(:/logo.png);");

    connect(&algorthom_, &ImageProcess::Algorithm::signal_register_finished, this, &MainWindow::slot_register_finished);
    connect(ui->horizontalSlider_threshold, &QSlider::valueChanged, this, &MainWindow::slot_thresholdvalue_changed);

    ui->widget_chart->setRenderHint(QPainter::Antialiasing);
    series = new QLineSeries();
    chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(series);
    series->setUseOpenGL(true);
    QValueAxis* xAxis = new QValueAxis();
    QValueAxis* yAxis = new QValueAxis();
    xAxis->setRange(1, 300);
    yAxis->setRange(0, 255);
    xAxis->setTitleText(QStringLiteral("Time Series"));
    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    brush.setColor(Qt::red);
    xAxis->setTitleBrush(brush);
    xAxis->setTickCount(300);
    xAxis->setLabelFormat("%d");
    xAxis->setTickInterval(10);
    xAxis->setLabelsVisible(false);

    chart->addAxis(xAxis, Qt::AlignBottom);
    chart->addAxis(yAxis, Qt::AlignLeft);
    chart->setTitle(QStringLiteral("PulmonaryFunctionAssessment"));

    ui->widget_chart->setChart(chart);
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

void MainWindow::showImage(CT_Image *img)
{
    ui->widget_src->setCTImage(img);
    ui->widget_src->Reset();
    ui->widget_imageProcess->setToImageProcessWidget();
    ui->widget_segment->setToSegmentWidget();
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
    uStatus::g_ctimage.loadNiftiFromFilePath(path, progressDialog);
    progressDialog->deleteLater();

    if (!uStatus::g_ctimage.checkLoadSuccess()) {
        progressDialog->close();
        QMessageBox::warning(this,"warning", "failed to load ct series!");
        return;
    }
    blockAllSignals(false);
    showImage(&uStatus::g_ctimage);
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
                ui->widget_imageProcess->show();
                ui->widget_src->show();
                ui->widget_segment_container->show();
                ui->widget_3d_postProcess->show();
                ui->widget_chart->show();
                ui->gridLayout->addWidget(ui->widget_3d, 1, 0);
            }else
            {
                ui->widget_imageProcess->hide();
                ui->widget_src->hide();
                ui->widget_segment_container->hide();
                ui->widget_3d_postProcess->hide();
                ui->widget_chart->hide();
                ui->gridLayout->addWidget(ui->widget_3d, 0, 0, 3, 3);
            }
            isShowMax = !isShowMax;
        }
        else if (watched == ui->widget_segment)
        {
            if (isShowMax)
            {
                ui->gridLayout->removeWidget(ui->widget_segment_container);
                ui->widget_imageProcess->show();
                ui->widget_src->show();
                ui->widget_3d->show();
                ui->widget_3d_postProcess->show();
                ui->widget_chart->show();
                ui->gridLayout->addWidget(ui->widget_segment_container, 0, 2);
            }
            else
            {
                ui->widget_imageProcess->hide();
                ui->widget_src->hide();
                ui->widget_3d->hide();
                ui->widget_3d_postProcess->hide();
                ui->widget_chart->hide();
                ui->gridLayout->addWidget(ui->widget_segment_container, 0, 0, 3, 3);
            }
            isShowMax = !isShowMax;
        }
        else if (watched == ui->widget_imageProcess)
        {
            if (isShowMax)
            {
                ui->gridLayout->removeWidget(ui->widget_imageProcess);
                ui->widget_3d->show();
                ui->widget_src->show();
                ui->widget_segment_container->show();
                ui->widget_3d_postProcess->show();
                ui->widget_chart->show();
                ui->gridLayout->addWidget(ui->widget_imageProcess, 0, 1);
            }
            else
            {
                ui->widget_3d->hide();
                ui->widget_src->hide();
                ui->widget_segment_container->hide();
                ui->widget_3d_postProcess->hide();
                ui->widget_chart->hide();
                ui->gridLayout->addWidget(ui->widget_imageProcess, 0, 0, 3, 3);
            }
            isShowMax = !isShowMax;
        }
        else if (watched == ui->widget_src)
        {
            if (isShowMax)
            {
                ui->gridLayout->removeWidget(ui->widget_src);
                ui->widget_imageProcess->show();
                ui->widget_3d->show();
                ui->widget_segment_container->show();
                ui->widget_3d_postProcess->show();
                ui->widget_chart->show();
                ui->gridLayout->addWidget(ui->widget_src, 0, 0);
            }
            else
            {
                ui->widget_imageProcess->hide();
                ui->widget_3d->hide();
                ui->widget_segment_container->hide();
                ui->widget_3d_postProcess->hide();
                ui->widget_chart->hide();
                ui->gridLayout->addWidget(ui->widget_src, 0, 0, 3, 3);
            }
            isShowMax = !isShowMax;
        }
        else if (watched == ui->widget_3d_postProcess)
        {
            if (isShowMax)
            {
                ui->gridLayout->removeWidget(ui->widget_3d_postProcess);
                ui->widget_imageProcess->show();
                ui->widget_3d->show();
                ui->widget_segment_container->show();
                ui->widget_src->show();
                ui->widget_chart->show();
                ui->gridLayout->addWidget(ui->widget_3d_postProcess, 1, 1);
            }
            else
            {
                ui->widget_imageProcess->hide();
                ui->widget_3d->hide();
                ui->widget_segment_container->hide();
                ui->widget_src->hide();
                ui->widget_chart->hide();
                ui->gridLayout->addWidget(ui->widget_3d_postProcess, 0, 0, 3, 3);
            }
            isShowMax = !isShowMax;
        }
        else if (watched == ui->widget_chart)
        {
        if (isShowMax)
        {
            ui->gridLayout->removeWidget(ui->widget_chart);
            ui->widget_imageProcess->show();
            ui->widget_3d->show();
            ui->widget_segment_container->show();
            ui->widget_src->show();
            ui->widget_3d_postProcess->show();
            ui->gridLayout->addWidget(ui->widget_chart, 1, 2);
        }
        else
        {
            ui->widget_imageProcess->hide();
            ui->widget_3d->hide();
            ui->widget_segment_container->hide();
            ui->widget_src->hide();
            ui->widget_3d_postProcess->hide();
            ui->gridLayout->addWidget(ui->widget_chart, 0, 0, 3, 3);
        }
        isShowMax = !isShowMax;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::on_pushButton_scanNifti_clicked()
{
    blockAllSignals(true);
    QStringList filenames=QFileDialog::getOpenFileNames(this, "Image Path", "../", "*.nii");
    if (filenames.size() == 0)
    {
        QMessageBox::warning(this, "warnning", "please select at least a series!");
        return;
    }
    QString filename = filenames[0];
    QProgressDialog* progressDialog = createProgressDialog(tr("Image Loading"), tr("Loading Image, Please Wait..."), 101);
    progressDialog->setWindowFlag(Qt::WindowStaysOnTopHint);
    bool ret=uStatus::g_ctimage.loadNiftiFromFilePath(filename, progressDialog);
    if (!ret)
    {
        QMessageBox::warning(this, "warnning", "failed to load ct series!");
        return;
    }
        
    progressDialog->deleteLater();

    // check the path is valid
    if (!uStatus::g_ctimage.checkLoadSuccess()) {
        progressDialog->close();
        return;
    }
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

void MainWindow::on_pushButton_exit_clicked()
{
    qApp->quit();
}

void MainWindow::on_pushButton_reset_clicked()
{
    ui->widget_src->Reset();
    ui->widget_imageProcess->Reset();
    ui->widget_segment->Reset();
}

void MainWindow::on_pushButton_register_clicked()
{
    setEnabled(false);
    algorthom_.doRegister();
}

void MainWindow::on_pushButton_segment_clicked()
{
    algorthom_.doSegment(ui->horizontalSlider_threshold->value());
    ui->widget_segment->Render();
}

void MainWindow::slot_draw()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if(action== roi_rectangle)
    {
        ui->widget_segment->setDrawModeToDrawRectangle();
    }
	else if(action==roi_ellipse)
    {
        ui->widget_segment->setDrawModeToDrawEllipse();
    }
    else if(action==roi_apply)
    {
        ui->widget_segment->applyROI();
        if (series) {
            ui->widget_chart->chart()->removeSeries(series);
            series->deleteLater();
        }
        series = new QLineSeries;
        std::map<int,float> mp=uStatus::g_ctimage.getAverageMap();
        for (auto m : mp) {
            series->append(m.first, m.second);
        }
        ui->widget_chart->chart()->addSeries(series);
    }
    else if(action==endroi)
    {
        ui->widget_segment->endDraw();
    }
}

void MainWindow::slot_register_finished()
{
    qDebug() << "register finished";
    ui->widget_imageProcess->setCTImage(&uStatus::g_ctimage);
    ui->widget_imageProcess->Reset();
    ui->widget_segment->setCTImage(&uStatus::g_ctimage);
    ui->widget_segment->Reset();
    setEnabled(true);
}

void MainWindow::slot_thresholdvalue_changed(int v)
{

}
