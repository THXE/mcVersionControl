#ifndef JSONHELPER_H
#define JSONHELPER_H

#include <QString>
#include <QMap>
#include <QList>
#include "Core/ProjectManager.h"
#include "Core/VersionManager.h"

namespace JsonHelper {

/// Read a manifest.json from a pack directory
bool loadManifest(const QString &path, ProjectInfo &out);

/// Read the .backup/guidance.json file
bool loadGuidance(const QString &backupDir, QList<VersionInfo> &out);

/// Write the .backup/guidance.json file
bool saveGuidance(const QString &backupDir, const ProjectInfo &project,
                  const QList<VersionInfo> &versions, const QStringList &packDirs);

/// Load a language JSON file (key -> localized string)
bool loadLangFile(const QString &path, QMap<QString, QString> &out);

} // namespace JsonHelper

#endif // JSONHELPER_H
