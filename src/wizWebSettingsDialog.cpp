#include "wizWebSettingsDialog.h"

#include <QWebView>
#include <QMovie>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

CWizWebSettingsDialog::CWizWebSettingsDialog(QSize sz, QWidget *parent)
    : QDialog(parent)
{
    setContentsMargins(0, 0, 0, 0);
    setFixedSize(sz);

    QPalette pal = palette();
    pal.setBrush(backgroundRole(), QBrush("#FFFFFF"));
    setPalette(pal);

    m_web = new QWebView(this);
    connect(m_web, SIGNAL(loadFinished(bool)), SLOT(on_web_loaded(bool)));

    QMovie* movie = new QMovie(this);
    movie->setFileName(":/loading.gif");
    movie->start();

    m_labelProgress = new QLabel(this);
    m_labelProgress->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_labelProgress->setAlignment(Qt::AlignCenter);
    m_labelProgress->setMovie(movie);

    m_labelError = new QLabel(tr("wow, seems unable to load what you want..."), this);
    m_labelError->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_labelError->setAlignment(Qt::AlignCenter);
    m_labelError->setVisible(false);

    m_btnOk = new QPushButton(tr("Close"), this);
    connect(m_btnOk, SIGNAL(clicked()), SLOT(accept()));

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(20, 0, 20, 20);
    layout->setSpacing(0);
    setLayout(layout);

    layout->addWidget(m_labelProgress);
    layout->addWidget(m_labelError);
    layout->addWidget(m_web);
    layout->addWidget(m_btnOk);
    layout->setAlignment(m_btnOk, Qt::AlignRight|Qt::AlignBottom);
}

void CWizWebSettingsDialog::showEvent(QShowEvent* event)
{
    Q_UNUSED(event);

    m_web->setVisible(false);

    Q_EMIT showProgress();
}

void CWizWebSettingsDialog::load(const QUrl& url)
{
    m_web->load(url);
}

void CWizWebSettingsDialog::on_web_loaded(bool ok)
{
    if (ok) {
        m_labelProgress->setVisible(false);
        m_web->setVisible(true);
    }
}

void CWizWebSettingsDialog::showError()
{
    m_labelProgress->setVisible(false);
    m_labelError->setVisible(true);
}
