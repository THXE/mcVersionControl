#include "ScrollBarHoverFilter.h"

#include <QAbstractScrollArea>
#include <QEvent>

ScrollBarHoverFilter::ScrollBarHoverFilter(QAbstractScrollArea *scrollArea,
                                           QObject *parent)
    : QObject(parent), m_scrollArea(scrollArea)
{
    if (m_scrollArea) {
        // Initially hide the scrollbar
        m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        // Install on the viewport
        if (m_scrollArea->viewport()) {
            m_scrollArea->viewport()->installEventFilter(this);
        }
    }
}

bool ScrollBarHoverFilter::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj)

    if (!m_scrollArea) return false;

    switch (event->type()) {
    case QEvent::Enter:
        m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        break;
    case QEvent::Leave:
        m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        break;
    default:
        break;
    }

    return false; // let the event continue propagating
}
