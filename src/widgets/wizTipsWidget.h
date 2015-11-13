#ifndef CWIZTIPSWIDGET_H
#define CWIZTIPSWIDGET_H

#include <functional>
#include <QList>
#include <QTimer>
#include "share/wizpopupwidget.h"

class QLabel;
class QPushButton;

class CWizTipsWidget : public CWizPopupWidget
{
    Q_OBJECT
public:
    explicit CWizTipsWidget(const QString id,QWidget *parent = 0);

    void setSizeHint(const QSize& hintSize);
    virtual QSize sizeHint() const;

    QString id();

    void setText(const QString& title, const QString& info, const QString& buttonText = tr("OK"));
    void setButtonVisible(bool visible);

    void addToTipListManager(QWidget* targetWidget, int nXOff = 0, int nYOff = 0);

    void bindFunction(std::function<void(void)> const& f);
signals:

public slots:

protected:
    void mouseReleaseEvent(QMouseEvent* ev);
    void showEvent(QShowEvent* ev);

private:
    QLabel* m_labelTitle;
    QLabel* m_labelInfo;
    QPushButton* m_btnOK;
    QSize m_hintSize;
    QString m_id;
    std::function<void(void)> m_function;
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
    void displayCurrentTipWidget();

private slots:
    void on_timerOut();

private:
    explicit CWizTipListManager(QObject* parent = 0);

    void deleteManager();

    struct TipItem
    {
        CWizTipsWidget* widget;
        QWidget* targetWidget;
        int nXOff;
        int nYOff;
    };

    QList<TipItem> m_tips;
    QTimer m_timer;

    static CWizTipListManager* m_instance;
};

#endif // CWIZTIPSWIDGET_H
