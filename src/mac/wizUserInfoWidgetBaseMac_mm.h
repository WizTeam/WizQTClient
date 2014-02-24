#ifndef WIZUSERINFOWIDGETBASEMAC_H
#define WIZUSERINFOWIDGETBASEMAC_H


#include <QMacCocoaViewContainer>
#include <QIcon>

class QMenu;
#ifdef Q_OS_OSX
Q_FORWARD_DECLARE_OBJC_CLASS(NSMenu);
#endif

class CWizUserInfoWidgetBaseMac : public QMacCocoaViewContainer
{
    Q_OBJECT

public:
    explicit CWizUserInfoWidgetBaseMac(QWidget *parent = 0);

protected:
    QString m_text;
    QMenu* m_menuPopup;
    //
    int m_textWidth;
    int m_textHeight;
    //
    void setMenu(QMenu* menu) { m_menuPopup = menu; }
    //
    void calTextSize();
    //
    virtual void updateUI();
public:

    QString text() const;
    void setText(QString val);

    NSMenu* getNSMewnu();
    //
    virtual QString userId() { return QString(); }
    virtual QPixmap getAvatar(int width, int height) { return QPixmap(); }
    virtual QIcon getArrow() { return QIcon(); }
    virtual int textWidth() const;
    virtual int textHeight() const;
};



#endif // WIZUSERINFOWIDGETBASEMAC_H
