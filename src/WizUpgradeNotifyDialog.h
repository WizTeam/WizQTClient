#ifndef WIZPGRADENOTIFYDIALOG_H
#define WIZPGRADENOTIFYDIALOG_H

#include <QDialog>

namespace Ui {
class WizUpgradeNotifyDialog;
}

class WizUpgradeNotifyDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit WizUpgradeNotifyDialog(const QString& changeUrl, QWidget *parent = 0);
    ~WizUpgradeNotifyDialog();
    
private:
    Ui::WizUpgradeNotifyDialog *ui;
};

#endif // WIZPGRADENOTIFYDIALOG_H
