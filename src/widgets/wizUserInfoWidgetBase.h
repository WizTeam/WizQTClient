#ifndef WIZUSERINFOWIDGETBASE_H
#define WIZUSERINFOWIDGETBASE_H

#ifndef Q_OS_MAC

#include <QToolButton>

class QMenu;


class CWizUserInfoWidgetBase : public QToolButton
{
    Q_OBJECT

public:
    explicit CWizUserInfoWidgetBase(QWidget *parent = 0);

protected:
    QIcon m_iconVipIndicator;
    QIcon m_iconArraw;
protected:
    virtual void paintEvent(QPaintEvent *event);
    //
    virtual QPixmap getAvatar();

protected:
    virtual void mousePressEvent(QMouseEvent* event);
    virtual bool hitButton(const QPoint& pos) const;
};

#endif //Q_OS_MAC

#endif // WIZUSERINFOWIDGETBASE_H
