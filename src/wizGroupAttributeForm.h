#ifndef WIZGROUPATTRIBUTEFORM_H
#define WIZGROUPATTRIBUTEFORM_H

#include <QFrame>
#include <QPointer>
#include <QWebView>
#include <QUrl>

#include "share/wizapi.h"
#include "wizdef.h"

class CWizGroupAttributeForm : public QFrame
{
    Q_OBJECT
public:
    explicit CWizGroupAttributeForm(CWizExplorerApp& app, QWidget* parent = 0);

    void sheetShow(const QString& strKbGUID);
    void sheetHide();

protected:
    CWizExplorerApp& m_app;
    CWizApiBase m_api;
    QPointer<QWebView> m_web;
    QString m_strKbGUID;

    virtual void showEvent(QShowEvent* event);

public Q_SLOTS:
    void onAnimationSheetHideStateChanged(QAbstractAnimation::State newState,
                                          QAbstractAnimation::State oldState);
    void on_btnClose_clicked();
    void on_clientLoginDone();
    
};

#endif // WIZGROUPATTRIBUTEFORM_H
