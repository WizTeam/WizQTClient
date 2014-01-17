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
    //
    int m_textWidth;
    int m_textHeight;

    void setMenu(QMenu* menu) { m_menuPopup = menu; }
    //
    void calTextSize();
public:

    QString text() const;
    void setText(QString val);
    //
    void showMenu();
    //
    virtual QPixmap getAvatar() { return QPixmap(); }
    virtual QIcon getArrow() { return m_iconArraw; }
    virtual int textWidth() const;
    virtual int textHeight() const;
};



#endif // WIZUSERINFOWIDGETBASEMAC_H
