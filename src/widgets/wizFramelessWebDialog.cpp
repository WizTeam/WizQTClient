#include "wizFramelessWebDialog.h"
#include <QWebFrame>
#include <QWebView>
#include <QWebPage>
#include <QVBoxLayout>

CWizFramelessWebDialog::CWizFramelessWebDialog(const QString& strUrl, QWidget *parent) :
    QDialog(parent)
{
    setFixedSize(800, 600);
    setWindowFlags(Qt::FramelessWindowHint);

    QWebView *view = new QWebView(this);
    view->load(QUrl(strUrl));
    m_frame = view->page()->mainFrame();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(view);
}

void CWizFramelessWebDialog::OnExecuteCommand(const QString& strFunction, QVariant param1,
                                              QVariant param2, QVariant param3, QVariant param4, QVariant* pvRet)
{
    if (strFunction == "close")
    {
        close();
    }
}


void CWizFramelessWebDialog::onJavaScriptWindowObjectCleared()
{
    m_frame->addToJavaScriptWindowObject("custumDialog", this);
}



