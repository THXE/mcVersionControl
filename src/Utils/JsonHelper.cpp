#include "JsonHelper.h"
#include "FileScanner.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

namespace JsonHelper {

bool loadManifest(const QString &projectDir, ProjectInfo &out)
{
    // 1. Try studio.json first (MC Studio project metadata)
    QString studioPath = projectDir + "/studio.json";
    QFile sFile(studioPath);
    if (sFile.open(QFile::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(sFile.readAll());
        sFile.close();
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            out.name   = obj.value("EditName").toString(out.name);
            out.author = obj.value("UserName").toString(out.author);
            out.id     = obj.value("Id").toString(out.id);
        }
    }

    // 2. Fallback: parse first found manifest.json for header info
    QStringList manifests = FileScanner::findManifestFiles(projectDir);
    if (manifests.isEmpty())
        return false;

    // Determine pack types
    QStringList types;
    for (const QString &mp : manifests) {
        QString t = FileScanner::determinePackType(mp);
        if (t != "unknown") types.append(t);
    }
    out.packType = types.join(",");

    // Fill name/id from manifest if not set by studio.json
    if (out.name.isEmpty() || out.author.isEmpty()) {
        QFile mFile(manifests.first());
        if (mFile.open(QFile::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(mFile.readAll());
            mFile.close();
            if (doc.isObject()) {
                QJsonObject header = doc.object().value("header").toObject();
                if (out.name.isEmpty())
                    out.name = header.value("name").toString();
                if (out.author.isEmpty())
                    out.author = header.value("author").toString();
                if (out.id.isEmpty())
                    out.id = header.value("uuid").toString();
            }
        }
    }

    if (out.name.isEmpty()) {
        out.name = QDir(projectDir).dirName();
    }

    out.path = projectDir;
    return true;
}

bool loadGuidance(const QString &backupDir, QList<VersionInfo> &out)
{
    QString filePath = backupDir + "/guidance.json";
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "Cannot open guidance.json:" << filePath;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << parseError.errorString();
        return false;
    }

    QJsonObject root = doc.object();
    // name and id will be used when we store more info
    // QString name = root.value("name").toString();
    // QString id   = root.value("id").toString();

    QJsonArray versionArray = root.value("version").toArray();
    for (const QJsonValue &val : versionArray) {
        QJsonObject verObj = val.toObject();
        VersionInfo info;
        info.version    = verObj.value("version").toString();
        info.source     = verObj.value("source").toString();
        info.timestamp  = verObj.value("timestamp").toString();
        info.size       = verObj.value("size").toString();
        info.backupPath = verObj.value("backupPath").toString();
        // Read packs list
        QJsonArray packsArr = verObj.value("packs").toArray();
        for (const QJsonValue &p : packsArr) {
            info.packs.append(p.toString());
        }
        // Fallback: if no packs stored, scan zip to find pack dirs
        out.append(info);
    }

    return true;
}

bool saveGuidance(const QString &backupDir, const ProjectInfo &project,
                  const QList<VersionInfo> &versions, const QStringList &packDirs)
{
    QJsonObject root;
    root["name"] = project.name;
    root["id"]   = project.id;

    QJsonArray versionArray;
    for (const auto &ver : versions) {
        QJsonObject verObj;
        verObj["version"]    = ver.version;
        verObj["source"]     = ver.source;
        verObj["timestamp"]  = ver.timestamp;
        verObj["size"]       = ver.size;
        verObj["backupPath"] = ver.backupPath;
        QJsonArray packs;
        for (const QString &p : packDirs) packs.append(p);
        verObj["packs"] = packs;
        versionArray.append(verObj);
    }
    root["version"] = versionArray;

    QJsonDocument doc(root);
    QFile file(backupDir + "/guidance.json");
    if (!file.open(QFile::WriteOnly)) {
        qWarning() << "Cannot write guidance.json:" << backupDir;
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

bool loadLangFile(const QString &path, QMap<QString, QString> &out)
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "Cannot open language file:" << path;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Language JSON parse error:" << parseError.errorString();
        return false;
    }

    QJsonObject root = doc.object();
    for (auto it = root.begin(); it != root.end(); ++it) {
        out[it.key()] = it.value().toString();
    }

    return true;
}

} // namespace JsonHelper
