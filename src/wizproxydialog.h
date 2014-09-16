#ifndef WIZPROXYDIALOG_H
#define WIZPROXYDIALOG_H

#include <QDialog>
#include <QNetworkProxy>

namespace Ui {
    class ProxyDialog;
}

class ProxyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProxyDialog(QWidget *parent = 0);
    ~ProxyDialog();

public Q_SLOTS:
    virtual void accept();

    void proxyStatusChanged(int state);


private:
    Ui::ProxyDialog *ui;

    void enableControl(bool b);
    void setApplicationProxy();
    QNetworkProxy::ProxyType getProxyType();
};

#endif // WIZPROXYDIALOG_H
