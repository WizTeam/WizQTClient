#ifndef WIZSCROLLBAR_H
#define WIZSCROLLBAR_H

#include <QScrollBar>
#include <QPointer>
#include <QTimer>


class CWizScrollBar : public QScrollBar
{
    Q_OBJECT

public:
    explicit CWizScrollBar(QWidget* parent = 0);
    void syncWith(QScrollBar* source);
    void show();

    virtual QSize sizeHint() const;
    virtual void mouseMoveEvent(QMouseEvent* event);

public Q_SLOTS:
    void on_sourceValueChanged(int value);
    void on_sourceRangeChanged(int min, int max);
    void on_valueChanged(int value);
    void on_scrollTimeout();

private:
    QPointer<QScrollBar> m_scrollSyncSource;
    QTimer m_timerScrollTimeout;
};


#endif // WIZSCROLLBAR_H
