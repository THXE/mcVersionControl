#ifndef PROJECTPANEL_H
#define PROJECTPANEL_H

#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QList>
#include <QMenu>
#include "Core/ProjectManager.h"

class ProjectCard;

class ProjectPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectPanel(QWidget *parent = nullptr);
    ~ProjectPanel() override;

    void addProject(const ProjectInfo &info);
    void removeProject(int index);
    void clearProjects();
    void setActiveProject(int index);
    int activeIndex() const { return m_activeIndex; }
    int projectCount() const { return m_cards.size(); }

signals:
    void projectSelected(int index);
    void projectDoubleClicked(const QString &path);
    void projectContextMenu(int index, const QPoint &globalPos);
    void backupRequested(int index);
    void deleteRequested(int index);

private slots:
    void onCardClicked();
    void onCardDoubleClicked();
    void onCardContextMenu(const QPoint &globalPos);

private:
    void setupUi();
    ProjectCard *senderCard() const;

    QLabel       *m_headerLabel   = nullptr;
    QScrollArea  *m_scrollArea    = nullptr;
    QWidget      *m_scrollContent = nullptr;
    QVBoxLayout  *m_cardLayout    = nullptr;

    QList<ProjectCard *> m_cards;
    int m_activeIndex = -1;
};

#endif // PROJECTPANEL_H
