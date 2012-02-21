#ifndef WIZMACHELPER_H
#define WIZMACHELPER_H

#include "wizqthelper.h"
#include <QtGui>

class CWizSearchBox : public QWidget
{
    Q_OBJECT
public:
    CWizSearchBox(QWidget* parent = 0);
    virtual QSize sizeHint() const;
private:
    QString m_keywords;
    QWidget* m_search;
public slots:
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
    //
    void adjustWidth(int width) { m_sz.setWidth(width); setFixedWidth(width); }

    QSize sizeHint() const { return m_sz; }
};

class CWizSplitter : public QSplitter
{
public:
    CWizSplitter(QWidget* parent = 0) : QSplitter(parent) {}
#ifdef Q_OS_MAC
    QSplitterHandle *createHandle();
#endif
};




class CWizSkin9GridImage
{
protected:
    QImage m_img;
    QRect m_arrayImageGrid[9];
    //
    BOOL Clear();
public:
    static BOOL SplitRect(const QRect& rcSrc, QPoint ptTopLeft, QRect* parrayRect, int nArrayCount);
    BOOL SetImage(const CString& strImageFileName, QPoint ptTopLeft);
    //
    void Draw(QPainter* p, QRect rc, int nAlpha) const;
    BOOL Valid() const;
};



#endif // WIZMACHELPER_H
