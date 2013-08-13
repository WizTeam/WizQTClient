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

    // load builtin arraw
    QString strIconPath = ::WizGetSkinResourcePath(app.userSettings().skin()) + "arrow.png";
    m_iconArraw.addFile(strIconPath);

    WIZUSERINFO info;
    if (m_db.GetUserInfo(info)) {
        setText(info.strDisplayName.left(30)); // to avoid to long
    }

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

    // draw display name
    QRect rectText = rectIcon;
    rectText.setLeft(rectText.right() + nMargin);
    rectText.setRight(rectText.left() + fontMetrics().width(opt.text));
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

