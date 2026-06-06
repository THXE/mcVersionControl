#include "MainWindow.h"
#include "UI/TitleBar.h"
#include "UI/ProjectPanel.h"
#include "UI/VersionPanel.h"
#include "Core/ProjectManager.h"
#include "Core/VersionManager.h"
#include "Utils/ThemeManager.h"
#include "Utils/JsonHelper.h"
#include "Utils/FileScanner.h"

#include <QVBoxLayout>
#include <QSplitter>
#include <QPainter>
#include <QPropertyAnimation>
#include <QSettings>
#include <QEvent>
#include <QResizeEvent>
#include <QShowEvent>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <windowsx.h>

extern void setWindowCornersRounded(HWND hwnd, bool rounded);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowFlags(Qt::FramelessWindowHint |
                   Qt::WindowSystemMenuHint |
                   Qt::WindowMinMaxButtonsHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_NoSystemBackground, true);

    setMinimumSize(1100, 700);
    resize(1200, 800);

    m_versionMgr = new VersionManager(this);
    setupUi();
    loadSettings();
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::setupUi()
{
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setObjectName("centralWidget");
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_titleBar = new TitleBar(this);
    m_titleBar->setFixedHeight(42);
    m_titleBar->setObjectName("titleBar");
    mainLayout->addWidget(m_titleBar);

    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_splitter->setHandleWidth(1);
    m_splitter->setChildrenCollapsible(false);

    m_projectPanel = new ProjectPanel(this);
    m_splitter->addWidget(m_projectPanel);

    m_versionPanel = new VersionPanel(this);
    m_splitter->addWidget(m_versionPanel);

    m_splitter->setSizes({300, 900});
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);

    mainLayout->addWidget(m_splitter, 1);

    // === TitleBar ===
    connect(m_titleBar, &TitleBar::minimizeClicked,
            this, &MainWindow::onMinimize);
    connect(m_titleBar, &TitleBar::maximizeRestoreClicked,
            this, &MainWindow::onFullscreenToggle);
    connect(m_titleBar, &TitleBar::closeClicked,
            this, &MainWindow::onClose);
    connect(m_titleBar, &TitleBar::helpClicked,
            this, &MainWindow::onHelpMenu);
    connect(m_titleBar, &TitleBar::contactClicked,
            this, &MainWindow::onContactMenu);
    connect(m_titleBar, &TitleBar::navigateBack,
            this, &MainWindow::onNavigateBack);
    connect(m_titleBar, &TitleBar::navigateForward,
            this, &MainWindow::onNavigateForward);
    connect(m_titleBar, &TitleBar::addProjectClicked,
            this, &MainWindow::onAddProject);
    connect(m_titleBar, &TitleBar::newProjectClicked,
            this, &MainWindow::onNewProject);
    connect(m_titleBar, &TitleBar::refreshClicked,
            this, &MainWindow::onRefresh);
    connect(m_titleBar, &TitleBar::settingsClicked,
            this, []() { /* stub */ });

    // === Project panel ===
    connect(m_projectPanel, &ProjectPanel::projectSelected,
            this, &MainWindow::loadVersionsForProject);
    connect(m_projectPanel, &ProjectPanel::projectDoubleClicked,
            this, [](const QString &path) {
                QDesktopServices::openUrl(QUrl::fromLocalFile(path));
            });
    connect(m_projectPanel, &ProjectPanel::backupRequested,
            this, &MainWindow::onProjectBackup);
    connect(m_projectPanel, &ProjectPanel::deleteRequested,
            this, &MainWindow::onProjectDelete);

    // === Version panel ===
    connect(m_versionPanel, &VersionPanel::versionRestoreRequested,
            this, &MainWindow::onVersionRestore);
    connect(m_versionPanel, &VersionPanel::versionDeleteRequested,
            this, &MainWindow::onVersionDelete);
}

// ============================================================
//  Window buttons
// ============================================================

void MainWindow::onMinimize()     { showMinimized(); }
void MainWindow::onClose()        { close(); }

