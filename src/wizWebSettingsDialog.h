#ifndef WIZWEBSETTINGSDIALOG_H
#define WIZWEBSETTINGSDIALOG_H

#include <QDialog>

class QUrl;
class QLabel;
class QMovie;
class WizWebEngineView;
class QPushButton;
class QNetworkReply;
class WizLocalProgressWebView;

class WizWebSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WizWebSettingsDialog(QString url, QSize sz, QWidget *parent = 0);
    WizWebEngineView* web();

    void showError();
protected:
    virtual void load();
    virtual void showEvent(QShowEvent* event);
protected:
    QString m_url;
    WizLocalProgressWebView* m_progressWebView;

private Q_SLOTS:
    void on_web_loaded(bool ok);
    void loadErrorPage();
    void on_networkRequest_finished(QNetworkReply* reply);
};

#define WIZ_TOKEN_IN_URL_REPLACE_PART   "wiz_web_settings_with_token_replace_part"

class WizWebSettingsWithTokenDialog : public WizWebSettingsDialog
{
    Q_OBJECT
public:
    explicit WizWebSettingsWithTokenDialog(QString url, QSize sz, QWidget *parent = 0)
        : WizWebSettingsDialog(url, sz, parent) {}
protected:
    virtual void load();
private Q_SLOTS:
    void on_token_acquired(const QString& token);
};



#endif // WIZWEBSETTINGSDIALOG_H
