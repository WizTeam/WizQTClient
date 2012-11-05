#ifndef WIZSTATUSBAR_H
#define WIZSTATUSBAR_H

#include <QtGui>

#include "wizdef.h"

class CWizStatusBar : public QStatusBar
{
    Q_OBJECT

public:
    explicit CWizStatusBar(CWizExplorerApp& app, QWidget *parent = 0);

    void setText(const QString& text);

//protected:
//    virtual void paintEvent(QPaintEvent* event);

private:
    CWizExplorerApp& m_app;

    QLabel m_label;
};

#endif // WIZSTATUSBAR_H
