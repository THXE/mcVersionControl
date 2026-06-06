#include "VersionPanel.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QShowEvent>

VersionPanel::VersionPanel(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("versionPanel");
    setupUi();
}

VersionPanel::~VersionPanel() = default;

void VersionPanel::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // =============================================
    //  Management bar (42px height)
    // =============================================
    m_managementBar = new QWidget(this);
    m_managementBar->setFixedHeight(42);
    m_managementBar->setObjectName("managementBar");

    QHBoxLayout *manageLayout = new QHBoxLayout(m_managementBar);
    manageLayout->setContentsMargins(0, 0, 0, 0);
    manageLayout->setSpacing(0);

    m_manageLabel = new QLabel(QStringLiteral("管理"), m_managementBar);
    m_manageLabel->setObjectName("manageLabel");
    m_manageLabel->setFixedSize(84, 42);
    m_manageLabel->setAlignment(Qt::AlignCenter);
    manageLayout->addWidget(m_manageLabel);

    m_autoBackupBtn = new QPushButton(QStringLiteral("自动备份"), m_managementBar);
    m_autoBackupBtn->setObjectName("autoBackupBtn");
    m_autoBackupBtn->setFixedSize(138, 42);
    m_autoBackupBtn->setCursor(Qt::ArrowCursor);
    manageLayout->addWidget(m_autoBackupBtn);

    manageLayout->addStretch(1);

    mainLayout->addWidget(m_managementBar);

    // =============================================
    //  Table view (fills remaining space)
    // =============================================
    m_versionTable = new QTableView(this);
    setupTableView();
    mainLayout->addWidget(m_versionTable, 1);

    // Connect auto-backup button
    connect(m_autoBackupBtn, &QPushButton::clicked, this, [this]() {
        emit autoBackupToggled(true);
    });

    // Connect table context menu
    m_versionTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_versionTable, &QTableView::customContextMenuRequested,
            this, &VersionPanel::onTableContextMenu);
}

// ============================================================
//  Table View Setup
// ============================================================

void VersionPanel::setupTableView()
{
    m_tableModel = new QStandardItemModel(0, 4, this);
    m_tableModel->setHorizontalHeaderLabels({
        QStringLiteral("版本"),
        QStringLiteral("来源"),
        QStringLiteral("时间"),
        QStringLiteral("大小")
    });

    m_versionTable->setModel(m_tableModel);
    m_versionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_versionTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_versionTable->setShowGrid(false);
    m_versionTable->setAlternatingRowColors(false);
    m_versionTable->verticalHeader()->hide();
    m_versionTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_versionTable->setFocusPolicy(Qt::NoFocus);

    QHeaderView *hHeader = m_versionTable->horizontalHeader();
    hHeader->setStretchLastSection(true);
    hHeader->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    hHeader->setHighlightSections(false);
}

void VersionPanel::updateColumnWidths()
{
    int totalWidth = m_versionTable->viewport()->width();
    if (totalWidth <= 0) return; // not laid out yet, resizeEvent will fix
    double unit = totalWidth / 3.0;

    m_versionTable->horizontalHeader()->resizeSection(0, static_cast<int>(unit));
    m_versionTable->horizontalHeader()->resizeSection(1, static_cast<int>(unit));
    m_versionTable->horizontalHeader()->resizeSection(2, static_cast<int>(unit * 0.5));
    m_versionTable->horizontalHeader()->resizeSection(3, static_cast<int>(unit * 0.5));
}

void VersionPanel::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    updateColumnWidths();
}

void VersionPanel::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateColumnWidths();
}

// ============================================================
//  Per-project demo version data
// ============================================================

static QList<VersionInfo> versionsForProject(int projectIndex)
{
    switch (projectIndex) {
    case 0: // My Behavior Pack
        return {
            {"1.0",      "My Behavior Pack",  "2026-01-04 20:13:52", "12.4 MB", ""},
            {"1.1",      "My Behavior Pack",  "2026-02-15 14:30:00", "13.1 MB", ""},
            {"2.0-beta", "My Behavior Pack",  "2026-05-20 09:22:08", "15.8 MB", ""},
        };
    case 1: // Resource Overhaul
        return {
            {"0.5",      "Resource Overhaul", "2025-11-10 18:45:00", "8.2 MB",  ""},
            {"1.0",      "Resource Overhaul", "2026-01-20 12:00:00", "9.7 MB",  ""},
            {"1.2",      "Resource Overhaul", "2026-03-08 07:15:33", "10.3 MB", ""},
            {"2.0",      "Resource Overhaul", "2026-04-30 22:10:11", "11.9 MB", ""},
        };
    case 2: // Map Addon
        return {
            {"0.1-dev",  "Map Addon",         "2026-03-01 01:00:00", "5.1 MB",  ""},
            {"0.2-dev",  "Map Addon",         "2026-04-15 16:30:00", "6.3 MB",  ""},
        };
    case 3: // Skyblock World
        return {
            {"1.0",      "Skyblock World",    "2026-02-01 00:00:00", "25.0 MB", ""},
            {"1.5",      "Skyblock World",    "2026-02-28 11:00:00", "28.4 MB", ""},
            {"2.0",      "Skyblock World",    "2026-05-10 08:20:00", "32.1 MB", ""},
            {"2.1",      "Skyblock World",    "2026-06-01 19:45:30", "34.7 MB", ""},
            {"3.0-beta", "Skyblock World",    "2026-06-05 23:59:59", "36.2 MB", ""},
        };
    default:
        return {
            {"?.?", "Unknown", "----", "---", ""},
        };
    }
}

void VersionPanel::refreshTable()
{
    m_tableModel->removeRows(0, m_tableModel->rowCount());

    for (const auto &ver : m_versions) {
        QList<QStandardItem *> row;
        row.append(new QStandardItem(ver.version));
        row.append(new QStandardItem(ver.source));
        row.append(new QStandardItem(ver.timestamp));
        row.append(new QStandardItem(ver.size));
        m_tableModel->appendRow(row);
    }

    updateColumnWidths();
}

// ============================================================
//  Slots
// ============================================================

void VersionPanel::onProjectChanged(int projectIndex)
{
    m_currentProjectIndex = projectIndex;
    m_versions = versionsForProject(projectIndex);
    refreshTable();
}

void VersionPanel::setVersions(const QList<VersionInfo> &versions)
{
    m_versions = versions;
    refreshTable();
}

void VersionPanel::onTableContextMenu(const QPoint &pos)
{
    QModelIndex index = m_versionTable->indexAt(pos);
    if (!index.isValid()) return;

    int row = index.row();

    QMenu menu(this);
    QAction *restoreAction = menu.addAction(QStringLiteral("恢复"));
    QAction *deleteAction  = menu.addAction(QStringLiteral("删除"));
    menu.addSeparator();
    QAction *compareAction = menu.addAction(QStringLiteral("比较"));

    QPoint globalPos = m_versionTable->viewport()->mapToGlobal(pos);
    QAction *chosen = menu.exec(globalPos);
    if (!chosen) return;

    if (chosen == restoreAction) {
        emit versionRestoreRequested(row);
    } else if (chosen == deleteAction) {
        emit versionDeleteRequested(row);
    } else if (chosen == compareAction) {
        emit versionCompareRequested(row);
    }
}
