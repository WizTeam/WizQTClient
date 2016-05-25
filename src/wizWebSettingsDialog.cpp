#include "wizWebSettingsDialog.h"
#include "sync/token.h"
#include "wizmainwindow.h"
#include "share/wizGlobal.h"
#include "utils/pathresolve.h"
#include "widgets/wizLocalProgressWebView.h"

#include <QMovie>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QApplication>
#include <QNetworkReply>
#include <QWebEngineView>
#include "share/wizwebengineview.h"


CWizWebSettingsDialog::CWizWebSettingsDialog(QString url, QSize sz, QWidget *parent)
    : QDialog(parent)
    , m_url(url)
{
    setContentsMargins(0, 0, 0, 0);
    setFixedSize(sz);

    QPalette pal = palette();
    pal.setBrush(backgroundRole(), QBrush("#FFFFFF"));
    setPalette(pal);

    m_progressWebView = new CWizLocalProgressWebView(this);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_progressWebView);
    setLayout(layout);

    WizWebEngineView* web = m_progressWebView->web();
    //
    MainWindow* mainWindow = WizGlobal::mainWindow();
    if (mainWindow) {
        web->addToJavaScriptWindowObject("WizExplorerApp", mainWindow->object());
    }
    connect(web, SIGNAL(loadFinishedEx(bool)), SLOT(on_web_loaded(bool)));
    //
    m_progressWebView->showLocalProgress();
}

WizWebEngineView* CWizWebSettingsDialog::web()
{
    return m_progressWebView->web();
}

void CWizWebSettingsDialog::load()
{
    m_progressWebView->showLocalProgress();
    web()->load(m_url);
}

void CWizWebSettingsDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    //
    load();
}

void CWizWebSettingsDialog::on_web_loaded(bool ok)
{
    if (ok)
    {
        m_progressWebView->hideLocalProgress();
    }
    else
    {
        loadErrorPage();
    }
}

void CWizWebSettingsDialog::loadErrorPage()
{
    QString strFileName = Utils::PathResolve::resourcesPath() + "files/errorpage/load_fail.html";
    QString strHtml;
    ::WizLoadUnicodeTextFromFile(strFileName, strHtml);    
    QUrl url = QUrl::fromLocalFile(strFileName);
    web()->setHtml(strHtml, url);
}

void CWizWebSettingsDialog::on_networkRequest_finished(QNetworkReply* reply)
{
    // 即使在连接正常情况下也会出现OperationCanceledError，此处将其忽略
    if (reply && reply->error() != QNetworkReply::NoError && reply->error() != QNetworkReply::OperationCanceledError)
    {
        showError();
    }
}

void CWizWebSettingsDialog::showError()
{
    m_progressWebView->hideLocalProgress();
    loadErrorPage();
}

void CWizWebSettingsWithTokenDialog::load()
{
    m_progressWebView->showLocalProgress();

    connect(Token::instance(), SIGNAL(tokenAcquired(const QString&)),
            SLOT(on_token_acquired(const QString&)), Qt::QueuedConnection);

    Token::requestToken();
}

void CWizWebSettingsWithTokenDialog::on_token_acquired(const QString& token)
{
    if (token.isEmpty()) {
        showError();
        return;
    }
    //
    QString url = m_url;
    url.replace(QString(WIZ_TOKEN_IN_URL_REPLACE_PART), token);
    //
    QUrl u = QUrl::fromEncoded(url.toUtf8());
    qDebug() << " show web dialog with token : " << u;

    //
    web()->load(u);
}
