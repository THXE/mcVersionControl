#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPoint>
#include <QRect>
#include <QVariantAnimation>
#include <windows.h>

class TitleBar;
class ProjectPanel;
class VersionPanel;
class VersionManager;
class QSplitter;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void changeEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;

private slots:
    void onMinimize();
    void onFullscreenToggle();
    void onClose();
    void onHelpMenu();
    void onContactMenu();
    void onNavigateBack();
    void onNavigateForward();

    // File menu actions
    void onAddProject();
    void onNewProject();
    void onRefresh();

    // Project context menu
    void onProjectBackup(int index);
    void onProjectDelete(int index);

    // Version context menu
    void onVersionRestore(int versionIndex);
    void onVersionDelete(int versionIndex);

private:
    void setupUi();
    void loadSettings();
    void saveSettings();
    void exitFullscreen();
    int hitTestBorder(long x, long y) const;
    void animateGeometry(const QRect &from, const QRect &to);
    void loadVersionsForProject(int projectIndex);

    TitleBar      *m_titleBar      = nullptr;
    QSplitter     *m_splitter      = nullptr;
    ProjectPanel  *m_projectPanel  = nullptr;
    VersionPanel  *m_versionPanel  = nullptr;
    VersionManager *m_versionMgr   = nullptr;

    // Project data (synced with ProjectPanel card order)
    struct ProjectEntry {
        QString name;
        QString id;
        QString path;
    };
    QList<ProjectEntry> m_projectEntries;

    bool  m_dragging       = false;
    QPoint m_dragStartPos;
    bool  m_acrylicEnabled = false;
    QRect m_normalGeometry;
    bool  m_isFullscreen   = false;
    bool  m_animating      = false;
};

#endif // MAINWINDOW_H
