#ifndef CWIZTIPSWIDGET_H
#define CWIZTIPSWIDGET_H

#include <QList>
#include "share/wizpopupwidget.h"

class QLabel;
class QPushButton;

class CWizTipsWidget : public CWizPopupWidget
{
    Q_OBJECT
public:
    explicit CWizTipsWidget(QWidget *parent = 0);

    void setSizeHint(const QSize& hintSize);
    virtual QSize sizeHint() const;

    void setText(const QString& title, const QString& info, const QString& buttonText = tr("OK"));
    void setButtonVisible(bool visible);

    void addToTipListManager(QWidget* targetWidget, int nXOff = 0, int nYOff = 0);
signals:

public slots:

protected:
    void mouseReleaseEvent(QMouseEvent* ev);

private:
    QLabel* m_labelTitle;
    QLabel* m_labelInfo;
    QPushButton* m_btnOK;
    QSize m_hintSize;
};

class CWizTipListManager : public QObject
{
    Q_OBJECT
public:
    static CWizTipListManager* instance();

    void addTipsWidget(CWizTipsWidget* widget, QWidget* targetWidget, int nXOff = 0, int nYOff = 0);

    CWizTipsWidget* firstTipWidget();
public slots:
    void displayNextTipWidget();

private:
    explicit CWizTipListManager(QObject* parent = 0);

    struct TipItem
    {
        CWizTipsWidget* widget;
        QWidget* targetWidget;
        int nXOff;
        int nYOff;
    };

    QList<TipItem> m_tips;

    static CWizTipListManager* m_instance;
};

#endif // CWIZTIPSWIDGET_H
