#ifndef WIZSTATUSBAR_H
#define WIZSTATUSBAR_H

#include <QtGui>

#include "wizdef.h"

class CWizStatusBar : public QLabel
{
    Q_OBJECT

public:
    explicit CWizStatusBar(CWizExplorerApp& app, QWidget *parent = 0);

protected:
    virtual void paintEvent(QPaintEvent* event);

private:
    CWizExplorerApp& m_app;

};

#endif // WIZSTATUSBAR_H
