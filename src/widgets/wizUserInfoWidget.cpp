#include "wizUserInfoWidget.h"

#include <QtGui>

#include "wizdef.h"
#include "share/wizsettings.h"
#include "share/wizDatabaseManager.h"
#include "share/wizUserAvatar.h"
#include "../wizmainwindow.h"


CWizUserInfoWidget::CWizUserInfoWidget(CWizExplorerApp& app, QWidget *parent)
    : QToolButton(parent)
    , m_app(app)
    , m_db(app.databaseManager().db())
{
    setPopupMode(QToolButton::MenuButtonPopup);

    resetAvatar();
    resetUserInfo();

    // load builtin arraw
    QString strIconPath = ::WizGetSkinResourcePath(app.userSettings().skin()) + "arrow.png";
    m_iconArraw.addFile(strIconPath);

    // setup menu
    m_menu = new QMenu(this);

    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
    m_avatarDownloader = mainWindow->avatarHost();
    connect(m_avatarDownloader, SIGNAL(downloaded(const QString&)),
            SLOT(on_userAvatar_downloaded(const QString&)));
}

QSize CWizUserInfoWidget::sizeHint() const
{
    // FIXME: builtin avatar size (36, 36), margin = 4 * 2, arraw width = 10
    return QSize(36 + fontMetrics().width(text()) + 18, 36);
}

void CWizUserInfoWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    int nAvatarWidth = 36;
    int nArrawWidth = 10;
    int nMargin = 4;

    QStyleOptionToolButton opt;
    initStyleOption(&opt);

    QPainter p(this);
    p.setClipRect(opt.rect);

    // draw user avatar
    QRect rectIcon = opt.rect;
    rectIcon.setLeft(rectIcon.left());
    rectIcon.setRight(rectIcon.left() + nAvatarWidth);
    if (!opt.icon.isNull()) {
        opt.icon.paint(&p, rectIcon);
    }

    // draw vip indicator
    QRect rectVip = rectIcon;
    rectVip.setLeft(rectVip.right() + nMargin);
    rectVip.setRight(rectVip.left() + fontMetrics().width(opt.text));
    rectVip.setBottom(rectVip.top() + rectVip.height()/2);
    if (!m_iconVipIndicator.isNull()) {
        m_iconVipIndicator.paint(&p, rectVip, Qt::AlignLeft|Qt::AlignTop);
    }

    // draw display name
    QRect rectText = rectVip;
    rectText.setBottom(rectText.bottom() + rectVip.height());
    rectText.setTop(rectText.bottom() - rectVip.height());
    if (!opt.text.isEmpty()) {
        if (opt.state & QStyle::State_MouseOver) {
            QFont font = p.font();
            font.setUnderline(true);
            p.setFont(font);
        }

        p.drawText(rectText, Qt::AlignCenter, opt.text);
    }

    // draw arraw
    QRect rectArrow = rectText;
    rectArrow.setLeft(rectArrow.right() + nMargin);
    rectArrow.setRight(rectArrow.left() + nArrawWidth);
    if (!m_iconArraw.isNull()) {
        m_iconArraw.paint(&p, rectArrow, Qt::AlignVCenter, QIcon::Normal);
    }
}

void CWizUserInfoWidget::resetAvatar()
{
    QString strAvatarPath = m_db.GetAvatarPath() + m_db.GetUserGUID() + ".png";
    if (!QFile::exists(strAvatarPath)) {
        strAvatarPath = ::WizGetSkinResourcePath(m_app.userSettings().skin()) + "avatar.png";
        QTimer::singleShot(3000, this, SLOT(downloadAvatar()));
    }
    setIcon(QIcon(strAvatarPath));
}

void CWizUserInfoWidget::resetUserInfo()
{
    WIZUSERINFO info;
    if (!m_db.GetUserInfo(info))
        return;

    // FIXME:  avoid display name is too long
    setText(info.strDisplayName.left(20));

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
}

void CWizUserInfoWidget::downloadAvatar()
{
    m_avatarDownloader->download(m_db.GetUserGUID());
}

void CWizUserInfoWidget::on_userAvatar_downloaded(const QString& strGUID)
{
    if (strGUID != m_db.GetUserGUID())
        return;

    resetAvatar();
    update();
}
