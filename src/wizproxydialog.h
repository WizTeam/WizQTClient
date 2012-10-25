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

private:
    Ui::ProxyDialog *ui;
};

#endif // WIZPROXYDIALOG_H
