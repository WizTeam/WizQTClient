#ifndef WIZSEARCHWIDGET_MM_H
#define WIZSEARCHWIDGET_MM_H

#include <QMacCocoaViewContainer>

class CWizSearchWidget : public QMacCocoaViewContainer
{
    Q_OBJECT

public:
    CWizSearchWidget(QWidget* parent = 0);
    void clear();
    void focus();

    virtual QSize sizeHint() const;

public Q_SLOTS:
    void on_search_textChanged(const QString& strText);

Q_SIGNALS:
    void doSearch(const QString& keywords);
};

#endif // WIZSEARCHWIDGET_MM_H
