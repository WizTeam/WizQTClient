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

    void setAutoAdjustPosition(bool autoAdjust);
    bool isAutoAdjustPosition() const;

    void setText(const QString& title, const QString& info, const QString& buttonText = tr("OK"));
    void setButtonVisible(bool visible);

    bool addToTipListManager(QWidget* targetWidget, int nXOff = 0, int nYOff = 0);

    void bindShowFunction(std::function<void(void)> const& f);
    void bindHideFunction(std::function<void(void)> const& f);
    void bindCloseFunction(std::function<void(void)> const& f);

    void hide();
signals:

public slots:
    void onTargetWidgetClicked();

protected:
    void mouseReleaseEvent(QMouseEvent* ev);
    void showEvent(QShowEvent* ev);


private:
    void closeTip();

private:
    QLabel* m_labelTitle;
    QLabel* m_labelInfo;
    QPushButton* m_btnOK;
    QSize m_hintSize;
    QString m_id;
    bool m_autoAdjustPosition;
    std::function<void(void)> m_showFunction;       //tip显示的时候执行的方法
    std::function<void(void)> m_hideFunction;       //用户没有点击tip，但是其他操作导致tip不应该被显示的时候执行的操作
    std::function<void(void)> m_closeFunction;      //用户点击了tip时候执行的方法
};

class CWizTipListManager : public QObject
{
    Q_OBJECT
public:
    static CWizTipListManager* instance();
    static void cleanOnQuit();

    void addTipsWidget(CWizTipsWidget* widget, QWidget* targetWidget, int nXOff = 0, int nYOff = 0);

    CWizTipsWidget* firstTipWidget();

    bool tipsWidgetExists(const QString id);


public slots:
    void displayNextTipWidget();
    void displayCurrentTipWidget();    

private slots:
    void on_timerOut();

private:
    explicit CWizTipListManager(QObject* parent = 0);
    ~CWizTipListManager();

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
