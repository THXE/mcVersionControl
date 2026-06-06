#include "ProjectCard.h"
#include "Utils/ThemeManager.h"

#include <QVBoxLayout>
#include <QPainter>
#include <QStyle>
#include <QMouseEvent>
#include <QContextMenuEvent>

ProjectCard::ProjectCard(const ProjectInfo &info, QWidget *parent)
    : QWidget(parent), m_info(info)
{
    setFixedHeight(72);
    setCursor(Qt::PointingHandCursor);
    setMouseTracking(true);
    setupUi();
    setActive(false);
}

void ProjectCard::setupUi()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 6, 10, 6);
    layout->setSpacing(2);

    // Project name — styled via QSS #projectCardName
    m_nameLabel = new QLabel(m_info.name, this);
    m_nameLabel->setObjectName("projectCardName");
    layout->addWidget(m_nameLabel);

    // Author — styled via QSS #projectCardAuthor
    m_authorLabel = new QLabel(m_info.author, this);
    m_authorLabel->setObjectName("projectCardAuthor");
    layout->addWidget(m_authorLabel);

    // Path — styled via QSS #projectCardId
    m_idLabel = new QLabel(m_info.path, this);
    m_idLabel->setObjectName("projectCardId");
    layout->addWidget(m_idLabel);

    setLayout(layout);
}

void ProjectCard::setActive(bool active)
{
    if (m_active != active) {
        m_active = active;
        setProperty("active", active);
        style()->unpolish(this);
        style()->polish(this);
        update();
    }
}

void ProjectCard::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect r = rect();

    QColor bgColor = m_active ? ThemeColors::bgActive()
                   : m_hovered ? ThemeColors::bgHover()
                   : ThemeColors::bgMain();
    painter.setBrush(bgColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(r, 6, 6);

    if (m_active) {
        QRect accentRect = r.adjusted(0, 8, -(r.width() - 2), -8);
        painter.setBrush(ThemeColors::accent());
        painter.drawRoundedRect(accentRect, 1, 1);
    }

    painter.setPen(QPen(ThemeColors::border(), 1));
    painter.drawLine(r.bottomLeft() + QPoint(8, 0),
                     r.bottomRight() - QPoint(8, 0));
}

void ProjectCard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) emit clicked();
    QWidget::mousePressEvent(event);
}

void ProjectCard::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    emit doubleClicked();
}

void ProjectCard::contextMenuEvent(QContextMenuEvent *event)
{
    emit contextMenuRequested(event->globalPos());
}

void ProjectCard::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event)
    m_hovered = true;
    update();
}

void ProjectCard::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    m_hovered = false;
    update();
}
