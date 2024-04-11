#pragma once

#include <QString>
#include <QPixmap>
#include <QMap>
class ImageRegister
{
public:
    ImageRegister(const QMap<QString,QString> &map,QString filePath);
    ImageRegister(QString imageCachePath);
    ~ImageRegister();
    void save();
    QMap<QString, QString> getMap();
    QString getSavePath();
private:
    // location of the registry
    QString cachePath = "./cache/";
    QMap<QString, QString> mMap;
    QString mPath;
    QString mSavePath;
    // descriptor for the file
};