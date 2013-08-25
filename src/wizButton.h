#ifndef WIZBUTTON_H
#define WIZBUTTON_H

#include <QToolButton>
#include <QIcon>

class CWizExplorerApp;

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

class CWizUtilButton : public QToolButton
{
    Q_OBJECT

public:
    enum Position {
        Left,
        Center,
        Right
    };

    explicit CWizUtilButton(Position pos, CWizExplorerApp& app, QWidget* parent = 0);

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual QSize sizeHint() const;

private:
    QIcon m_backgroundIcon;
    Position m_pos;
};

#endif // WIZBUTTON_H
