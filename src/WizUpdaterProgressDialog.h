#ifndef WIZUPDATERPROGRESSDIALOG_H
#define WIZUPDATERPROGRESSDIALOG_H

#include <QDialog>

class QMouseEvent;

namespace Ui {
class WizUpdaterDialog;
}

class WizUpdaterDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit WizUpdaterDialog(QWidget *parent = 0);
    void center();
    ~WizUpdaterDialog();

    bool checkNeedUpdate();
    bool isPrepared();
    void prepare();
    void doUpdate();

private:
    Ui::WizUpdaterDialog *ui;

    bool m_bMovable;
    QPoint m_lastPos;

    bool m_isOldDatabase;
    bool m_bUpdateDatabase;
    bool m_bUpdateApp;

    // suppress all key press event to avoid quit
    virtual void keyPressEvent(QKeyEvent* event) { Q_UNUSED(event); }

    // use mouse button left to move window
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);

    void setGuiNotify(const QString& text);

    bool needUpdateDatabase();
    bool detectIsOldDatabaseStruct();
    void doOldDatabaseUpgradeSetSteps();
    void doOldDatabaseUpgrade();
    void doDatabaseUpgrade();

    bool needUpdateApp();
    void doUpdateAppSetSteps();
    void doUpdateApp();
};

#endif // WIZUPDATERPROGRESSDIALOG_H
