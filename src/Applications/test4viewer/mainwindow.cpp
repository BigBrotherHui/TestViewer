#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <filesystem>

#include <QMimeData>
#include <QDropEvent>
#include <QDebug>
#include <QFileInfo>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QTextEdit>

#include "vvImage.h"
#include "vvImageReader.h"
#include "widget.h"
namespace fs = std::filesystem;

std::string convertToForwardSlashes(const std::string& path)
{
    std::string result = path;
    std::replace(result.begin(), result.end(), '\\', '/');
    return result;
}
std::vector<std::string> getFiles(const std::string& directoryPath)
{
    std::vector<std::string> files;

    if (!fs::exists(directoryPath)) {
        std::cerr << "dir not exist" << std::endl;
        return files;
    }

    for (const auto& entry : fs::recursive_directory_iterator(directoryPath)) {
        if (fs::is_regular_file(entry.path())) {            
            std::string path = entry.path().string();
            path = convertToForwardSlashes(path);
            files.push_back(path);
        }
    }

    return files;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setAcceptDrops(true);
    m_widget = new Widget(this);
    setCentralWidget(m_widget);
    /*m_mdiArea = new QMdiArea(this);
    setCentralWidget(m_mdiArea);*/
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();  // 3. 有拖拽文件时设置接受
    }
}

void MainWindow::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        if (urls.isEmpty()) {
            return;
        }
        QStringList pathList;
        for (int i = 0; i < urls.size(); ++i) {
            pathList.append(urls.at(i).toLocalFile());
        }
        load(pathList);
    }
}

void MainWindow::load(QStringList paths)
{
    if (paths.isEmpty())
    {
        qDebug() << "path is empty";
        return;
    }
    int dirCount{0}, fileCount{0};
    for (QString path : paths) {
        QFileInfo info(path);
        if (dirCount==0 && info.isDir()) {
            ++dirCount;
        }
        else if (fileCount==0 && info.isFile()) {
            ++fileCount;
        }
        if (dirCount && fileCount) {
            qDebug() << "can't add both dir and files";
            return;
        }
        if (dirCount > 1) {
            qDebug() << "can't add more than one dir";
            return;
        }
    }
    if (dirCount==0)
    {
        loadFiles(paths);
    }else
    {
        loadDirectory(paths[0]);
    }
}

void MainWindow::loadDirectory(QString path)
{
    auto files = getFiles(path.toStdString());
    vvImageReader::Pointer reader = vvImageReader::New();
    reader->SetInputFilenames(files);
    reader->Update();
    vtkImageData *imageData=reader->GetOutput()->GetFirstVTKImageData();
    m_widget->setInputImage(imageData);
    m_widget->renderImage();
}

void MainWindow::loadFiles(QStringList files)
{

}

MainWindow::~MainWindow()
{
    delete ui;
}

