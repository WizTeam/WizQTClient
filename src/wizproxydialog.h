#ifndef WIZPROXYDIALOG_H
#define WIZPROXYDIALOG_H

#include <QDialog>

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

};

#endif // WIZPROXYDIALOG_H
