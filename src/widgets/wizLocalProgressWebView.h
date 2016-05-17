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
    QMovie* movie();
    QLabel* labelProgress();

    void showLocalProgress();
    void hideLocalProgress();

signals:
    void widgetStatusChanged();
    void willShow();

public slots:

protected:
    void hideEvent(QHideEvent* ev);
    void showEvent(QShowEvent* ev);

private:
    WizWebEngineView* m_web;
    QMovie* m_movie;
    QLabel* m_labelProgress;
};

#endif // CWIZLOCALPROGRESSWEBVIEW_H
