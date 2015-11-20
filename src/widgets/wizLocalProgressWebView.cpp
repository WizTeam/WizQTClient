#include "wizLocalProgressWebView.h"
#include <QMovie>
#include <QWebView>
#include <QLabel>
#include <QVBoxLayout>
#include <QPalette>

CWizLocalProgressWebView::CWizLocalProgressWebView(QWidget *parent) : QWidget(parent)
{
    setContentsMargins(0, 0, 0, 0);

    QPalette pal = palette();
    pal.setBrush(backgroundRole(), QBrush("#FFFFFF"));
    setPalette(pal);

    m_web = new QWebView(this);
    m_web->settings()->globalSettings()->setAttribute(QWebSettings::LocalStorageEnabled, true);
    m_web->settings()->globalSettings()->setAttribute(QWebSettings::LocalStorageDatabaseEnabled, true);

    m_movie = new QMovie(this);
    m_movie->setFileName(":/loading.gif");

    m_labelProgress = new QLabel(this);
    m_labelProgress->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_labelProgress->setAlignment(Qt::AlignCenter);
    m_labelProgress->setMovie(m_movie);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    layout->addWidget(m_labelProgress);
    layout->addWidget(m_web);
    m_web->setVisible(true);
    m_labelProgress->setVisible(false);
}

CWizLocalProgressWebView::~CWizLocalProgressWebView()
{

}

QWebView*CWizLocalProgressWebView::web()
{
    return m_web;
}

QMovie*CWizLocalProgressWebView::movie()
{
    return m_movie;
}

QLabel*CWizLocalProgressWebView::labelProgress()
{
    return m_labelProgress;
}

void CWizLocalProgressWebView::showLocalProgress()
{
    m_web->hide();
    m_labelProgress->show();
    m_movie->start();
}

void CWizLocalProgressWebView::hideLocalProgress()
{
    m_movie->stop();
    m_labelProgress->hide();
    m_web->show();
}

void CWizLocalProgressWebView::hideEvent(QHideEvent* ev)
{
    QWidget::hideEvent(ev);

    emit widgetStatusChanged();
}

void CWizLocalProgressWebView::showEvent(QShowEvent* ev)
{
    emit willShow();

    QWidget::showEvent(ev);

    emit widgetStatusChanged();
}

