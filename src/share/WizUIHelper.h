#ifndef WIZUIHELPER_H
#define WIZUIHELPER_H

#include <QtGui>
#include <QSplitter>

#include "WizUIBase.h"

QBrush WizGetLeftViewBrush();

class WizSpacer : public QWidget
{
public:
    WizSpacer(QWidget* parent = 0);
    QSize sizeHint() const { return QSize(1, 1); }
};

class WizVerSpacer : public QWidget
{
public:
    WizVerSpacer(QWidget* parent = 0);
    QSize sizeHint() const { return QSize(1, 1); }
};

class WizFixedSpacer : public QWidget
{
    QSize m_sz;
public:
    WizFixedSpacer(QSize sz, QWidget* parent = 0)
        : QWidget(parent)
        , m_sz(sz)
    {
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        setSizePolicy(sizePolicy);
    }

    void adjustWidth(int width) { m_sz.setWidth(width); setFixedWidth(width); }

    QSize sizeHint() const { return m_sz; }
};

class WizSplitter : public QSplitter
{
public:
    WizSplitter(QWidget* parent = 0);

protected:
    virtual QSplitterHandle* createHandle();    
};


#endif // WIZUIHELPER_H
