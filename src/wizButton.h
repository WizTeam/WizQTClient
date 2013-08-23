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
    virtual QSize sizeHint() const;

private:
    CWizExplorerApp& m_app;
};

#endif // WIZBUTTON_H
