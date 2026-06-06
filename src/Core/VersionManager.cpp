#include "VersionManager.h"
#include "Utils/JsonHelper.h"
#include "Utils/FileScanner.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

VersionManager::VersionManager(QObject *parent)
    : QObject(parent)
{
}

VersionInfo VersionManager::versionAt(int index) const
{
    if (index >= 0 && index < m_versions.size())
        return m_versions.at(index);
    return {};
}

void VersionManager::clear()
{
    m_versions.clear();
}

void VersionManager::loadVersions(const QString &backupDir)
{
    clear();
    QList<VersionInfo> loaded;
    if (JsonHelper::loadGuidance(backupDir, loaded)) {
        // Filter out entries whose backup files don't exist
        for (const auto &v : loaded) {
            QString fullPath = backupDir + "/" + v.backupPath;
            if (QFile::exists(fullPath)) {
                m_versions.append(v);
            } else {
                qDebug() << "Skipping missing backup:" << fullPath;
            }
        }
        // If some were removed, rewrite guidance.json to stay in sync
        if (m_versions.size() != loaded.size()) {
            // Rewrite with existing versions only
            QDir().mkpath(backupDir);
            // Read project info from existing guidance
            QFile f(backupDir + "/guidance.json");
            if (f.open(QFile::ReadOnly)) {
                QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
                f.close();
                QString name = doc.object().value("name").toString();
                QString id   = doc.object().value("id").toString();
                ProjectInfo pi; pi.name = name; pi.id = id;
                QStringList packs;
                if (!m_versions.isEmpty()) packs = m_versions.first().packs;
                JsonHelper::saveGuidance(backupDir, pi, m_versions, packs);
            }
        }
    }
    emit versionsLoaded(m_versions);
}

QString VersionManager::backupBaseDir(const QString &projectPath)
{
    return projectPath + "/.backup";
}

QString VersionManager::backupTimestamp()
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd-HH-mm-ss");
}

QString VersionManager::formatSize(qint64 bytes)
{
    if (bytes < 1024) return QString::number(bytes) + " B";
    double kb = bytes / 1024.0;
    if (kb < 1024) return QString::number(kb, 'f', 1) + " KB";
    double mb = kb / 1024.0;
    return QString::number(mb, 'f', 1) + " MB";
}

// ============================================================
//  Backup
// ============================================================

bool VersionManager::createBackup(const QString &projectPath,
                                   const QString &projectName,
                                   const QString &projectId,
                                   const QString &versionLabel)
{
    // 1. Find pack directories in project
    QStringList manifests = FileScanner::findManifestFiles(projectPath);
    if (manifests.isEmpty()) {
        qWarning() << "No packs found in project:" << projectPath;
        return false;
    }

    QDir projDir(projectPath);
    QStringList packDirs;
    qint64 totalSize = 0;
    for (const QString &m : manifests) {
        QString relDir = projDir.relativeFilePath(QFileInfo(m).absolutePath());
        if (!packDirs.contains(relDir) && relDir != ".") {
            packDirs.append(relDir);
            totalSize += FileScanner::dirSize(projectPath + "/" + relDir);
        }
    }

    // 2. Create backup directories
    QString baseDir = backupBaseDir(projectPath);
    QString timestamp = backupTimestamp();
    QString versionDir = baseDir + "/version";
    QString zipPath = versionDir + "/" + timestamp + ".zip";

    if (!QDir().mkpath(versionDir)) {
        qWarning() << "Failed to create:" << versionDir;
        return false;
    }

    // 3. Build PowerShell command to create .zip
    // cd into project dir so archive uses relative paths (behavior_pack/, resource_pack/)
    QString absZipPath = QDir(baseDir + "/version").absolutePath() + "/" + timestamp + ".zip";

    QStringList pathsToCompress;
    for (const QString &pack : packDirs) {
        pathsToCompress.append(pack);
    }
    QString pathArgs = pathsToCompress.join("\",\"");

    QString psCmd = QString(
        "Set-Location \"%1\"; Compress-Archive -Path \"%2\" -DestinationPath \"%3\" -Force"
    ).arg(QDir(projectPath).absolutePath(), pathArgs, absZipPath);

    QProcess ps;
    ps.start("powershell", {"-NoProfile", "-Command", psCmd});
    ps.waitForFinished(30000);

    if (ps.exitCode() != 0) {
        qWarning() << "PowerShell Compress-Archive failed:" << ps.readAllStandardError();
        // Fallback: copy packs as a directory
        QString fallbackDir = absZipPath;
        fallbackDir.replace(".zip", "");
        QDir().mkpath(fallbackDir);
        for (const QString &pack : packDirs) {
            copyDir(projectPath + "/" + pack, fallbackDir + "/" + pack);
        }
    }

    // 4. Update guidance.json
    VersionInfo info;
    info.version    = versionLabel;
    info.source     = projectName;
    info.timestamp  = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    info.size       = formatSize(totalSize);
    info.backupPath = "version/" + timestamp + ".zip";
    info.packs      = packDirs;

    m_versions.append(info);

    if (!writeGuidance(baseDir, projectName, projectId)) {
        m_versions.removeLast();
        return false;
    }

    emit versionAdded(info);
    return true;
}

