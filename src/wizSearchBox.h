#ifndef WIZSEARCHBOX_H
#define WIZSEARCHBOX_H

#include <QWidget>
#include <QLineEdit>

#include "wizdef.h"


// Qt native search widget, can be used on OSX, but it's ugly!
class CWizSearchWidget : public QWidget
{
    Q_OBJECT

public:
    CWizSearchWidget(CWizExplorerApp& app, QWidget* parent);
    virtual QSize sizeHint() const;

    void clear();
    void focus();

private:
    QLineEdit* m_editSearch;

public Q_SLOTS:
    void on_search_textChanged(const QString& strText);

Q_SIGNALS:
    void doSearch(const QString& keywords);
};


// Wrap widget for native widget and mac widget
// refer to wizSearchBox_mac.h for osx native search field
class CWizSearchBox : public QWidget
{
    Q_OBJECT

private:
    CWizSearchWidget* s;

public:
    explicit CWizSearchBox(CWizExplorerApp& app, QWidget *parent = 0);
    virtual QSize sizeHint() const;

    void clear() { s->clear(); }
    void focus() { s->focus(); }

Q_SIGNALS:
    void doSearch(const QString& keywords);
};


#endif // WIZSEARCHBOX_H
