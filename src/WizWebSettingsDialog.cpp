#include "WizWebSettingsDialog.h"
#include "sync/WizToken.h"
#include "WizMainWindow.h"
#include "share/WizGlobal.h"
#include "utils/WizPathResolve.h"
#include "widgets/WizLocalProgressWebView.h"

#include <QMovie>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QApplication>
#include <QNetworkReply>
#include <QWebEngineView>
#include "share/WizWebEngineView.h"

WizWebSettingsDialog::WizWebSettingsDialog(const WizWebEngineViewInjectObjects& objects, QString url, QSize sz, QWidget *parent )
    : WizWebEngineViewContainerDialog(parent)
    , m_url(url)
{
    init(objects, sz, parent);
}


WizWebSettingsDialog::WizWebSettingsDialog(QString url, QSize sz, QWidget *parent)
    : WizWebEngineViewContainerDialog(parent)
    , m_url(url)
{
    init(WizWebEngineViewInjectObjects(), sz, parent);
}
//
void WizWebSettingsDialog::init(const WizWebEngineViewInjectObjects& objects, QSize sz, QWidget *parent)
{
    setContentsMargins(0, 0, 0, 0);
    if (!sz.isEmpty()) {
        setFixedSize(sz);
    } else {
        setWindowFlags(Qt::Window);
        setWindowState(windowState() | Qt::WindowMaximized);
        setGeometry(parent->geometry());
    }

    QPalette pal = palette();
    pal.setBrush(backgroundRole(), QBrush("#FFFFFF"));
    setPalette(pal);

    m_progressWebView = new WizLocalProgressWebView(objects, this);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSizeConstraint(QLayout::SetMaximumSize);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_progressWebView);   
    setLayout(layout);

    WizWebEngineView* web = m_progressWebView->web();
    //
    WizMainWindow* mainWindow = WizGlobal::mainWindow();
    if (mainWindow) {
        WizWebEngineViewInjectObjects temp = objects;
        temp.push_back({"WizExplorerApp", mainWindow->object()});
        //
        WizWebEngineView::initWebEngineView(web, temp);
    }
    connect(web, SIGNAL(loadFinishedEx(bool)), SLOT(on_web_loaded(bool)));
    //
    QTimer::singleShot(100, [=] {
        load();
    });
}

WizWebEngineView* WizWebSettingsDialog::web()
{
    return m_progressWebView->web();
}

void WizWebSettingsDialog::load()
{
    web()->load(m_url);
}

void WizWebSettingsDialog::on_web_loaded(bool ok)
{
    if (ok)
    {
        onLoaded(true);
    }
    else
    {
        onLoaded(false);
        //失败的时候会造成死循环
        //loadErrorPage();
    }
}

void WizWebSettingsDialog::loadErrorPage()
{
    QString strFileName = Utils::WizPathResolve::resourcesPath() + "files/errorpage/load_fail.html";
    QString strHtml;
    ::WizLoadUnicodeTextFromFile(strFileName, strHtml);    
    QUrl url = QUrl::fromLocalFile(strFileName);
    web()->setHtml(strHtml, url);
}

void WizWebSettingsDialog::on_networkRequest_finished(QNetworkReply* reply)
{
    // 即使在连接正常情况下也会出现OperationCanceledError，此处将其忽略
    if (reply && reply->error() != QNetworkReply::NoError && reply->error() != QNetworkReply::OperationCanceledError)
    {
        showError();
    }
}

void WizWebSettingsDialog::showError()
{
    //loadErrorPage();
}

void WizWebSettingsWithTokenDialog::load()
{
    if (!m_loaded) {
        connect(WizToken::instance(), SIGNAL(tokenAcquired(const QString&)),
                SLOT(on_token_acquired(const QString&)), Qt::QueuedConnection);

        WizToken::requestToken();
    }
}

void WizWebSettingsWithTokenDialog::on_token_acquired(const QString& token)
{
    if (token.isEmpty()) {
        if (!m_delayShow) {
            showError();
        }
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


void WizWebSettingsWithTokenDialog::onLoaded(bool ok)
{
    if (m_loaded) {
        return;
    }
    //
    m_loaded = true;
    //
    if (m_delayShow && ok) {
        show();
    }
}

////

WizWebSettingsWithTokenDialog* WizWebSettingsWithTokenDialog::delayShow(QString title, QString url, QSize sz, QWidget* parent)
{
    WizWebSettingsWithTokenDialog* dialog = new WizWebSettingsWithTokenDialog(url, sz, parent);
    //
    dialog->setWindowTitle(title);
    dialog->m_delayShow = true;
    //
    return dialog;
}
