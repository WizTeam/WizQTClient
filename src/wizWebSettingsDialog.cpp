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

    QMovie* movie = new QMovie(":/loading.gif");
    movie->start();

    m_labelProgress = new QLabel(this);
    m_labelProgress->setMovie(movie);

    m_btnOk = new QPushButton(tr("Close"), this);
    connect(m_btnOk, SIGNAL(clicked()), SLOT(accept()));

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(20, 0, 20, 20);
    layout->setSpacing(0);
    setLayout(layout);

    layout->addWidget(m_labelProgress);
    layout->addWidget(m_web);
    layout->addWidget(m_btnOk);
    layout->setAlignment(m_btnOk, Qt::AlignRight|Qt::AlignVCenter);
}

void CWizWebSettingsDialog::load(const QUrl& url)
{
    m_web->load(url);

    m_web->hide();
    m_btnOk->hide();

    m_labelProgress->show();
    open();
}

void CWizWebSettingsDialog::on_web_loaded(bool ok)
{
    if (ok) {
        m_labelProgress->hide();
        m_web->show();
        m_btnOk->show();
    }
}
