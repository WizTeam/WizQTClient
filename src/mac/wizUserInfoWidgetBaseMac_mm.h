#ifndef WIZUSERINFOWIDGETBASEMAC_H
#define WIZUSERINFOWIDGETBASEMAC_H


#include <QMacCocoaViewContainer>
#include <QIcon>

class QMenu;

class CWizUserInfoWidgetBaseMac : public QMacCocoaViewContainer
{
    Q_OBJECT

public:
    explicit CWizUserInfoWidgetBaseMac(QWidget *parent = 0);

protected:
    QString m_text;
    QIcon m_iconVipIndicator;
    QIcon m_iconArraw;
    QMenu* m_menuPopup;

    void setMenu(QMenu* menu) { m_menuPopup = menu; }
    //
    QString text() const;
    void setText(QString val);
    //
    virtual QPixmap getAvatar() { return QPixmap(); }
};



#endif // WIZUSERINFOWIDGETBASEMAC_H
