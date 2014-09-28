#include "wizFramelessWebDialog.h"
#include <QWebFrame>
#include <QWebView>
#include <QWebPage>
#include <QVBoxLayout>
#include <QDesktopServices>

CWizFramelessWebDialog::CWizFramelessWebDialog(QWidget *parent) :
    QDialog(parent)
{
    setFixedSize(800, 600);
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose);

    QWebView *view = new QWebView(this);
    m_frame = view->page()->mainFrame();
    connect(m_frame, SIGNAL(javaScriptWindowObjectCleared()),
            SLOT(onJavaScriptWindowObjectCleared()));
    view->setContextMenuPolicy(Qt::NoContextMenu);
    connect(m_frame, SIGNAL(loadFinished(bool)), SLOT(onPageLoadFinished(bool)));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(view);
}

void CWizFramelessWebDialog::loadAndShow(const QString& strUrl)
{
    m_url = strUrl;
    m_frame->load(QUrl(m_url));
}

void CWizFramelessWebDialog::timerEvent(QTimerEvent* /*event*/)
{
    deleteLater();
}

void CWizFramelessWebDialog::Execute(const QString& strFunction, QVariant param1,
                                              QVariant param2, QVariant param3, QVariant param4)
{
    if (strFunction == "close")
    {
        close();
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


void CWizFramelessWebDialog::onJavaScriptWindowObjectCleared()
{
    m_frame->addToJavaScriptWindowObject("customObject", this);
}

void CWizFramelessWebDialog::onPageLoadFinished(bool ok)
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
        exec();
    }
    else
    {
        // start timer to delete my self
        int nTimerID = startTimer(2 * 60 * 1000);
        m_timerIDList.append(nTimerID);
    }
}



