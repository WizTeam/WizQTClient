#ifndef WIZBUTTON_H
#define WIZBUTTON_H

#include <QToolButton>
#include "wizdef.h"

class CWizButton : public QToolButton
{
    Q_OBJECT

public:
    explicit CWizButton(CWizExplorerApp& app, QWidget* parent = 0);
    void setAction(QAction* action);

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void leaveEvent(QEvent *event);
    virtual QSize sizeHint() const;

private:
    CWizExplorerApp& m_app;
    QIcon m_iconDefault;
    QIcon m_iconPressed;

};

#endif // WIZBUTTON_H
