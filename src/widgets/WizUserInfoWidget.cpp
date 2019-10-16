#include <QtGlobal>
#include "WizUserInfoWidget.h"
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>

#include "WizDef.h"
#include "share/WizSettings.h"
#include "share/WizDatabaseManager.h"
#include "share/WizThreads.h"
#include "sync/WizApiEntry.h"
#include "sync/WizKMServer.h"
#include "sync/WizAvatarUploader.h"
#include "core/WizAccountManager.h"
#include "sync/WizAvatarHost.h"
#include "sync/WizToken.h"
#include "widgets/WizIAPDialog.h"
#include "WizWebSettingsDialog.h"
#include "WizMainWindow.h"
#include "WizOEMSettings.h"


WizUserInfoWidget::WizUserInfoWidget(WizExplorerApp& app, QWidget *parent)
    : WIZUSERINFOWIDGETBASE(parent)
    , m_app(app)
    , m_db(app.databaseManager().db())
{
    connect(WizAvatarHost::instance(), SIGNAL(loaded(const QString&)),
            SLOT(on_userAvatar_loaded(const QString&)));

    WizAvatarHost::load(m_db.getUserId(), false);

    resetUserInfo();

    connect(&m_db, SIGNAL(userInfoChanged()), SLOT(on_userInfo_changed()));

    // load builtin arraw
    QString strIconPath = ::WizGetSkinResourcePath(app.userSettings().skin()) + "arrow.png";
    m_iconArraw.addFile(strIconPath);

    // setup menu
    m_menuMain = new QMenu(NULL);

    QAction* actionAccountInfo = new QAction(tr("View account info..."), m_menuMain);
    connect(actionAccountInfo, SIGNAL(triggered()), SLOT(on_action_accountInfo_triggered()));
    actionAccountInfo->setVisible(false);

    QAction* actionAccountSetup = new QAction(tr("Account settings..."), m_menuMain);
    connect(actionAccountSetup, SIGNAL(triggered()), SLOT(on_action_accountSettings_triggered()));


    QAction* actionChangeAvatar = new QAction(tr("Change avatar..."), m_menuMain);
    connect(actionChangeAvatar, SIGNAL(triggered()), SLOT(on_action_changeAvatar_triggered()));

    QAction* actionWebService = new QAction(tr("Open web client..."), m_menuMain);
    connect(actionWebService, SIGNAL(triggered()), SLOT(on_action_viewNotesOnWeb_triggered()));

    QAction* actionLogout = new QAction(tr("Logout..."), m_menuMain);
    connect(actionLogout, SIGNAL(triggered()), SLOT(on_action_logout_triggered()));

    m_menuMain->addAction(actionAccountInfo);
    m_menuMain->addAction(actionAccountSetup);
    m_menuMain->addAction(actionChangeAvatar);
    WizOEMSettings oemSettings(m_db.getAccountPath());
    if (!oemSettings.isHideBuyVip() && app.userSettings().serverType() != EnterpriseServer)
    {
        WizAccountManager manager(m_app.databaseManager());
        QAction* actionUpgradeVIP = new QAction(manager.isVip() ? tr("Renew Vip...") : tr("Upgrade VIP..."), m_menuMain);
        connect(actionUpgradeVIP, SIGNAL(triggered()), SLOT(on_action_upgradeVip_triggered()));
        m_menuMain->addAction(actionUpgradeVIP);
    }
    m_menuMain->addSeparator();
    m_menuMain->addAction(actionWebService);
//    if (!oemSettings.isHideMyShare())
//    {
//        QAction* actionMyShare = new QAction(tr("My shared links..."), m_menuMain);
//        connect(actionMyShare, SIGNAL(triggered()), SLOT(on_action_mySharedNotes_triggered()));
//        m_menuMain->addAction(actionMyShare);
//    }
    m_menuMain->addSeparator();
    m_menuMain->addAction(actionLogout);
    //
#ifndef Q_OS_MAC
    if (isDarkMode()) {
        m_menuMain->setStyleSheet("background-color:#272727");
    }
#endif
    //
    setMenu(m_menuMain);
}

void WizUserInfoWidget::showAccountSettings()
{
    on_action_accountSettings_triggered();
}

void WizUserInfoWidget::resetUserInfo()
{
    WIZUSERINFO info;
    if (!m_db.getUserInfo(info))
        return;

    if (info.strDisplayName.isEmpty()) {
        setText(::WizGetEmailPrefix(m_db.getUserId()));
    } else {
        QString strName = info.strDisplayName;
        //QString strName = fontMetrics().elidedText(info.strDisplayName, Qt::ElideRight, 150);
        setText(strName);
    }
    //
    //m_textWidth = fontMetrics().width(text());

    QString iconName;
    if (info.strUserType == "vip") {
        iconName = "vip1.png";
    } else if (info.strUserType == "vip2") {
        iconName = "vip2.png";
    } else if (info.strUserType == "vip3") {
        iconName = "vip3.png";
    } else {
        iconName = "vip0.png";
    }

    QString strIconPath = ::WizGetSkinResourcePath(m_app.userSettings().skin()) + iconName;
    m_iconVipIndicator.addFile(strIconPath);
    //
    updateUI();
}

