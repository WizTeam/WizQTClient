#ifndef WIZMACHELPER_H
#define WIZMACHELPER_H

#include <QtGui>
#include <QSplitter>

#include "wizdef.h"


class CWizSpacer : public QWidget
{
public:
    CWizSpacer(QWidget* parent = 0);
    QSize sizeHint() const { return QSize(1, 1); }
};

class CWizVerSpacer : public QWidget
{
public:
    CWizVerSpacer(QWidget* parent = 0);
    QSize sizeHint() const { return QSize(1, 1); }
};

class CWizFixedSpacer : public QWidget
{
    QSize m_sz;
public:
    CWizFixedSpacer(QSize sz, QWidget* parent = 0)
        : QWidget(parent)
        , m_sz(sz)
    {
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        setSizePolicy(sizePolicy);
    }

    void adjustWidth(int width) { m_sz.setWidth(width); setFixedWidth(width); }

    QSize sizeHint() const { return m_sz; }
};

class CWizSplitter : public QSplitter
{
public:
    CWizSplitter(QWidget* parent = 0);
    virtual QSplitterHandle *createHandle();
};

class CWizScrollBar : public QScrollBar
{
    Q_OBJECT

public:
    CWizScrollBar(QWidget* parent = 0);
    void syncWith(QScrollBar* source);

    virtual QSize sizeHint() const;
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void paintEvent(QPaintEvent* event);

public Q_SLOTS:
    void on_sourceValueChanged(int value);
    void on_sourceRangeChanged(int min, int max);
    void on_valueChanged(int value);
    void on_scrollTimeout();

private:
    QPointer<QScrollBar> m_scrollSyncSource;
    QTimer m_timerScrollTimeout;
};


#endif // WIZMACHELPER_H
