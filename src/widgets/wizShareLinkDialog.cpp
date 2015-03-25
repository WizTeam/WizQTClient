#include "wizShareLinkDialog.h"
#include "sync/token.h"
#include "utils/pathresolve.h"
#include <QVBoxLayout>
#include <QWebFrame>
#include <QTimer>
#include <QDebug>

CWizShareLinkDialog::CWizShareLinkDialog(QWidget* parent, Qt::WindowFlags f)
    : QDialog(parent, f)
    , m_view(new QWebView(this))
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_view);
    QTimer::singleShot(100, this, SLOT(loadHtml()));

    connect(m_view->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
            SLOT(onJavaScriptWindowObject()));
}

CWizShareLinkDialog::~CWizShareLinkDialog()
{
}

void CWizShareLinkDialog::logAction(const QString& strAction)
{
    qDebug() << "[Share Link] " << strAction;
}

void CWizShareLinkDialog::writeToLog(const QString& strLog)
{
    qDebug() << "[Share Link] " << strLog;
}

void CWizShareLinkDialog::getToken(const QString& callback)
{
    qDebug() << "get token called ";
    QString strToken = WizService::Token::token();
    emit tokenObtained(strToken, callback);
}

void CWizShareLinkDialog::loadHtml()
{
    QString strFile = Utils::PathResolve::resourcesPath() + "files/share_link/index.html";
    QUrl url = QUrl::fromLocalFile(strFile);
    qDebug() << "url : " << url;
    m_view->load(url);
}

void CWizShareLinkDialog::onJavaScriptWindowObject()
{
    m_view->page()->mainFrame()->addToJavaScriptWindowObject("external", this);
}
