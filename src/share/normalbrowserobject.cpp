#include "normalbrowserobject.h"
#include <QWebEngineView>
#include "WizThreads.h"
#include "sync/WizToken.h"

NormalBrowserObject::NormalBrowserObject(WizMainWindow* mainWindow, QWebEngineView* web, QObject* parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
    , m_web(web)
{

}

void NormalBrowserObject::GetToken(const QString& strFunctionName)
{
    QString functionName(strFunctionName);
    ::WizExecuteOnThread(WIZ_THREAD_NETWORK, [=] {
        //
        QString strToken = WizToken::token();
        if (strToken.isEmpty())
            return;
        //
        ::WizExecuteOnThread(WIZ_THREAD_MAIN, [=] {

            QString strExec = functionName + QString("('%1')").arg(strToken);
            qDebug() << "cpp get token callled : " << strExec;
            m_web->page()->runJavaScript(strExec);
        });
    });

}
