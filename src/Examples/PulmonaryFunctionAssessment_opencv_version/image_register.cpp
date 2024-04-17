#include "image_register.h"
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <vtkTransform.h>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

ImageRegister::ImageRegister(const QMap<QString, QString>& map, QString filePath)
{
    QDir dir;
    if (!dir.mkpath(this->cachePath + "imageInfo/"))
    {
        qDebug() << "make directory failed! Cannot save cache.";
        throw 0;
    }
    mMap = map;
    mPath = filePath;
}

ImageRegister::ImageRegister(QString imageCachePath)
{
    QFile loadFile(imageCachePath);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open save file.");
        return;
    }
    QByteArray saveData = loadFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
    QJsonObject imageObject = loadDoc.object();
    mMap["Patient Name"] = imageObject["Patient Name"].toString();
    mMap["Patient's Sex"] = imageObject["Patient's Sex"].toString();
    mMap["Patient's Age"] = imageObject["Patient's Age"].toString();
    mMap["Instance Creation Date"] = imageObject["Instance Creation Date"].toString();
    mMap["filePath"] = imageObject["filePath"].toString();
}

ImageRegister::~ImageRegister()
{
}
// save the class variables to the cache path
void ImageRegister::save()
{
    QJsonObject imageObject;
    // basic info
    for(auto iter= mMap.begin();iter!= mMap.end();iter++)
    {
        imageObject[iter.key()] = iter.value();
    }
    imageObject["filePath"] = mPath;
    // store the json to the path
    QString date = QDateTime::currentDateTime().toString("yyyyMMdd_hh_mm_ss");
    QString json_path = this->cachePath + "imageInfo/" + date + ".json";
    QFile saveFile(json_path);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return;
    }
    mSavePath = json_path;
    saveFile.write(QJsonDocument(imageObject).toJson());
}

QMap<QString, QString> ImageRegister::getMap()
{
    return mMap;
}

QString ImageRegister::getSavePath()
{
    return mSavePath;
}
