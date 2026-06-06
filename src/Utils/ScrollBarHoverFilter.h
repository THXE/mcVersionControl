#ifndef SCROLLBARHOVERFILTER_H
#define SCROLLBARHOVERFILTER_H

#include <QObject>

class QAbstractScrollArea;

/// Event filter that shows the vertical scrollbar only when the mouse
/// hovers over the scroll area viewport, replicating the CSS
/// `::-webkit-scrollbar-thumb` hover behavior.
class ScrollBarHoverFilter : public QObject
{
    Q_OBJECT

public:
    explicit ScrollBarHoverFilter(QAbstractScrollArea *scrollArea,
                                  QObject *parent = nullptr);
    ~ScrollBarHoverFilter() override = default;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QAbstractScrollArea *m_scrollArea = nullptr;
};

#endif // SCROLLBARHOVERFILTER_H
