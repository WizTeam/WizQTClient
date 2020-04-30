#include "WizFramelessWebDialog.h"
#include <QVBoxLayout>
#include <QDesktopServices>
#include <QWebEnginePage>
#include "../share/WizWebEngineView.h"
#include "../share/WizThreads.h"
#include <QTimer>

bool WizFramelessWebDialog::m_bVisibling = false;

WizFramelessWebDialog::WizFramelessWebDialog(QWidget *parent) :
    WizWebEngineViewContainerDialog(parent)
{
    setFixedSize(800, 600);
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose);

    m_web = new WizWebEngineView(this);
    WizWebEngineView::initWebEngineView(m_web, {{"customObject", this}});
    //
    m_frame = m_web->page();
    connect(m_web, SIGNAL(loadFinishedEx(bool)), SLOT(onPageLoadFinished(bool)));
    //
    m_web->setContextMenuPolicy(Qt::NoContextMenu);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_web);
}

void WizFramelessWebDialog::loadAndShow(const QString& strUrl)
{
    m_url = strUrl;
    m_frame->load(QUrl(m_url));
}

void WizFramelessWebDialog::Execute(const QString& strFunction, QVariant param1,
                                              QVariant param2, QVariant param3, QVariant param4)
{
    if (strFunction == "close")
    {
        ::WizExecuteOnThread(WIZ_THREAD_MAIN, [=]{
            hide();
            //
            QTimer::singleShot(1000, Qt::PreciseTimer, [=]{
                deleteLater();
            });
        });
    }
    else if (strFunction == "openindefaultbrowser")
    {
        QString strUrl = param1.toString();
        QDesktopServices::openUrl(strUrl);
    }
    else if (strFunction == "setdonotprompt")
    {
        emit doNotShowThisAgain(param1.toBool());
    }
}


void WizFramelessWebDialog::onPageLoadFinished(bool ok)
{
    if (ok)
    {
        while (!m_timerIDList.isEmpty())
        {
            int nTimerID = m_timerIDList.first();
            killTimer(nTimerID);
            m_timerIDList.removeFirst();
        }
        //avoid QDialog::exec: Recursive call
        disconnect(m_frame, SIGNAL(loadFinished(bool)), this, SLOT(onPageLoadFinished(bool)));
        //
        WizExecuteOnThread(WIZ_THREAD_MAIN, [=]{
            //
            if (m_bVisibling)
                return;
            //
            m_bVisibling = true;
            show();
            m_bVisibling = false;
            //
        });
    }
    else
    {
        QTimer::singleShot(2 * 60 * 1000, Qt::PreciseTimer, [=]{
            deleteLater();
        });
    }
}



