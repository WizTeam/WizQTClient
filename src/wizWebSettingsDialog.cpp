#include "wizWebSettingsDialog.h"

#include <QtGui>
#include <QtWebKit>

CWizWebSettingsDialog::CWizWebSettingsDialog(QSize sz, QWidget *parent)
    : QDialog(parent)
{
    setFixedSize(sz);
    setModal(true);

    m_web = new QWebView(this);
    connect(m_web, SIGNAL(loadFinished(bool)), SLOT(on_web_loaded(bool)));

    QMovie* movie = new QMovie(":/loading.gif");
    movie->start();

    m_labelProgress = new QLabel(this);
    m_labelProgress->setMovie(movie);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    layout->addWidget(m_labelProgress);
    layout->addWidget(m_web);
}

void CWizWebSettingsDialog::load(const QUrl& url)
{
    m_web->load(url);

    m_web->hide();
    m_labelProgress->show();
    open();
}

void CWizWebSettingsDialog::on_web_loaded(bool ok)
{
    if (ok) {
        m_labelProgress->hide();
        m_web->show();
    }
}
