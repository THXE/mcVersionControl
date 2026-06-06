#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QPoint>

class TitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit TitleBar(QWidget *parent = nullptr);
    ~TitleBar() override = default;

    /// Returns true if point is in the drag zone (not on a button)
    bool isDragZone(const QPoint &pos) const;

    /// Get the file button pointer for menu positioning
    QPushButton *fileButton() const { return m_fileBtn; }

    /// Update fullscreen button to reflect window state
    void setFullscreen(bool fullscreen);

signals:
    void minimizeClicked();
    void maximizeRestoreClicked();
    void closeClicked();
    void helpClicked();
    void contactClicked();
    void navigateBack();
    void navigateForward();

    // File menu actions
    void addProjectClicked();
    void newProjectClicked();
    void refreshClicked();
    void settingsClicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private slots:
    void showFileMenu();

private:
    void setupUi();

    QLabel      *m_logoLabel    = nullptr;
    QPushButton *m_fileBtn      = nullptr;
    QPushButton *m_helpBtn      = nullptr;
    QPushButton *m_contactBtn   = nullptr;
    QPushButton *m_backBtn      = nullptr;
    QPushButton *m_forwardBtn   = nullptr;
    QPushButton *m_minBtn       = nullptr;
    QPushButton *m_maxBtn       = nullptr;
    QPushButton *m_closeBtn     = nullptr;

    bool    m_dragging  = false;
    QPoint  m_dragStartPos;
};

#endif // TITLEBAR_H
