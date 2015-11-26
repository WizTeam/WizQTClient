#ifndef WIZMACHELPER_H
#define WIZMACHELPER_H

#include <QtGui>
#include <QSplitter>

//#include "wizdef.h"

QBrush WizGetLeftViewBrush();

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

protected:
    virtual QSplitterHandle* createHandle();    
};

#endif // WIZMACHELPER_H
