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
    explicit CWizWebSettingsDialog(QString url, QSize sz, QWidget *parent = 0);

public:
    void showError();
protected:
    virtual void load();
    virtual void showEvent(QShowEvent* event);
protected:
    QString m_url;
    QLabel* m_labelProgress;
    QMovie* m_movie;
    QLabel* m_labelError;
    QWebView* m_web;

private Q_SLOTS:
    void on_web_loaded(bool ok);
};


#define WIZ_TOKEN_IN_URL_REPLACE_PART   "wiz_web_settings_with_token_replace_part"

class CWizWebSettingsWithTokenDialog : public CWizWebSettingsDialog
{
    Q_OBJECT
public:
    explicit CWizWebSettingsWithTokenDialog(QString url, QSize sz, QWidget *parent = 0)
        : CWizWebSettingsDialog(url, sz, parent) {}
protected:
    virtual void load();
private Q_SLOTS:
    void on_token_acquired(const QString& token);
};



#endif // WIZWEBSETTINGSDIALOG_H
