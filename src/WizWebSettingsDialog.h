#ifndef WIZWEBSETTINGSDIALOG_H
#define WIZWEBSETTINGSDIALOG_H

#include <QDialog>
#include "share/WizWebEngineView.h"

class QUrl;
class QLabel;
class QMovie;
class WizWebEngineView;
class QPushButton;
class QNetworkReply;
class WizLocalProgressWebView;

class WizWebSettingsDialog : public WizWebEngineViewContainerDialog
{
    Q_OBJECT

public:
    explicit WizWebSettingsDialog(QString url, QSize sz, QWidget *parent = 0);
    explicit WizWebSettingsDialog(const WizWebEngineViewInjectObjects& objects, QString url, QSize sz, QWidget *parent = 0);

    WizWebEngineView* web();

    void showError();
private:
    void init(const WizWebEngineViewInjectObjects& objects, QSize sz, QWidget *parent);
protected:
    virtual void load();
    virtual void onLoaded(bool ok) {}
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
        : WizWebSettingsDialog(url, sz, parent)
        , m_delayShow(false)
        , m_loaded(false)
    {

    }
    //    
    static WizWebSettingsWithTokenDialog* delayShow(QString title, QString url, QSize sz, QWidget* parent = 0);
protected:
    bool m_delayShow;
    bool m_loaded;
protected:
    virtual void load();
    virtual void onLoaded(bool ok);
private Q_SLOTS:
    void on_token_acquired(const QString& token);
};



#endif // WIZWEBSETTINGSDIALOG_H
