#ifndef WIZUSERINFOWIDGET_H
#define WIZUSERINFOWIDGET_H

#ifdef USECOCOATOOLBAR
#include "mac/wizUserInfoWidgetBaseMac_mm.h"
#define WIZUSERINFOWIDGETBASE CWizUserInfoWidgetBaseMac
#else
#include "widgets/wizUserInfoWidgetBase.h"
#define WIZUSERINFOWIDGETBASE CWizUserInfoWidgetBase
#endif

class CWizExplorerApp;
class CWizDatabase;
class QMenu;
class CWizWebSettingsDialog;
class CWizIAPHelper;

class CWizUserInfoWidget : public WIZUSERINFOWIDGETBASE
{
    Q_OBJECT

public:
    explicit CWizUserInfoWidget(CWizExplorerApp& app, QWidget *parent = 0);

protected:
    CWizExplorerApp& m_app;
    CWizDatabase& m_db;
    QMenu* m_menuMain;
    QIcon m_iconVipIndicator;
    QIcon m_iconArraw;
    QPixmap m_circleAvatar;
    //
    QPixmap getCircleAvatar(int width, int height);

    virtual QPixmap getAvatar(int width, int height);
    virtual QIcon getVipIcon();
    virtual QIcon getArrow() { return m_iconArraw; }
    virtual QSize sizeHint() const;
    virtual QString userId();
    virtual void updateUI();

protected Q_SLOTS:
    void resetUserInfo();

    void on_userInfo_changed();

    void on_userAvatar_loaded(const QString& strGUID);
    void on_action_accountInfo_triggered();
    void on_action_accountSettings_triggered();
    void on_action_upgradeVip_triggered();

    void on_action_changeAvatar_triggered();
    void on_action_changeAvatar_uploaded(bool ok);

    void on_action_viewNotesOnWeb_triggered();
    void on_action_mySharedNotes_triggered();

    void on_action_logout_triggered();
};


#endif // WIZUSERINFOWIDGET_H
