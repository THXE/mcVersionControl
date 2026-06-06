#include "FileScanner.h"

#include <QDirIterator>
#include <QFileInfo>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

namespace FileScanner {

QStringList findManifestFiles(const QString &rootDir)
{
    QStringList results;
    QDirIterator it(rootDir, QDir::Dirs | QDir::NoDotAndDotDot,
                    QDirIterator::Subdirectories);

    while (it.hasNext()) {
        it.next();
        QString manifestPath = it.filePath() + "/manifest.json";
        if (QFileInfo::exists(manifestPath)) {
            results.append(manifestPath);
            qDebug() << "Found manifest at:" << manifestPath;
        }
    }

    return results;
}

QString determinePackType(const QString &manifestPath)
{
    QFile file(manifestPath);
    if (!file.open(QFile::ReadOnly))
        return "unknown";

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject())
        return "unknown";

    QJsonArray modules = doc.object().value("modules").toArray();
    for (const QJsonValue &val : modules) {
        QJsonObject mod = val.toObject();
        QString type = mod.value("type").toString();
        if (type == "resources")
            return "resource";
        if (type == "data")
            return "behavior";
    }
    return "unknown";
}

qint64 dirSize(const QString &path)
{
    qint64 total = 0;
    QDirIterator it(path, QDir::Files | QDir::NoDotAndDotDot,
                    QDirIterator::Subdirectories);

    while (it.hasNext()) {
        it.next();
        total += it.fileInfo().size();
    }

    return total;
}

} // namespace FileScanner
