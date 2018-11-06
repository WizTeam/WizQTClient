#ifndef WIZSEARCHWIDGET_H
#define WIZSEARCHWIDGET_H

#include <QtGlobal>
#include <QWidget>
#include <QIcon>
#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>

// Wrap widget for qt widget and mac native search field
// refer to mac/wizSearchWidget.mm for osx NSSearchField

#ifdef USECOCOATOOLBAR
class WizSearchView;

#else

class WizExplorerApp;

class WizSearchEdit : public QLineEdit
{
    Q_OBJECT
public:
    WizSearchEdit(QWidget* parent = 0);

public slots:
    void on_actionAdvancedSearch();
    void on_addCustomSearch();

signals:
    void advanceSearchRequest();
    void addCustomSearchRequest();

protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

private:
    QIcon m_searchIcon;
    QIcon m_deleteIcon;
    QMenu* m_menu;
};

class WizSearchView : public QWidget
{
    Q_OBJECT

public:
    WizSearchView(QWidget* parent = 0);
    void clear();
    void focus();

    virtual QSize sizeHint() const;
    //
    void setCurrentKb(const QString& kbGuid) { m_strCurrentKbGuid = kbGuid; }
    QString currentKb() const { return m_strCurrentKbGuid; }


public Q_SLOTS:
    void on_search_returnPressed();
    void on_searchTextChanged(QString str);
    void setWidthHint(int nWidth);

private:
    WizSearchEdit* m_editSearch;
    int m_widthHint;
    QString m_strCurrentKbGuid;

Q_SIGNALS:
    void addCustomSearchRequest();
    void doSearch(const QString& keywords);
};

#endif // Q_OS_MAC



#endif // WIZSEARCHWIDGET_H
