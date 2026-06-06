#ifndef FILESCANNER_H
#define FILESCANNER_H

#include <QString>
#include <QStringList>

namespace FileScanner {

/// Recursively search a directory for subdirectories containing manifest.json.
/// Returns a list of paths to those manifest.json files.
QStringList findManifestFiles(const QString &rootDir);

/// Determine pack type from a manifest.json path.
/// Returns "behavior", "resource", or "unknown".
QString determinePackType(const QString &manifestPath);

/// Calculate total size of a directory in bytes.
qint64 dirSize(const QString &path);

} // namespace FileScanner

#endif // FILESCANNER_H
