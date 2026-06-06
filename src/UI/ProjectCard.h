#ifndef PROJECTCARD_H
#define PROJECTCARD_H

#include <QWidget>
#include <QLabel>
#include "Core/ProjectManager.h"

class ProjectCard : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool active READ isActive WRITE setActive)

public:
    explicit ProjectCard(const ProjectInfo &info, QWidget *parent = nullptr);
    ~ProjectCard() override = default;

    bool isActive() const { return m_active; }
    void setActive(bool active);

    ProjectInfo projectInfo() const { return m_info; }
    int projectIndex() const { return m_index; }
    void setProjectIndex(int idx) { m_index = idx; }

signals:
    void clicked();
    void doubleClicked();
    void contextMenuRequested(const QPoint &globalPos);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void setupUi();

    QLabel *m_nameLabel   = nullptr;
    QLabel *m_authorLabel = nullptr;
    QLabel *m_idLabel     = nullptr;

    bool m_active  = false;
    bool m_hovered = false;
    int  m_index   = -1;
    ProjectInfo m_info;
};

#endif // PROJECTCARD_H
