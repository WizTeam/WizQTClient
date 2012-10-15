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
    
private:
    Ui::CWizUpdaterDialog *ui;

    bool m_bMovable;
    QPoint m_lastPos;

    // suppress all key press event to avoid quit
    virtual void keyPressEvent(QKeyEvent* event) { Q_UNUSED(event); }

    // use mouse button left to move window
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
};

#endif // WIZUPDATERDIALOG_H