void MainWindow::onHelpMenu()     { QDesktopServices::openUrl(QUrl("https://github.com")); }
void MainWindow::onContactMenu()  { QDesktopServices::openUrl(QUrl("https://github.com")); }
void MainWindow::onNavigateBack() {}
void MainWindow::onNavigateForward() {}

// ============================================================
//  File menu actions
// ============================================================

void MainWindow::onAddProject()
{
    QString dir = QFileDialog::getExistingDirectory(
        this, QStringLiteral("选择 AddOn 项目目录"), QString(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty()) return;

    // Scan for packs
    QStringList manifests = FileScanner::findManifestFiles(dir);
    if (manifests.isEmpty()) {
        QMessageBox::information(this,
            QStringLiteral("未找到项目"),
            QStringLiteral("所选目录中未找到 manifest.json 文件，请确认这是 Minecraft AddOn 项目。"));
        return;
    }

    // Parse project info
    ProjectInfo projInfo;
    if (!JsonHelper::loadManifest(dir, projInfo)) {
        QMessageBox::warning(this,
            QStringLiteral("读取失败"),
            QStringLiteral("无法读取项目信息。"));
        return;
    }

    // Add to panel
    m_projectPanel->addProject(projInfo);

    // Store
    m_projectEntries.append({projInfo.name, projInfo.id, projInfo.path});

    // Select and load versions
    int idx = m_projectPanel->projectCount() - 1;
    m_projectPanel->setActiveProject(idx);
}

void MainWindow::onNewProject()
{
    QMessageBox::information(this,
        QStringLiteral("新建项目"),
        QStringLiteral("新建 AddOn 项目功能即将推出。"));
}

void MainWindow::onRefresh()
{
    // Re-scan all registered projects and reload versions
    for (int i = 0; i < m_projectEntries.size(); ++i) {
        const auto &entry = m_projectEntries[i];
        ProjectInfo info;
        info.path = entry.path;
        if (JsonHelper::loadManifest(entry.path, info)) {
            m_projectEntries[i].name = info.name;
            m_projectEntries[i].id = info.id;
        }
    }

    // Clear and reload version panel
    int activeIdx = m_projectPanel->activeIndex();
    if (activeIdx >= 0) {
        m_versionMgr->clear();
        loadVersionsForProject(activeIdx);
    }
}

// ============================================================
//  Version loading
// ============================================================

void MainWindow::loadVersionsForProject(int projectIndex)
{
    if (projectIndex < 0 || projectIndex >= m_projectEntries.size())
        return;

    const auto &entry = m_projectEntries[projectIndex];
    QString backupDir = entry.path + "/.backup";

    m_versionMgr->clear();
    m_versionMgr->loadVersions(backupDir);

    // Reload version panel with new data
    m_versionPanel->refreshTable();
    // Pass versions to panel
    m_versionPanel->setVersions(m_versionMgr->versions());
}

// ============================================================
//  Project context menu actions
// ============================================================

void MainWindow::onProjectBackup(int index)
{
    if (index < 0 || index >= m_projectEntries.size()) return;

    const auto &entry = m_projectEntries[index];

    // Auto-suggest next version number
    int nextVer = m_versionMgr->versionCount() + 1;
    QString defaultLabel = QString::number(nextVer) + ".0";

    bool ok;
    QString label = QInputDialog::getText(this,
        QStringLiteral("备份"),
        QStringLiteral("请输入版本标签："),
        QLineEdit::Normal,
        defaultLabel,
        &ok);

    if (!ok || label.isEmpty()) return;

    if (m_versionMgr->createBackup(entry.path, entry.name, entry.id, label)) {
        loadVersionsForProject(index);
    } else {
        QMessageBox::warning(this,
            QStringLiteral("备份失败"),
            QStringLiteral("创建备份时发生错误。"));
    }
}

void MainWindow::onProjectDelete(int index)
{
    if (index < 0 || index >= m_projectEntries.size()) return;

    const auto &entry = m_projectEntries[index];

    int ret = QMessageBox::question(this,
        QStringLiteral("删除项目"),
        QString("确定要删除 \"%1\" 的版本控制数据吗？\n这只会删除 .backup 目录，不会影响项目文件。")
            .arg(entry.name),
        QMessageBox::Yes | QMessageBox::No);

    if (ret != QMessageBox::Yes) return;

    // Remove .backup directory
    QString backupDir = entry.path + "/.backup";
    QDir d(backupDir);
    if (d.exists()) {
        d.removeRecursively();
    }

    m_projectEntries.removeAt(index);
    m_projectPanel->removeProject(index);

    // Reload
    if (m_projectEntries.isEmpty()) {
        m_versionMgr->clear();
        m_versionPanel->refreshTable();
    }
}

// ============================================================
//  Version context menu actions
// ============================================================

void MainWindow::onVersionRestore(int versionIndex)
{
    int projIdx = m_projectPanel->activeIndex();
    if (projIdx < 0 || projIdx >= m_projectEntries.size()) return;

    int ret = QMessageBox::question(this,
        QStringLiteral("恢复版本"),
        QStringLiteral("确定要恢复到此版本吗？当前的文件将被覆盖。"),
        QMessageBox::Yes | QMessageBox::No);

    if (ret != QMessageBox::Yes) return;

    if (m_versionMgr->restoreBackup(m_projectEntries[projIdx].path, versionIndex)) {
        QMessageBox::information(this,
            QStringLiteral("恢复完成"),
            QStringLiteral("已成功恢复到选定版本。"));
    } else {
        // Auto-refresh to remove missing backup from list
        loadVersionsForProject(projIdx);
        QMessageBox::critical(this,
            QStringLiteral("恢复失败"),
            QStringLiteral("备份文件不存在或已损坏，当前项目文件未被修改。\n版本列表已自动刷新。"));
    }
}

void MainWindow::onVersionDelete(int versionIndex)
{
    int projIdx = m_projectPanel->activeIndex();
    if (projIdx < 0 || projIdx >= m_projectEntries.size()) return;

    int ret = QMessageBox::question(this,
        QStringLiteral("删除备份"),
        QStringLiteral("确定要删除此备份吗？此操作不可撤销。"),
        QMessageBox::Yes | QMessageBox::No);

    if (ret != QMessageBox::Yes) return;

    if (m_versionMgr->deleteBackup(m_projectEntries[projIdx].path, versionIndex)) {
        loadVersionsForProject(projIdx);
    }
}

// ============================================================
//  Show / Paint / Change / Resize / Move / Native
// ============================================================

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    auto *anim = new QPropertyAnimation(this, "windowOpacity", this);
    anim->setDuration(250);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(ThemeColors::bgMainTrans());
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect());
}

void MainWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange)
        m_titleBar->setFullscreen(m_isFullscreen);
}

void MainWindow::onFullscreenToggle()
{
    if (m_animating) return;
    if (m_isFullscreen) {
        exitFullscreen();
    } else {
        m_normalGeometry = geometry();
        m_isFullscreen = true;
        setWindowCornersRounded(reinterpret_cast<HWND>(winId()), false);
        m_titleBar->setFullscreen(true);
        animateGeometry(m_normalGeometry, screen()->availableGeometry());
    }
}

void MainWindow::exitFullscreen()
{
    if (!m_isFullscreen) return;
    m_isFullscreen = false;
    setWindowCornersRounded(reinterpret_cast<HWND>(winId()), true);
    m_titleBar->setFullscreen(false);
    QRect target = m_normalGeometry.isValid() ? m_normalGeometry : QRect(100, 100, 1200, 800);
    animateGeometry(geometry(), target);
}

void MainWindow::animateGeometry(const QRect &from, const QRect &to)
{
    m_animating = true;
    auto *anim = new QVariantAnimation(this);
    anim->setDuration(350);
    anim->setStartValue(from);
    anim->setEndValue(to);
    anim->setEasingCurve(QEasingCurve::InOutCubic);
    connect(anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        setGeometry(value.toRect());
    });
    connect(anim, &QVariantAnimation::finished, this, [this]() {
        m_animating = false;
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::moveEvent(QMoveEvent *event)
{
    QMainWindow::moveEvent(event);
    if (!m_animating && m_isFullscreen && geometry() != screen()->availableGeometry())
        exitFullscreen();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (!m_animating && m_isFullscreen && geometry() != screen()->availableGeometry())
        exitFullscreen();
    saveSettings();
}

void MainWindow::loadSettings()
{
    QSettings settings("MCVC", "MCVersionControl");
    settings.beginGroup("MainWindow");
    resize(settings.value("size", QSize(1200, 800)).toSize());
    move(settings.value("pos", QPoint(100, 100)).toPoint());
    if (settings.value("maximized", false).toBool()) showMaximized();
    if (m_splitter) m_splitter->restoreState(settings.value("splitterState").toByteArray());
    settings.endGroup();
}

void MainWindow::saveSettings()
{
    QSettings settings("MCVC", "MCVersionControl");
    settings.beginGroup("MainWindow");
    if (!isMaximized()) {
        settings.setValue("size", size());
        settings.setValue("pos", pos());
    }
    settings.setValue("maximized", isMaximized());
    if (m_splitter) settings.setValue("splitterState", m_splitter->saveState());
    settings.endGroup();
}

int MainWindow::hitTestBorder(long x, long y) const
{
    static const int BORDER = 8;
    RECT winRect;
    GetWindowRect(reinterpret_cast<HWND>(winId()), &winRect);

    int frameX = x - winRect.left;
    int frameY = y - winRect.top;
    int width  = winRect.right - winRect.left;
    int height = winRect.bottom - winRect.top;

    bool left   = (frameX <= BORDER);
    bool right  = (frameX >= width - BORDER);
    bool top    = (frameY <= BORDER);
    bool bottom = (frameY >= height - BORDER);

    if (top && left)     return HTTOPLEFT;
    if (top && right)    return HTTOPRIGHT;
    if (bottom && left)  return HTBOTTOMLEFT;
    if (bottom && right) return HTBOTTOMRIGHT;
    if (left)            return HTLEFT;
    if (right)           return HTRIGHT;
    if (top)             return HTTOP;
    if (bottom)          return HTBOTTOM;

    return HTCLIENT;
}

bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
    Q_UNUSED(eventType)
    MSG *msg = static_cast<MSG *>(message);

    switch (msg->message) {
    case WM_NCHITTEST: {
        long defResult = HTCLIENT;
        POINT pt = {GET_X_LPARAM(msg->lParam), GET_Y_LPARAM(msg->lParam)};
        ScreenToClient(msg->hwnd, &pt);
        int ht = hitTestBorder(GET_X_LPARAM(msg->lParam), GET_Y_LPARAM(msg->lParam));

        if (ht == HTCLIENT && m_titleBar) {
            QPoint titlePt = m_titleBar->mapFromGlobal(
                QPoint(GET_X_LPARAM(msg->lParam), GET_Y_LPARAM(msg->lParam)));
            if (m_titleBar->rect().contains(titlePt) && m_titleBar->isDragZone(titlePt)) {
                ht = HTCAPTION;
            }
        }
        if (ht != HTCLIENT) {
            *result = ht;
            return true;
        }
        *result = defResult;
        return true;
    }

    case WM_NCCALCSIZE: {
        if (msg->wParam == TRUE) { *result = 0; return true; }
        break;
    }

    case WM_SETTINGCHANGE: {
        if (msg->lParam && wcscmp(reinterpret_cast<LPCWSTR>(msg->lParam), L"ImmersiveColorSet") == 0) {
            ThemeManager::instance().syncWithSystem();
        }
        *result = 0;
        return true;
    }

    case WM_GETMINMAXINFO: {
        MINMAXINFO *mmi = reinterpret_cast<MINMAXINFO *>(msg->lParam);
        mmi->ptMinTrackSize.x = 1100;
        mmi->ptMinTrackSize.y = 700;
        *result = 0;
        return true;
    }
    }

    return QMainWindow::nativeEvent(eventType, message, result);
}
