#ifndef WIZFRAMELESSWEBDIALOG_H
#define WIZFRAMELESSWEBDIALOG_H

#include <QDialog>
#include <QString>
#include <QList>
#include "share/WizWebEngineView.h"

class WizWebEngineView;
class QWebEnginePage;

class WizFramelessWebDialog : public WizWebEngineViewContainerDialog
{
    Q_OBJECT
public:
    explicit WizFramelessWebDialog(QWidget *parent = 0);

    void loadAndShow(const QString& strUrl);

signals:
    void doNotShowThisAgain(bool bAgain);

public slots:
    void Execute(const QString& strFunction, QVariant param1, QVariant param2,
                                  QVariant param3, QVariant param4);
    void onPageLoadFinished(bool ok);


private:
    WizWebEngineView* m_web;
    QWebEnginePage *m_frame;
    QString m_url;
    QList<int> m_timerIDList;
    //
    static bool m_bVisibling;
};

#endif // WIZFRAMELESSWEBDIALOG_H
