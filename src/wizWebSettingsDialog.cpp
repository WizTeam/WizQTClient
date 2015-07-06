#include "wizWebSettingsDialog.h"
#include "sync/token.h"
#include "wizmainwindow.h"
#include "coreplugin/icore.h"
#include "utils/pathresolve.h"
#include "widgets/wizLocalProgressWebView.h"

#include <QWebView>
#include <QMovie>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWebFrame>
#include <QApplication>
#include <QNetworkReply>

using namespace WizService;

CWizWebSettingsDialog::CWizWebSettingsDialog(QString url, QSize sz, QWidget *parent)
    : QDialog(parent)
    , m_url(url)
{
    setContentsMargins(0, 0, 0, 0);
    setFixedSize(sz);

    QPalette pal = palette();
    pal.setBrush(backgroundRole(), QBrush("#FFFFFF"));
    setPalette(pal);

    CWizLocalProgressWebView* localProgressWebView = new CWizLocalProgressWebView(this);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(localProgressWebView);
    setLayout(layout);

    m_web = localProgressWebView->web();
//    connect(m_web->page()->networkAccessManager(), SIGNAL(finished(QNetworkReply*)),
//            SLOT(on_networkRequest_finished(QNetworkReply*)));
    connect(m_web, SIGNAL(loadFinished(bool)), SLOT(on_web_loaded(bool)));
    connect(m_web->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
            SLOT(onEditorPopulateJavaScriptWindowObject()));

    m_movie = localProgressWebView->movie();
    m_labelProgress = localProgressWebView->labelProgress();

    //
    m_labelProgress->setVisible(false);
    m_web->setVisible(true);
}

QWebView*CWizWebSettingsDialog::webVew()
{
    return m_web;
}

void CWizWebSettingsDialog::load()
{
    m_web->setVisible(false);
    m_labelProgress->setVisible(true);
    m_movie->start();
    m_web->page()->mainFrame()->load(m_url);
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
        m_movie->stop();
        m_labelProgress->setVisible(false);
        m_web->setVisible(true);
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
    m_web->setHtml(strHtml, url);
}

void CWizWebSettingsDialog::onEditorPopulateJavaScriptWindowObject()
{
    Core::Internal::MainWindow* mainWindow = qobject_cast<Core::Internal::MainWindow *>(Core::ICore::mainWindow());
    if (mainWindow) {
        m_web->page()->mainFrame()->addToJavaScriptWindowObject("WizExplorerApp", mainWindow->object());
    }
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
    m_movie->stop();
    m_labelProgress->setVisible(false);
    m_web->setVisible(true);
    loadErrorPage();
}

void CWizWebSettingsWithTokenDialog::load()
{
    m_web->setVisible(false);
    m_labelProgress->setVisible(true);

    m_movie->start();

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
    m_web->page()->mainFrame()->load(u);
}
