#include "ProjectManager.h"

ProjectManager::ProjectManager(QObject *parent)
    : QObject(parent)
{
}

ProjectInfo ProjectManager::projectAt(int index) const
{
    if (index >= 0 && index < m_projects.size()) {
        return m_projects.at(index);
    }
    return {};
}

void ProjectManager::addProject(const ProjectInfo &info)
{
    m_projects.append(info);
    emit projectAdded(info);
    emit projectUpdated(m_projects.size() - 1);
}

void ProjectManager::removeProject(int index)
{
    if (index >= 0 && index < m_projects.size()) {
        m_projects.removeAt(index);
        emit projectRemoved(index);
    }
}

void ProjectManager::clear()
{
    m_projects.clear();
}
