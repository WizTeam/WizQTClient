#ifndef CWIZSINGLEDOCUMENTVIEW_H
#define CWIZSINGLEDOCUMENTVIEW_H

#include <QWidget>
#include <QMap>
#include "WizDef.h"

struct WIZDOCUMENTDATA;
class WizDocumentWebEngine;

class WizDocumentView;

class WizSingleDocumentViewer : public QWidget
{
    Q_OBJECT
public:
    WizSingleDocumentViewer(WizExplorerApp& app, const QString& guid, QWidget* parent = 0);
    ~WizSingleDocumentViewer();

    WizDocumentView* docView();
    QString guid() const { return m_guid; }   

public slots:
    void on_commentWidget_statusChanged();
    void on_commentWidget_willShow();

signals:
    void documentViewerDeleted(QString guid);

protected:
    void resizeEvent(QResizeEvent* ev);
    void closeEvent(QCloseEvent *ev);
    bool event(QEvent *ev);
    virtual void keyPressEvent(QKeyEvent* ev);

private:
    void applyWidgetBackground(bool isFullScreen);

private:
    WizDocumentView* m_docView;
    QString m_guid;

    QWidget* m_containerWgt;
};



class WizSingleDocumentViewDelegate : public QObject
{
    Q_OBJECT
public:
    WizSingleDocumentViewDelegate(WizExplorerApp& app, QObject* parent = 0);

    WizSingleDocumentViewer* getDocumentViewer(const QString& guid);
    QMap<QString, WizSingleDocumentViewer*>& getDocumentViewerMap();

public slots:
    void viewDocument(const WIZDOCUMENTDATA& doc);
    void onDocumentViewerDeleted(QString guid);

signals:
    void documentChanged(const QString& strGUID, WizDocumentView* viewer);
    void documentViewerClosed(QString guid);

private:

private:
    QMap<QString, WizSingleDocumentViewer*> m_viewerMap;
    WizExplorerApp& m_app;
};

void bindESCToQuitFullScreen(QWidget* wgt);
void bringWidgetToFront(QWidget* wgt);

#endif // CWIZSINGLEDOCUMENTVIEW_H