// ============================================================
//  Restore
// ============================================================

bool VersionManager::restoreBackup(const QString &projectPath, int versionIndex)
{
    if (versionIndex < 0 || versionIndex >= m_versions.size())
        return false;

    const VersionInfo &info = m_versions.at(versionIndex);
    QString backupFile = backupBaseDir(projectPath) + "/" + info.backupPath;

    // 0. Verify backup file exists BEFORE destroying anything
    if (!QFile::exists(backupFile)) {
        qWarning() << "Backup file not found:" << backupFile;
        return false;
    }

    // 1. Delete current pack dirs from project
    for (const QString &pack : info.packs) {
        QDir d(projectPath + "/" + pack);
        if (d.exists()) d.removeRecursively();
    }

    // 2. Extract .zip into project directory
    if (info.backupPath.endsWith(".zip")) {
        QProcess ps;
        ps.start("powershell", {
            "-NoProfile", "-Command",
            QString("Expand-Archive -Path '%1' -DestinationPath '%2' -Force")
                .arg(QDir::toNativeSeparators(backupFile),
                     QDir::toNativeSeparators(projectPath))
        });
        ps.waitForFinished(30000);

        if (ps.exitCode() != 0) {
            qWarning() << "Expand-Archive failed:" << ps.readAllStandardError();
            return false;
        }
    } else {
        // Old-style directory backup fallback
        for (const QString &pack : info.packs) {
            QString src = backupFile + "/" + pack;
            QString dst = projectPath + "/" + pack;
            if (QDir(src).exists()) copyDir(src, dst);
        }
    }

    emit versionRestored(versionIndex);
    return true;
}

// ============================================================
//  Delete
// ============================================================

bool VersionManager::deleteBackup(const QString &projectPath, int versionIndex)
{
    if (versionIndex < 0 || versionIndex >= m_versions.size())
        return false;

    const VersionInfo &info = m_versions.at(versionIndex);
    QString backupPath = backupBaseDir(projectPath) + "/" + info.backupPath;

    if (info.backupPath.endsWith(".zip")) {
        QFile::remove(backupPath);
    } else {
        removeDir(backupPath);
    }

    m_versions.removeAt(versionIndex);

    // Re-save guidance.json, or remove entire .backup if empty
    QString baseDir = backupBaseDir(projectPath);
    if (m_versions.isEmpty()) {
        removeDir(baseDir);
    }

    emit versionRemoved(versionIndex);
    return true;
}

// ============================================================
//  Utilities
// ============================================================

bool VersionManager::writeGuidance(const QString &backupDir,
                                    const QString &projectName,
                                    const QString &projectId)
{
    ProjectInfo proj;
    proj.name = projectName;
    proj.id   = projectId;

    // Determine pack dirs from first version info
    QStringList packs;
    if (!m_versions.isEmpty()) {
        packs = m_versions.last().packs;
    }

    return JsonHelper::saveGuidance(backupDir, proj, m_versions, packs);
}

bool VersionManager::copyDir(const QString &src, const QString &dst)
{
    QDir srcDir(src);
    if (!srcDir.exists()) return false;

    QDir().mkpath(dst);

    for (const QFileInfo &entry : srcDir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries)) {
        QString srcPath = entry.absoluteFilePath();
        QString dstPath = dst + "/" + entry.fileName();
        if (entry.isDir()) {
            if (!copyDir(srcPath, dstPath)) return false;
        } else {
            if (!QFile::copy(srcPath, dstPath)) {
                qWarning() << "Failed to copy:" << srcPath << "->" << dstPath;
                return false;
            }
        }
    }
    return true;
}

bool VersionManager::removeDir(const QString &path)
{
    QDir dir(path);
    if (!dir.exists()) return true;

    dir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
    for (const QFileInfo &entry : dir.entryInfoList()) {
        if (entry.isDir()) {
            removeDir(entry.absoluteFilePath());
        } else {
            QFile::remove(entry.absoluteFilePath());
        }
    }
    return dir.rmdir(path);
}
