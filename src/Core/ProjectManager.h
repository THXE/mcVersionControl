#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QObject>
#include <QString>
#include <QList>

/// Data structure for a single Minecraft AddOn project
struct ProjectInfo {
    QString name;       // EditName from studio.json / manifest
    QString author;     // UserName
    QString id;         // UUID string
    QString path;       // Filesystem path to the project directory
    QString packType;   // "behavior" | "resource" | "unknown"
};

class ProjectManager : public QObject
{
    Q_OBJECT

public:
    explicit ProjectManager(QObject *parent = nullptr);
    ~ProjectManager() override = default;

    const QList<ProjectInfo> &projects() const { return m_projects; }
    int projectCount() const { return m_projects.size(); }
    ProjectInfo projectAt(int index) const;

    void addProject(const ProjectInfo &info);
    void removeProject(int index);
    void clear();

signals:
    void projectAdded(const ProjectInfo &info);
    void projectRemoved(int index);
    void projectUpdated(int index);
    void projectsLoaded(const QList<ProjectInfo> &projects);

private:
    QList<ProjectInfo> m_projects;
};

#endif // PROJECTMANAGER_H
