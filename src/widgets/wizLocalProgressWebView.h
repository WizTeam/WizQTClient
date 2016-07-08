#ifndef CWIZLOCALPROGRESSWEBVIEW_H
#define CWIZLOCALPROGRESSWEBVIEW_H

#include <QWidget>

class WizWebEngineView;
class QMovie;
class QLabel;

class CWizLocalProgressWebView : public QWidget
{
    Q_OBJECT
public:
    explicit CWizLocalProgressWebView(QWidget *parent = 0);
    ~CWizLocalProgressWebView();

    WizWebEngineView* web();

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
