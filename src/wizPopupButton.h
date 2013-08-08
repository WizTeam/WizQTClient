#ifndef WIZPOPUPBUTTON_H
#define WIZPOPUPBUTTON_H

#include <QtGui>

class CWizExplorerApp;

class CWizPopupButton : public QToolButton
{
    Q_OBJECT

public:
    explicit CWizPopupButton(CWizExplorerApp &app, QWidget* parent = 0);

protected:
    QIcon m_iconArraw;

    void paintEvent(QPaintEvent* event);
};


class CWizViewTypePopupButton: public CWizPopupButton
{
    Q_OBJECT

    enum {
        TypeOneLine,
        TypeTwoLine,
        TypeThumbnail
    };

public:
    explicit CWizViewTypePopupButton(CWizExplorerApp &app, QWidget* parent = 0);
    virtual QSize sizeHint() const;
};

class CWizSortingPopupButton : public CWizPopupButton
{
    Q_OBJECT

    enum SortingType {
        SortingCreateTime,
        SortingUpdateTime,
        SortingTitle,
        SortingLocation,
        SortingTag,
        SortingSize
    };

public:
    explicit CWizSortingPopupButton(CWizExplorerApp& app, QWidget *parent = 0);
    virtual QSize sizeHint() const;

private:
    void createAction(const QString& text, SortingType type, QMenu* menu, QActionGroup* group);

private Q_SLOTS:
    void on_sortingTypeChanged();
};

#endif // WIZPOPUPBUTTON_H
