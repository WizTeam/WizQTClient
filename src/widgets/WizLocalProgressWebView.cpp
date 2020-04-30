#include "WizLocalProgressWebView.h"
#include <QMovie>
#include <QWebEngineView>
#include <QWebEngineSettings>
#include <QLabel>
#include <QVBoxLayout>
#include <QPalette>
#include "share/WizWebEngineView.h"

WizLocalProgressWebView::WizLocalProgressWebView(const WizWebEngineViewInjectObjects& objects, QWidget *parent)
    : QWidget(parent)
{
    init(objects);
}

WizLocalProgressWebView::WizLocalProgressWebView(QWidget *parent) : QWidget(parent)
{
    init(WizWebEngineViewInjectObjects());
}
//
void WizLocalProgressWebView::init(const WizWebEngineViewInjectObjects& objects)
{
    setContentsMargins(0, 0, 0, 0);

    m_web = new WizWebEngineView(this);
    WizWebEngineView::initWebEngineView(m_web, objects);
    //
    m_web->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    //
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    layout->addWidget(m_web);
}

WizLocalProgressWebView::~WizLocalProgressWebView()
{

}
WizWebEngineView* WizLocalProgressWebView::web()
{
    return m_web;
}


void WizLocalProgressWebView::hideEvent(QHideEvent* ev)
{
    QWidget::hideEvent(ev);

    emit widgetStatusChanged();
}

void WizLocalProgressWebView::showEvent(QShowEvent* ev)
{
    emit willShow();

    QWidget::showEvent(ev);

    emit widgetStatusChanged();
}

