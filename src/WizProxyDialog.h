#ifndef WIZPROXYDIALOG_H
#define WIZPROXYDIALOG_H

#include <QDialog>
#include <QNetworkProxy>

namespace Ui {
    class WizProxyDialog;
}

class WizProxyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WizProxyDialog(QWidget *parent = 0);
    ~WizProxyDialog();

public Q_SLOTS:
    virtual void accept();

    void proxyStatusChanged(int state);


private:
    Ui::WizProxyDialog *ui;

    void enableControl(bool b);
    void setApplicationProxy();
    QNetworkProxy::ProxyType getProxyType();
};

#endif // WIZPROXYDIALOG_H
