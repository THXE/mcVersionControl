#ifndef VERSIONMANAGER_H
#define VERSIONMANAGER_H

#include <QObject>
#include <QString>
#include <QList>

/// Data structure for a single version / backup entry
struct VersionInfo {
    QString version;    // User-entered version label, e.g. "1.0"
    QString source;     // Project name detected by the app
    QString timestamp;  // Backup creation time, precise to seconds
    QString size;       // Backup file size (human-readable)
    QString backupPath; // Path to the backup folder
    QStringList packs;  // Pack dir names in this backup
};

class VersionManager : public QObject
{
    Q_OBJECT

public:
    explicit VersionManager(QObject *parent = nullptr);
    ~VersionManager() override = default;

    const QList<VersionInfo> &versions() const { return m_versions; }
    int versionCount() const { return m_versions.size(); }
    VersionInfo versionAt(int index) const;

    /// Load versions from an existing .backup/guidance.json
    void loadVersions(const QString &backupDir);

    /// Create a new backup for a project
    bool createBackup(const QString &projectPath, const QString &projectName,
                      const QString &projectId, const QString &versionLabel);

    /// Restore project packs from a specific backup
    bool restoreBackup(const QString &projectPath, int versionIndex);

    /// Delete a backup
    bool deleteBackup(const QString &projectPath, int versionIndex);

    /// Clear all version data in memory
    void clear();

signals:
    void versionAdded(const VersionInfo &info);
    void versionRemoved(int index);
    void versionRestored(int index);
    void versionsLoaded(const QList<VersionInfo> &versions);

private:
    /// Copy a directory recursively
    static bool copyDir(const QString &src, const QString &dst);

    /// Remove a directory recursively
    static bool removeDir(const QString &path);

    /// Format bytes to human-readable string
    static QString formatSize(qint64 bytes);

    /// Get timestamp string for backup folder naming
    static QString backupTimestamp();

    /// Write guidance.json for the project
    bool writeGuidance(const QString &backupDir, const QString &projectName,
                       const QString &projectId);

    /// Get backup base dir: projectPath/.backup
    static QString backupBaseDir(const QString &projectPath);

    QList<VersionInfo> m_versions;
};

#endif // VERSIONMANAGER_H
