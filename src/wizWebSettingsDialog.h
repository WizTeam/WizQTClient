#ifndef WIZWEBSETTINGSDIALOG_H
#define WIZWEBSETTINGSDIALOG_H

#include <QDialog>

class QUrl;
class QWebView;
class QLabel;
class QPushButton;

class CWizWebSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CWizWebSettingsDialog(QSize sz, QWidget *parent = 0);
    void load(const QUrl& url);

private:
    QLabel* m_labelProgress;
    QWebView* m_web;
    QPushButton* m_btnOk;

private Q_SLOTS:
    void on_web_loaded(bool ok);

Q_SIGNALS:
    void loaded(bool ok);
};

#endif // WIZWEBSETTINGSDIALOG_H
