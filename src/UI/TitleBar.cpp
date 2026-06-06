#include "TitleBar.h"
#include "Utils/ThemeManager.h"

#include <QHBoxLayout>
#include <QMenu>
#include <QMouseEvent>
#include <QWindow>
#include <QIcon>

TitleBar::TitleBar(QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(42);
    setObjectName("titleBar");
    setAutoFillBackground(true);
    setupUi();
}

void TitleBar::setupUi()
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // === Logo ===
    m_logoLabel = new QLabel(this);
    m_logoLabel->setFixedSize(42, 42);
    m_logoLabel->setPixmap(
        QPixmap(":/icon.png").scaled(36, 36, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_logoLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_logoLabel);

    // === Menu buttons ===
    m_fileBtn = new QPushButton(QStringLiteral("文件"), this);    // 文件
    m_fileBtn->setObjectName("menuFile");
    m_fileBtn->setFixedSize(84, 42);
    m_fileBtn->setCursor(Qt::ArrowCursor);
    layout->addWidget(m_fileBtn);

    m_helpBtn = new QPushButton(QStringLiteral("帮助"), this);    // 帮助
    m_helpBtn->setObjectName("menuHelp");
    m_helpBtn->setFixedSize(84, 42);
    m_helpBtn->setCursor(Qt::ArrowCursor);
    layout->addWidget(m_helpBtn);

    m_contactBtn = new QPushButton(QStringLiteral("联系我们"), this); // 联系我们
    m_contactBtn->setObjectName("menuContact");
    m_contactBtn->setFixedSize(138, 42);
    m_contactBtn->setCursor(Qt::ArrowCursor);
    layout->addWidget(m_contactBtn);

    // === Navigation buttons ===
    m_backBtn = new QPushButton(QStringLiteral("←"), this);           // ←
    m_backBtn->setObjectName("menuBack");
    m_backBtn->setFixedSize(42, 42);
    m_backBtn->setCursor(Qt::ArrowCursor);
    layout->addWidget(m_backBtn);

    m_forwardBtn = new QPushButton(QStringLiteral("→"), this);        // →
    m_forwardBtn->setObjectName("menuForward");
    m_forwardBtn->setFixedSize(42, 42);
    m_forwardBtn->setCursor(Qt::ArrowCursor);
    layout->addWidget(m_forwardBtn);

    // === Spacer (push window controls to the right) ===
    layout->addStretch(1);

    // === Window control buttons ===
    m_minBtn = new QPushButton(QStringLiteral("─"), this);            // ─
    m_minBtn->setObjectName("minButton");
    m_minBtn->setFixedSize(40, 28);
    m_minBtn->setCursor(Qt::ArrowCursor);
    layout->addWidget(m_minBtn);

    m_maxBtn = new QPushButton(QStringLiteral("□"), this);            // □
    m_maxBtn->setObjectName("maxButton");
    m_maxBtn->setFixedSize(40, 28);
    m_maxBtn->setCursor(Qt::ArrowCursor);
    layout->addWidget(m_maxBtn);

    m_closeBtn = new QPushButton(QStringLiteral("✕"), this);          // ✕
    m_closeBtn->setObjectName("closeButton");
    m_closeBtn->setFixedSize(40, 28);
    m_closeBtn->setCursor(Qt::ArrowCursor);
    layout->addWidget(m_closeBtn);

    // === Connect signals ===
    connect(m_fileBtn, &QPushButton::clicked,
            this, &TitleBar::showFileMenu);
    connect(m_helpBtn, &QPushButton::clicked,
            this, &TitleBar::helpClicked);
    connect(m_contactBtn, &QPushButton::clicked,
            this, &TitleBar::contactClicked);
    connect(m_backBtn, &QPushButton::clicked,
            this, &TitleBar::navigateBack);
    connect(m_forwardBtn, &QPushButton::clicked,
            this, &TitleBar::navigateForward);
    connect(m_minBtn, &QPushButton::clicked,
            this, &TitleBar::minimizeClicked);
    connect(m_maxBtn, &QPushButton::clicked,
            this, &TitleBar::maximizeRestoreClicked);
    connect(m_closeBtn, &QPushButton::clicked,
            this, &TitleBar::closeClicked);
}

// ============================================================
//  File menu dropdown
// ============================================================

void TitleBar::showFileMenu()
{
    QMenu menu(this);
    menu.setObjectName("fileMenu");

    QAction *addAction     = menu.addAction(QStringLiteral("添加 AddOn 项目"));
    QAction *newAction     = menu.addAction(QStringLiteral("新建 AddOn 项目"));
    menu.addSeparator();
    QAction *refreshAction = menu.addAction(QStringLiteral("刷新"));
    menu.addSeparator();
    QAction *settingsAction = menu.addAction(QStringLiteral("设置"));

    // Show menu below the file button
    QPoint pos = m_fileBtn->mapToGlobal(
        QPoint(0, m_fileBtn->height()));
    QAction *chosen = menu.exec(pos);
    if (!chosen) return;

    if (chosen == addAction) {
        emit addProjectClicked();
    } else if (chosen == newAction) {
        emit newProjectClicked();
    } else if (chosen == refreshAction) {
        emit refreshClicked();
    } else if (chosen == settingsAction) {
        emit settingsClicked();
    }
}

void TitleBar::setFullscreen(bool fullscreen)
{
    // □ = go fullscreen, ❐ = exit fullscreen (windowed)
    m_maxBtn->setText(fullscreen ? QStringLiteral("❐") : QStringLiteral("□"));
}

bool TitleBar::isDragZone(const QPoint &pos) const
{
    // Anywhere on the title bar that is NOT a button is a drag zone
    QWidget *child = childAt(pos);
    // If no child under the cursor, it's a drag zone
    if (!child || child == this) {
        return true;
    }
    // Only QPushButton children are excluded
    return !qobject_cast<QPushButton *>(child);
}

// ============================================================
//  Mouse — window dragging
// ============================================================

void TitleBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isDragZone(event->pos())) {
        m_dragging = true;
        m_dragStartPos = event->globalPosition().toPoint() - window()->pos();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void TitleBar::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging) {
        window()->move(event->globalPosition().toPoint() - m_dragStartPos);
        event->accept();
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void TitleBar::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_dragging && event->button() == Qt::LeftButton) {
        m_dragging = false;
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isDragZone(event->pos())) {
        emit maximizeRestoreClicked();
        event->accept();
        return;
    }
    QWidget::mouseDoubleClickEvent(event);
}
