#ifndef WIZUPDATERDIALOG_H
#define WIZUPDATERDIALOG_H

#include <QDialog>
#include <QMouseEvent>
#include <QPoint>


namespace Ui {
class CWizUpdaterDialog;
}

class CWizUpdaterDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit CWizUpdaterDialog(QWidget *parent = 0);
    ~CWizUpdaterDialog();

    bool checkNeedUpdate();
    bool isPrepared();
    void prepare();
    void doUpdate();

    QString execPath() { return m_strExecPath; }

private:
    Ui::CWizUpdaterDialog *ui;

    bool m_bMovable;
    QPoint m_lastPos;

    QString m_strExecPath;

    bool m_isOldDatabase;
    bool m_bUpdateDatabase;
    bool m_bUpdateApp;

    // suppress all key press event to avoid quit
    virtual void keyPressEvent(QKeyEvent* event) { Q_UNUSED(event); }

    // use mouse button left to move window
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);

    bool detectIsOldDatabaseStruct();
    bool needUpdateDatabase();
    bool needUpdateApp();
    void doOldDatabaseUpgrade();
    void doDatabaseUpgrade();
    void doUpdateApp();
};

#endif // WIZUPDATERDIALOG_H
