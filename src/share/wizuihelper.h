#ifndef WIZMACHELPER_H
#define WIZMACHELPER_H

#include <QtGui>

#include "wizdef.h"

class CWizSearchBox : public QWidget
{
    Q_OBJECT

public:
    CWizSearchBox(CWizExplorerApp& app, QWidget* parent = 0);
    virtual QSize sizeHint() const;

private:
    CWizExplorerApp& m_app;
    QString m_keywords;
    QWidget* m_search;

public Q_SLOTS:
    void on_search_editingFinished();
    void on_search_edited(const QString& str);

Q_SIGNALS:
    void doSearch(const QString& keywords);
};

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
    QSplitterHandle *createHandle();

#ifndef Q_OS_MAC
private:
    int m_splitterWidth;
    QColor m_splitterColor;
public:
    void setSplitterWidth(int width);
    int splitterWidth() const { return m_splitterWidth; }
    void setSplitterColor(const QColor& color);
    QColor splitterColor() const { return m_splitterColor; }
#endif

};


#endif // WIZMACHELPER_H
