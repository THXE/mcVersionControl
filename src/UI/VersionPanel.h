#ifndef VERSIONPANEL_H
#define VERSIONPANEL_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QStandardItemModel>
#include <QList>
#include "Core/VersionManager.h"

class VersionPanel : public QWidget
{
    Q_OBJECT

public:
    explicit VersionPanel(QWidget *parent = nullptr);
    ~VersionPanel() override;

public slots:
    void onProjectChanged(int projectIndex);
    void setVersions(const QList<VersionInfo> &versions);
    void refreshTable();

signals:
    void versionRestoreRequested(int versionIndex);
    void versionDeleteRequested(int versionIndex);
    void versionCompareRequested(int versionIndex);
    void autoBackupToggled(bool enabled);

protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onTableContextMenu(const QPoint &pos);

private:
    void setupUi();
    void setupTableView();
    void updateColumnWidths();

    // Management bar
    QWidget     *m_managementBar  = nullptr;
    QLabel      *m_manageLabel    = nullptr;
    QPushButton *m_autoBackupBtn  = nullptr;

    // Table
    QTableView         *m_versionTable = nullptr;
    QStandardItemModel *m_tableModel   = nullptr;

    int m_currentProjectIndex = -1;
    QList<VersionInfo> m_versions;
};

#endif // VERSIONPANEL_H
