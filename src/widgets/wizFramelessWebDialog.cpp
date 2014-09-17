#include "wizFramelessWebDialog.h"
#include <QWebFrame>
#include <QWebView>
#include <QWebPage>
#include <QVBoxLayout>
#include <QDesktopServices>

CWizFramelessWebDialog::CWizFramelessWebDialog(const QString& strUrl, QWidget *parent) :
    QDialog(parent)
{
    setFixedSize(800, 600);
    setWindowFlags(Qt::FramelessWindowHint);

    QWebView *view = new QWebView(this);
    m_frame = view->page()->mainFrame();
    m_frame->load(QUrl(strUrl));
    connect(m_frame, SIGNAL(javaScriptWindowObjectCleared()),
            SLOT(onJavaScriptWindowObjectCleared()));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(view);
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



