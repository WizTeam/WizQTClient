#ifndef WIZUSERINFOWIDGETBASEMAC_H
#define WIZUSERINFOWIDGETBASEMAC_H


#ifdef USECOCOATOOLBAR

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

#if QT_VERSION >= 0x050200
    NSMenu* getNSMewnu();
#else
    QMenu* getMenu() { return m_menuPopup; }
#endif
    //
    virtual QString userId() { return QString(); }
    virtual QPixmap getAvatar(int width, int height) { return QPixmap(); }
    virtual QIcon getArrow() { return QIcon(); }
    virtual int textWidth() const;
    virtual int textHeight() const;
};

class CWizToolButtonWidget : public QMacCocoaViewContainer
{
    Q_OBJECT

public:
    CWizToolButtonWidget(QWidget *parent = 0);
    void setIcon(const QIcon& icon) { m_icon = icon; }
    QIcon getIcon() const { return m_icon; }
    void setText(const QString& text) { m_text = text; }
    QString getText() const { return m_text; }
    QPixmap getBackgroundImage(bool selected);

    void buttonClicked();

protected:
    virtual QSize sizeHint() const;

signals:
    void triggered(bool checked);

private:
    QString m_text;
    QSize m_size;
    QIcon m_icon;
    QPixmap m_backgroundNormal;
    QPixmap m_backgroundSelected;
};


#endif

#endif // WIZUSERINFOWIDGETBASEMAC_H
