#ifndef WIZPOPUPBUTTON_H
#define WIZPOPUPBUTTON_H

#include <QToolButton>
#include <QActionGroup>

class CWizExplorerApp;

class CWizPopupButton : public QToolButton
{
    Q_OBJECT

public:
    explicit CWizPopupButton(CWizExplorerApp &app, QWidget* parent = 0);
    void createAction(const QString& text, int type, QMenu* menu, QActionGroup* group);
    void setActionChecked(const QMenu* menu, int type);

protected:
    CWizExplorerApp& m_app;
    QIcon m_iconArraw;
    QSize m_iconSize;

    void paintEvent(QPaintEvent* event);

protected Q_SLOTS:
    virtual void on_action_triggered() = 0;
};


class CWizViewTypePopupButton: public CWizPopupButton
{
    Q_OBJECT

public:

    explicit CWizViewTypePopupButton(CWizExplorerApp &app, QWidget* parent = 0);
//    void setActionIcon(int type);

public slots:
    void on_viewTypeChanged(int type);

private:
//    QIcon m_iconOneLine;
//    QIcon m_iconTwoLine;
//    QIcon m_iconThumbnail;

protected:
    virtual QSize sizeHint() const;

protected Q_SLOTS:
    virtual void on_action_triggered();

Q_SIGNALS:
    void viewTypeChanged(int type);
};

class CWizSortingPopupButton : public CWizPopupButton
{
    Q_OBJECT

public:       

    explicit CWizSortingPopupButton(CWizExplorerApp& app, QWidget *parent = 0);

public slots:
    void on_sortingTypeChanged(int type);

protected:
    virtual QSize sizeHint() const;

protected Q_SLOTS:
    virtual void on_action_triggered();

Q_SIGNALS:
    void sortingTypeChanged(int type);

private:
    bool m_isAscendingOrder;
};

#endif // WIZPOPUPBUTTON_H
