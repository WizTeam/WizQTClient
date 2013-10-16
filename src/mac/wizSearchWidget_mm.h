#ifndef WIZSEARCHWIDGET_MM_H
#define WIZSEARCHWIDGET_MM_H

//#if 0

// Qt5 remove QMacCocoaViewContainter from mainstream, but still left headers on
// include direcotry, it's a bug!!! critical bug!!! ugly bug!!!
//#include <QMacCocoaViewContainer>
#include "qmaccocoaviewcontainer.h"

class CWizExplorerApp;

class CWizSearchWidget : public QMacCocoaViewContainer
{
    Q_OBJECT

public:
    CWizSearchWidget(CWizExplorerApp& app, QWidget* parent = 0);
    void clear();
    void focus();

    virtual QSize sizeHint() const;

public Q_SLOTS:
    void on_search_textChanged(const QString& strText);

Q_SIGNALS:
    void doSearch(const QString& keywords);
};

//#endif

#endif // WIZSEARCHWIDGET_MM_H
