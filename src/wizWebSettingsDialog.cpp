#include "wizWebSettingsDialog.h"
#include "sync/token.h"

#include <QWebView>
#include <QMovie>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

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

    m_web = new QWebView(this);
    connect(m_web, SIGNAL(loadFinished(bool)), SLOT(on_web_loaded(bool)));

    m_movie = new QMovie(this);
    m_movie->setFileName(":/loading.gif");

    m_labelProgress = new QLabel(this);
    m_labelProgress->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_labelProgress->setAlignment(Qt::AlignCenter);
    m_labelProgress->setMovie(m_movie);

    m_labelError = new QLabel(tr("wow, seems unable to load what you want..."), this);
    m_labelError->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_labelError->setAlignment(Qt::AlignCenter);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    layout->addWidget(m_labelProgress);
    layout->addWidget(m_labelError);
    layout->addWidget(m_web);
}

void CWizWebSettingsDialog::load()
{
    m_web->setVisible(false);
    m_labelError->setVisible(false);
    m_labelProgress->setVisible(true);

    m_movie->start();
    m_web->load(m_url);
}

void CWizWebSettingsDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    //
    load();
}


void CWizWebSettingsDialog::on_web_loaded(bool ok)
{
    if (ok) {
        m_movie->stop();
        m_labelProgress->setVisible(false);
        m_web->setVisible(true);
    }
}

void CWizWebSettingsDialog::showError()
{
    m_movie->stop();
    m_labelProgress->setVisible(false);
    m_labelError->setVisible(true);
}


void CWizWebSettingsWithTokenDialog::load()
{
    m_web->setVisible(false);
    m_labelError->setVisible(false);
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
    //
    m_web->load(u);
}
