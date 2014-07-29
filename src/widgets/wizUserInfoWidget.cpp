#include <QtGlobal>
#include "wizUserInfoWidget.h"

#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>

#include "wizdef.h"
#include "share/wizsettings.h"
#include "share/wizDatabaseManager.h"
#include "../wizmainwindow.h"
#include "sync/apientry.h"
#include "sync/wizkmxmlrpc.h"
#include "wizWebSettingsDialog.h"
#include "sync/avataruploader.h"
#include "sync/avatar.h"
#include "sync/token.h"

using namespace WizService;
using namespace WizService::Internal;
using namespace Core::Internal;


CWizUserInfoWidget::CWizUserInfoWidget(CWizExplorerApp& app, QWidget *parent)
    : WIZUSERINFOWIDGETBASE(parent)
    , m_app(app)
    , m_db(app.databaseManager().db())
{
    connect(AvatarHost::instance(), SIGNAL(loaded(const QString&)),
            SLOT(on_userAvatar_loaded(const QString&)));

    AvatarHost::load(m_db.getUserId());

    resetUserInfo();

    connect(&m_db, SIGNAL(userInfoChanged()), SLOT(on_userInfo_changed()));

    // load builtin arraw
    QString strIconPath = ::WizGetSkinResourcePath(app.userSettings().skin()) + "arrow.png";
    m_iconArraw.addFile(strIconPath);

    // setup menu
    m_menuMain = new QMenu(this);

    QAction* actionAccountInfo = new QAction(tr("View account info..."), m_menuMain);
    connect(actionAccountInfo, SIGNAL(triggered()), SLOT(on_action_accountInfo_triggered()));
    actionAccountInfo->setVisible(false);

    QAction* actionAccountSetup = new QAction(tr("Account settings..."), m_menuMain);
    connect(actionAccountSetup, SIGNAL(triggered()), SLOT(on_action_accountSetup_triggered()));

    QAction* actionChangeAvatar = new QAction(tr("Change avatar..."), m_menuMain);
    connect(actionChangeAvatar, SIGNAL(triggered()), SLOT(on_action_changeAvatar_triggered()));

    QAction* actionLogout = new QAction(tr("Logout..."), m_menuMain);
    connect(actionLogout, SIGNAL(triggered()), SLOT(on_action_logout_triggered()));

    m_menuMain->addAction(actionAccountInfo);
    m_menuMain->addAction(actionAccountSetup);
    m_menuMain->addAction(actionChangeAvatar);
    m_menuMain->addSeparator();
    m_menuMain->addAction(actionLogout);
    //
    setMenu(m_menuMain);
}

void CWizUserInfoWidget::resetUserInfo()
{
    WIZUSERINFO info;
    if (!m_db.GetUserInfo(info))
        return;

    if (info.strDisplayName.isEmpty()) {
        setText(::WizGetEmailPrefix(m_db.GetUserId()));
    } else {
        QString strName = fontMetrics().elidedText(info.strDisplayName, Qt::ElideRight, 150);
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

void CWizUserInfoWidget::on_userAvatar_loaded(const QString& strGUID)
{
    if (strGUID != m_db.GetUserId())
        return;

    updateUI();
}

void CWizUserInfoWidget::on_action_accountInfo_triggered()
{

}

void CWizUserInfoWidget::on_action_accountSetup_triggered()
{
    QString strUrl = WizService::ApiEntry::accountInfoUrl(WIZ_TOKEN_IN_URL_REPLACE_PART);
    showWebDialogWithToken(tr("Account settings"), strUrl, window());
}

void CWizUserInfoWidget::on_action_changeAvatar_triggered()
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

    AvatarUploader* uploader = new AvatarUploader(this);
    connect(uploader, SIGNAL(uploaded(bool)), SLOT(on_action_changeAvatar_uploaded(bool)));
    uploader->upload(listFiles[0]);
}

void CWizUserInfoWidget::on_action_changeAvatar_uploaded(bool ok)
{
    AvatarUploader* uploader = qobject_cast<AvatarUploader*>(sender());

    if (ok) {
        AvatarHost::load(m_db.GetUserId(), true);
    } else {
        QMessageBox::warning(this, tr("Upload Avatar"), uploader->lastErrorMessage());
    }

    uploader->deleteLater();
}

void CWizUserInfoWidget::on_action_logout_triggered()
{
    MainWindow* window = dynamic_cast<MainWindow*>(m_app.mainWindow());
    window->on_actionLogout_triggered();
}

void CWizUserInfoWidget::on_userInfo_changed()
{
    AvatarHost::load(m_db.GetUserId(), true);
    resetUserInfo();
}
QString CWizUserInfoWidget::userId()
{
    return m_db.getUserId();
}
void CWizUserInfoWidget::updateUI()
{
    m_circleAvatar = QPixmap();
    WIZUSERINFOWIDGETBASE::updateUI();
}

QPixmap CWizUserInfoWidget::getCircleAvatar(int width, int height)
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

    QPixmap org = AvatarHost::orgAvatar(userId());
    if (org.isNull())
        return org;
    //
    m_circleAvatar = AvatarHost::circleImage(org, width, height);
    return m_circleAvatar;
}



QPixmap CWizUserInfoWidget::getAvatar(int width, int height)
{
    return getCircleAvatar(width, width);
}

QSize CWizUserInfoWidget::sizeHint() const
{
    // FIXME: builtin avatar size (36, 36), margin = 4 * 2, arraw width = 10

    return QSize(32+ textWidth() + 24, 32);
}
