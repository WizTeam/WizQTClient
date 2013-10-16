#ifndef WIZPGRADENOTIFYDIALOG_H
#define WIZPGRADENOTIFYDIALOG_H

#include <QDialog>

namespace Ui {
class CWizUpgradeNotifyDialog;
}

class CWizUpgradeNotifyDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit CWizUpgradeNotifyDialog(const QString& changeUrl, QWidget *parent = 0);
    ~CWizUpgradeNotifyDialog();
    
private:
    Ui::CWizUpgradeNotifyDialog *ui;
};

#endif // WIZPGRADENOTIFYDIALOG_H