void WizUserInfoWidget::on_userAvatar_loaded(const QString& strGUID)
{
    if (strGUID != m_db.getUserId())
        return;

    updateUI();
}

void WizUserInfoWidget::on_action_accountInfo_triggered()
{

}

void WizUserInfoWidget::on_action_accountSettings_triggered()
{    
    WizMainWindow* window = dynamic_cast<WizMainWindow*>(m_app.mainWindow());
#ifndef BUILD4APPSTORE
    QString extInfo = WizCommonApiEntry::appstoreParam(false);
    QString strUrl = WizCommonApiEntry::makeUpUrlFromCommand("user_info",
                                                              WIZ_TOKEN_IN_URL_REPLACE_PART, extInfo);
    WizShowWebDialogWithToken(tr("Account settings"), strUrl, window);
#else
    WizIAPDialog* dlg = window->iapDialog();
    dlg->loadUserInfo();
    dlg->exec();
#endif
    //
    //// 用户可能会在设置页面中修改信息，此处清除token以便重新同步
    //
    WizToken::clearToken();
}

void WizUserInfoWidget::on_action_upgradeVip_triggered()
{
    WizMainWindow* window = dynamic_cast<WizMainWindow*>(m_app.mainWindow());
#ifndef BUILD4APPSTORE
    QString strToken = WizToken::token();
    QString extInfo = WizCommonApiEntry::appstoreParam(false);
    QString strUrl = WizCommonApiEntry::makeUpUrlFromCommand("vip", strToken, extInfo);
    WizShowWebDialogWithToken(tr("Account settings"), strUrl, window);
    //QDesktopServices::openUrl(strUrl);
#else
    WizIAPDialog* dlg = window->iapDialog();
    dlg->loadIAPPage();
    dlg->exec();
#endif
}

void WizUserInfoWidget::on_action_changeAvatar_triggered()
{
    QFileDialog dialog;
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter("*.png *.jpg *.jpeg");

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QStringList listFiles = dialog.selectedFiles();
    if (listFiles.size() != 1) {
        return;
    }

    QString fileName = listFiles[0];
    WizExecuteOnThread(WIZ_THREAD_NETWORK, [=](){
        WizAvatarUploader* uploader = new WizAvatarUploader(nullptr);
        connect(uploader, SIGNAL(uploaded(bool)), SLOT(on_action_changeAvatar_uploaded(bool)));
        uploader->upload(fileName);
    });
}

void WizUserInfoWidget::on_action_changeAvatar_uploaded(bool ok)
{
    WizAvatarUploader* uploader = qobject_cast<WizAvatarUploader*>(sender());    

    if (ok) {
        WizAvatarHost::reload(m_db.getUserId());
    } else {
        WizMainWindow* window = dynamic_cast<WizMainWindow*>(m_app.mainWindow());
        QMessageBox::warning(window, tr("Upload Avatar"), uploader->lastErrorMessage());
    }

    uploader->deleteLater();
}

void WizUserInfoWidget::on_action_viewNotesOnWeb_triggered()
{
    QString strToken = WizToken::token();
    QString strUrl = WizCommonApiEntry::makeUpUrlFromCommand("service", strToken);

    qDebug() << "open dialog with token: "  << strUrl;
    //
    QDesktopServices::openUrl(strUrl);
}

void WizUserInfoWidget::on_action_mySharedNotes_triggered()
{
    QString strToken = WizToken::token();
    QString strUrl = WizCommonApiEntry::newStandardCommandUrl("my_share", strToken, "");

    qDebug() << "open dialog with token: "  << strUrl;
    QDesktopServices::openUrl(strUrl);
}

void WizUserInfoWidget::on_action_logout_triggered()
{
    WizMainWindow* window = dynamic_cast<WizMainWindow*>(m_app.mainWindow());
    window->on_actionLogout_triggered();
}

void WizUserInfoWidget::on_userInfo_changed()
{
    resetUserInfo();
}
QString WizUserInfoWidget::userId()
{
    return m_db.getUserId();
}
void WizUserInfoWidget::updateUI()
{
    m_circleAvatar = QPixmap();
    WIZUSERINFOWIDGETBASE::updateUI();
}

QPixmap WizUserInfoWidget::getCircleAvatar(int width, int height)
{
    if (width <= 0 || height <= 0)
        return QPixmap();
    //
    if (!m_circleAvatar.isNull())
    {
        if (QSize(width, height) == m_circleAvatar.size())
            return m_circleAvatar;
        //
        m_circleAvatar = QPixmap();
    }

    QPixmap org = WizAvatarHost::orgAvatar(userId());
    if (org.isNull())
        return org;
    //
    m_circleAvatar = WizAvatarHost::circleImage(org, width, height);
    return m_circleAvatar;
}



QPixmap WizUserInfoWidget::getAvatar(int width, int height)
{
    return getCircleAvatar(width, width);
}

QIcon WizUserInfoWidget::getVipIcon()
{
    return m_iconVipIndicator;
}

QSize WizUserInfoWidget::sizeHint() const
{
    // FIXME: builtin avatar size (36, 36), margin = 4 * 2, arraw width = 10
    int vipIconWidth = WizSmartScaleUI(35);
    return QSize(WizSmartScaleUI(36)+ textWidth() + WizSmartScaleUI(8) + vipIconWidth, WizSmartScaleUI(36));
}

