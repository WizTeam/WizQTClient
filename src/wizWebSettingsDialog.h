#ifndef WIZWEBSETTINGSDIALOG_H
#define WIZWEBSETTINGSDIALOG_H

#include <QDialog>

class QUrl;
class QLabel;
class QMovie;
class QWebView;
class QPushButton;


class CWizWebSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CWizWebSettingsDialog(QSize sz, QWidget *parent = 0);
    void load(const QUrl& url);
    void showError();

protected:
    virtual void showEvent(QShowEvent* event);

private:
    QLabel* m_labelProgress;
    QMovie* m_movie;
    QLabel* m_labelError;
    QWebView* m_web;
    QPushButton* m_btnOk;

private Q_SLOTS:
    void on_web_loaded(bool ok);

Q_SIGNALS:
    void showProgress();
    void loaded(bool ok);
};

#endif // WIZWEBSETTINGSDIALOG_H
