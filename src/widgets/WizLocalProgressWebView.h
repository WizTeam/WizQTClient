#ifndef CWIZLOCALPROGRESSWEBVIEW_H
#define CWIZLOCALPROGRESSWEBVIEW_H

#include <QWidget>
#include "share/WizWebEngineView.h"

class WizWebEngineView;
class QMovie;
class QLabel;


class WizLocalProgressWebView : public QWidget
{
    Q_OBJECT
public:
    explicit WizLocalProgressWebView(const WizWebEngineViewInjectObjects& objects, QWidget *parent = 0);
    explicit WizLocalProgressWebView(QWidget *parent = 0);
    ~WizLocalProgressWebView();

    WizWebEngineView* web();

private:
    void init(const WizWebEngineViewInjectObjects& objects);

signals:
    void widgetStatusChanged();
    void willShow();

public slots:

protected:
    void hideEvent(QHideEvent* ev);
    void showEvent(QShowEvent* ev);

private:
    WizWebEngineView* m_web;
};

#endif // CWIZLOCALPROGRESSWEBVIEW_H
