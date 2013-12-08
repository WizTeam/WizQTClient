#ifndef WIZBUTTON_H
#define WIZBUTTON_H

#include <QToolButton>
#include <QIcon>

class CWizExplorerApp;

class CWizButton : public QToolButton
{
    Q_OBJECT

public:
    explicit CWizButton(QWidget* parent);
    void setAction(QAction* action);

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual QSize sizeHint() const;
};

#endif // WIZBUTTON_H
