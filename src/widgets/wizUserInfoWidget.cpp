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
    , m_userSettings(NULL)
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

    QAction* actionAccountInfo = new QAction(tr("View account info"), m_menuMain);
    connect(actionAccountInfo, SIGNAL(triggered()), SLOT(on_action_accountInfo_triggered()));
    actionAccountInfo->setVisible(false);

    QAction* actionAccountSetup = new QAction(tr("Account settings"), m_menuMain);
    connect(actionAccountSetup, SIGNAL(triggered()), SLOT(on_action_accountSetup_triggered()));

    QAction* actionChangeAvatar = new QAction(tr("Change avatar"), m_menuMain);
    connect(actionChangeAvatar, SIGNAL(triggered()), SLOT(on_action_changeAvatar_triggered()));

    m_menuMain->addAction(actionAccountInfo);
    m_menuMain->addAction(actionAccountSetup);
    m_menuMain->addSeparator();
    m_menuMain->addAction(actionChangeAvatar);
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

    update();
}

void CWizUserInfoWidget::on_action_accountInfo_triggered()
{

}

void CWizUserInfoWidget::on_action_accountSetup_triggered()
{
    if (!m_userSettings) {
        m_userSettings = new CWizWebSettingsDialog(QSize(720, 400), window()); // use toplevel window as parent
        m_userSettings->setWindowTitle(tr("Account settings"));
        connect(m_userSettings, SIGNAL(accepted()), m_userSettings, SLOT(deleteLater()));
        connect(m_userSettings, SIGNAL(showProgress()), SLOT(on_action_accountSetup_showProgress()));
    }

    m_userSettings->open();
}

void CWizUserInfoWidget::on_action_accountSetup_showProgress()
{
    connect(Token::instance(), SIGNAL(tokenAcquired(const QString&)),
            SLOT(on_action_accountSetup_requested(const QString&)), Qt::QueuedConnection);

    Token::requestToken();
}

void CWizUserInfoWidget::on_action_accountSetup_requested(const QString& strToken)
{
    Token::instance()->disconnect(this);

    if (!m_userSettings)
        return;

    if (strToken.isEmpty()) {
        m_userSettings->showError();
        return;
    }

    QString strUrl = WizService::ApiEntry::accountInfoUrl(strToken);
    m_userSettings->load(QUrl::fromEncoded(strUrl.toUtf8()));
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
        //downloadAvatar();
    } else {
        QMessageBox::warning(this, tr("Upload Avatar"), uploader->lastErrorMessage());
    }

    uploader->deleteLater();
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



QPixmap corpAvatar(const QPixmap& org)
{
    if (org.isNull())
        return org;
    //
    QSize sz = org.size();
    //
    int width = sz.width();
    int height = sz.height();
    if (width == height)
        return org;
    //
    if (width > height)
    {
        int xOffset = (width - height) / 2;
        return org.copy(xOffset, 0, height, height);
    }
    else
    {
        int yOffset = (height - width) / 2;
        return org.copy(0, yOffset, width, width);
    }
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
    org = corpAvatar(org);
    //
    int largeWidth = width * 8;
    int largeHeight = height * 8;
    //
    QPixmap orgResized = org.scaled(QSize(largeWidth, largeHeight), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    //
    QPixmap largePixmap(QSize(largeWidth, largeHeight));
    largePixmap.fill(QColor(Qt::transparent));
    //
    QPainter painter(&largePixmap);
    //
    painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
    QPainterPath path;
    path.addEllipse(0, 0, largeWidth, largeHeight);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, orgResized);
    //
    m_circleAvatar = largePixmap.scaled(QSize(width, height), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    //
    return m_circleAvatar;
}



QPixmap CWizUserInfoWidget::getAvatar()
{
    return getCircleAvatar(32, 32);
    /*
    QPixmap pixmap;
    AvatarHost::avatar(m_db.getUserId(), &pixmap);
    return pixmap;
    */
}

QSize CWizUserInfoWidget::sizeHint() const
{
    // FIXME: builtin avatar size (36, 36), margin = 4 * 2, arraw width = 10

    return QSize(32+ textWidth() + 24, 32);
}
