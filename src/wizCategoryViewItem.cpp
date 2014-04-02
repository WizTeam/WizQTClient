#include "wizCategoryViewItem.h"

#include <QDebug>
#include <QTextCodec>
#include <QPainter>
#include <cstring>
#include <QFile>

#include <extensionsystem/pluginmanager.h>
#include "utils/pinyin.h"
#include "utils/stylehelper.h"
#include "utils/notify.h"
#include "utils/logger.h"

#include "wizCategoryView.h"
#include "wizmainwindow.h"
#include "wizDocumentTransitionView.h"
#include "share/wizObjectDataDownloader.h"
#include "wizProgressDialog.h"

#include "wizdef.h"
#include "share/wizsettings.h"
#include "wiznotestyle.h"
#include "share/wizDatabaseManager.h"

#define PREDEFINED_TRASH            QObject::tr("Trash")
#define PREDEFINED_UNCLASSIFIED     QObject::tr("Unclassified")

#define CATEGORY_OWN_GROUPS      QObject::tr("My Groups")
#define CATEGORY_OTHER_GROUPS      QObject::tr("Other Groups")


#define WIZ_CATEGORY_SECTION_GENERAL QObject::tr("General")
#define WIZ_CATEGORY_SECTION_PERSONAL QObject::tr("Personal Notes")
#define WIZ_CATEGORY_SECTION_GROUPS QObject::tr("Team & Groups")


using namespace Core;

/* ------------------------------ CWizCategoryViewItemBase ------------------------------ */

CWizCategoryViewItemBase::CWizCategoryViewItemBase(CWizExplorerApp& app,
                                                   const QString& strName,
                                                   const QString& strKbGUID)
    : QTreeWidgetItem()
    , m_app(app)
    , m_strName(strName)
    , m_strKbGUID(strKbGUID)
{
}

bool IsSimpChinese()
{
    QLocale local;
    QString name = local.name().toLower();
    if (name == "zh_cn"
        || name == "zh-cn")
    {
        return true;
    }
    return false;
}
bool CWizCategoryViewItemBase::operator < (const QTreeWidgetItem &other) const
{
    const CWizCategoryViewItemBase* pOther = dynamic_cast<const CWizCategoryViewItemBase*>(&other);
    if (!pOther)
        return false;
    //
    int nThis = getSortOrder();
    int nOther = pOther->getSortOrder();
    //
    if (nThis != nOther)
    {
        return nThis < nOther;
    }
    //
    QString strThis = text(0).toLower();
    QString strOther = pOther->text(0).toLower();
    //
    static bool isSimpChinese = IsSimpChinese();
    if (isSimpChinese)
    {
        if (QTextCodec* pCodec = QTextCodec::codecForName("GBK"))
        {
            QByteArray arrThis = pCodec->fromUnicode(strThis);
            QByteArray arrOther = pCodec->fromUnicode(strOther);
            //
            std::string strThisA(arrThis.data(), arrThis.size());
            std::string strOtherA(arrOther.data(), arrOther.size());
            //
            return strThisA.compare(strOtherA.c_str()) < 0;
        }
    }
    //
    return strThis.compare(strOther) < 0;
}

QVariant CWizCategoryViewItemBase::data(int column, int role) const
{
    if (role == Qt::SizeHintRole) {
        int fontHeight = treeWidget()->fontMetrics().height();
        int defHeight = fontHeight + 8;
        int height = getItemHeight(defHeight);
        QSize sz(-1, height);
        return QVariant(sz);
    } else {
        return QTreeWidgetItem::data(column, role);
    }
}

int CWizCategoryViewItemBase::getItemHeight(int hintHeight) const
{
    return hintHeight;
}


QString CWizCategoryViewItemBase::id() const
{
    return ::WizMd5StringNoSpaceJava(QString(text(0) + m_strKbGUID).toUtf8());
}

void CWizCategoryViewItemBase::setDocumentsCount(int nCurrent, int nTotal)
{
    Q_ASSERT(nTotal != -1);

    if (nCurrent == -1)
    {
        if (nTotal == 0)
        {
            m_countString = "";
        }
        else
        {
            m_countString = QString("(%1)").arg(nTotal);
        }
    }
    else
    {
        m_countString = QString("(%1/%2)").arg(nCurrent).arg(nTotal);
    }
}

bool CWizCategoryViewItemBase::getExtraButtonIcon(QPixmap &ret) const
{
    ret = m_extraButtonIcon;
    return !m_extraButtonIcon.isNull();
}

QRect CWizCategoryViewItemBase::getExtraButtonRect(const QRect& rcItemBorder) const
{
    int nMargin = 4;
    QSize szBtn(16, 16);
    if (!m_extraButtonIcon.isNull()) {
        szBtn = m_extraButtonIcon.size();
    } else {
        return QRect(0, 0, 0, 0);
    }
    int nWidth = szBtn.width() + 2 * nMargin;
    int nHeight = szBtn.height() + 2 * nMargin;
    //
    int nTop = rcItemBorder.y() + (rcItemBorder.height() - nHeight) / 2;
    QRect rcb(rcItemBorder.right() - nWidth - 2 * nMargin, nTop, nWidth, nHeight);
    rcb.adjust(nMargin, nMargin, -nMargin, -nMargin);
    return rcb;
}

bool CWizCategoryViewItemBase::extraButtonClickTest()
{
    QPixmap pixmap;
    if(!getExtraButtonIcon(pixmap) || pixmap.isNull())
        return false;

    CWizCategoryBaseView* view = dynamic_cast<CWizCategoryBaseView*>(treeWidget());
    Q_ASSERT(view);

    QRect rcIemBorder = view->visualItemRect(this);
    QRect btnRect = getExtraButtonRect(rcIemBorder);
    int nClickDist = 2;
    btnRect.adjust(-nClickDist, -nClickDist, nClickDist, nClickDist);

    return btnRect.contains(view->hitPoint());
}

void CWizCategoryViewItemBase::draw(QPainter* p, const QStyleOptionViewItemV4* vopt) const
{
    QPixmap pixmap;
    if(getExtraButtonIcon(pixmap) && !pixmap.isNull())
    {
        p->save();

        QRect rcb = getExtraButtonRect(vopt->rect);
        p->setRenderHint(QPainter::Antialiasing);
        p->drawPixmap(rcb, pixmap);

        p->restore();
    }



#if 0
    if (!vopt->icon.isNull()) {
        QRect iconRect = subElementRect(SE_ItemViewItemDecoration, vopt, view);

        if (vopt->state.testFlag(State_Selected)) {
            vopt->icon.paint(p, iconRect, Qt::AlignCenter, QIcon::Selected);
        } else {
            vopt->icon.paint(p, iconRect, Qt::AlignCenter, QIcon::Normal);
        }
    }

    // text should not empty
    if (vopt->text.isEmpty()) {
        Q_ASSERT(0);
        return;
    }

    // draw text little far from icon than the default
    QRect textRect = subElementRect(SE_ItemViewItemText, vopt, view);
    //textRect.adjust(8, 0, 0, 0);

    // draw the text
    QPalette::ColorGroup cg = vopt->state & QStyle::State_Enabled
            ? QPalette::Normal : QPalette::Disabled;
    if (cg == QPalette::Normal && !(vopt->state & QStyle::State_Active))
        cg = QPalette::Inactive;

    if (vopt->state & QStyle::State_Selected) {
        p->setPen(vopt->palette.color(cg, QPalette::HighlightedText));
    } else {
        p->setPen(vopt->palette.color(cg, QPalette::Text));
    }

    if (vopt->state & QStyle::State_Editing) {
        p->setPen(vopt->palette.color(cg, QPalette::Text));
        p->drawRect(textRect.adjusted(0, 0, -1, -1));
    }

    // compute document count string length and leave enough space for drawing
    QString strCount = pItem->m_countString;
    int nCountWidthMax;
    int nMargin = 3;
    QFont fontCount = p->font();
    fontCount.setPointSize(10);

    if (!strCount.isEmpty()) {
        QFont fontOld = p->font();
        p->setFont(fontCount);
        nCountWidthMax = p->fontMetrics().width(strCount) + nMargin;
        textRect.adjust(0, 0, -nCountWidthMax, 0);
        p->setFont(fontOld);
    }

    QFont f = p->font();
    f.setPixelSize(12);
    p->setFont(f);

    QColor colorText = vopt->state.testFlag(State_Selected) ?
                m_colorCategoryTextSelected : m_colorCategoryTextNormal;

    CString strText = vopt->text;
    int nWidth = ::WizDrawTextSingleLine(p, textRect, strText,
                                         Qt::TextSingleLine | Qt::AlignVCenter,
                                         colorText, true);

    // only draw document count if count string is not empty
    if (!strCount.isEmpty()) {
        p->setFont(fontCount);
        textRect.adjust(nWidth + nMargin, 0, nCountWidthMax, 0);
        QColor colorCount = vopt->state.testFlag(State_Selected) ? QColor(230, 230, 230) : QColor(150, 150, 150);
        CString strCount2(strCount);
        ::WizDrawTextSingleLine(p, textRect, strCount2,  Qt::TextSingleLine | Qt::AlignVCenter, colorCount, false);
    }

    p->restore();

#endif
}

/* ------------------------------ CWizCategoryViewSectionItem ------------------------------ */

CWizCategoryViewSectionItem::CWizCategoryViewSectionItem(CWizExplorerApp& app, const QString& strName, int sortOrder)
    : CWizCategoryViewItemBase(app, strName, "")
    , m_sortOrder(sortOrder)
{
    setFlags(Qt::NoItemFlags); // user can not interact with it.
    setText(0, strName);
}

int CWizCategoryViewSectionItem::getItemHeight(int nHeight) const
{
    return nHeight + 12;
}
void CWizCategoryViewSectionItem::reset(const QString& sectionName, int sortOrder)
{
    m_strName = sectionName;
    m_sortOrder = sortOrder;
    //
    setText(0, sectionName);
}

QRect CWizCategoryViewSectionItem::getExtraButtonRect(const QRect& itemBorder) const
{
    int nMargin = 4;
    QSize szBtn = m_extraButtonIcon.size();
    int nWidth = szBtn.width() + 2 * nMargin;
    int nHeight = szBtn.height() + 2 * nMargin;

    //
    int nTop = itemBorder.y() + (itemBorder.height() - nHeight) / 2 + 1.5 * nMargin;
    QRect rcb(itemBorder.right() - nWidth - 2 * nMargin, nTop, nWidth, nHeight);
    rcb.adjust(nMargin, nMargin, -nMargin, -nMargin);
    return rcb;
}

void CWizCategoryViewSectionItem::draw(QPainter* p, const QStyleOptionViewItemV4* vopt) const
{
    QRect rc = vopt->rect;
    rc.setTop(rc.bottom());
    p->fillRect(rc, Utils::StyleHelper::treeViewItemCategoryBackground());

    CWizCategoryViewItemBase::draw(p, vopt);
}


/* -------------------- CWizCategoryViewMessageRootItem -------------------- */
QPoint CWizCategoryViewMessageItem::m_ptUnreadOffset;   //avoid const error
QSize CWizCategoryViewMessageItem::m_szUnreadSize;

CWizCategoryViewMessageItem::CWizCategoryViewMessageItem(CWizExplorerApp& app,
                                                                 const QString& strName, int nFilterType)
    : CWizCategoryViewItemBase(app, strName)
    , m_nUnread(0)
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "messages_normal"),
                 QSize(16, 16), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "messages_selected"),
                 QSize(16, 16), QIcon::Selected);
    setIcon(0, icon);
    setText(0, strName);

    m_nFilter = nFilterType;
}

void CWizCategoryViewMessageItem::getMessages(CWizDatabase& db, CWizMessageDataArray& arrayMsg)
{
    if (hitTestUnread() && m_nUnread) {
        db.getUnreadMessages(arrayMsg);
    } else {
        db.getLastestMessages(arrayMsg);
    }
}

void CWizCategoryViewMessageItem::setUnread(int nCount)
{
   m_nUnread = nCount;

#ifdef Q_OS_MAC
    Utils::Notify::setDockBadge(nCount);
#endif

   CWizCategoryBaseView* view = dynamic_cast<CWizCategoryBaseView*>(treeWidget());
   Q_ASSERT(view);
   view->updateItem(this);
}

QString CWizCategoryViewMessageItem::unreadString() const
{
    if (m_nUnread <= 0)
        return "";
    else if (m_nUnread > 99)
        return "99+";
    else
        return QString::number(m_nUnread);
}

bool CWizCategoryViewMessageItem::hitTestUnread()
{
    CWizCategoryBaseView* view = dynamic_cast<CWizCategoryBaseView*>(treeWidget());
    Q_ASSERT(view);

    QRect rcItem = view->visualItemRect(this);
    //
    QPoint pt = view->hitPoint();
    //
    int x = rcItem.right() - m_ptUnreadOffset.x() - m_szUnreadSize.width();
    int y = rcItem.top() + m_ptUnreadOffset.y();
    QRect rcUnread = QRect(x, y, m_szUnreadSize.width(), m_szUnreadSize.height());
    //
    int left = rcUnread.left();
    int top = rcUnread.top();
    int right = rcUnread.right();
    int bottom = rcUnread.bottom();

    return rcUnread.contains(pt);
}

QString CWizCategoryViewMessageItem::getSectionName()
{
    return WIZ_CATEGORY_SECTION_GENERAL;
}

void CWizCategoryViewMessageItem::draw(QPainter* p, const QStyleOptionViewItemV4 *vopt) const
{
    if (!m_nUnread)
        return;
    //
    QString text = unreadString();
    if (text.isEmpty())
        return;

    p->save();

    QFont f;
    Utils::StyleHelper::fontExtend(f);
    p->setFont(f);
    //
    QSize szText = p->fontMetrics().size(0, text);
    int textWidth = szText.width();
    int textHeight = szText.height();
    //
    int nMargin = textHeight / 3;
    //
    int nWidth = textWidth + 2 * nMargin;
    int nHeight = textHeight + 2 * nMargin;
    if (nWidth < nHeight)
        nWidth = nHeight;
    //
    QRect rcExtButton = getExtraButtonRect(vopt->rect);
    //
    int nTop = vopt->rect.y() + (vopt->rect.height() - nHeight) / 2;
    int nLeft = rcExtButton.right() - nWidth - 2;
    QRect rcb(nLeft, nTop, nWidth, nHeight);

    p->setRenderHint(QPainter::Antialiasing);

    p->setPen(QColor("#2874c9"));
    p->setBrush(QColor("#2874c9"));
    p->drawRoundedRect(rcb, rcb.height() / 2, rcb.height() / 2);
    //
    p->setPen(QColor("#ffffff"));
    p->drawText(rcb, Qt::AlignCenter, text);

    p->restore();
    //
    m_ptUnreadOffset.setX(vopt->rect.right() - rcb.right());
    m_ptUnreadOffset.setY(rcb.top() - vopt->rect.top());
    //
    m_szUnreadSize = rcb.size();
}


/* -------------------- CWizCategoryViewShortcutRootItem -------------------- */
CWizCategoryViewShortcutRootItem::CWizCategoryViewShortcutRootItem(CWizExplorerApp& app,
                                                                   const QString& strName)
    : CWizCategoryViewItemBase(app, strName)
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "shortcut_normal"),
                 QSize(16, 16), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "shortcut_selected"),
                 QSize(16, 16), QIcon::Selected);
    setIcon(0, icon);
    setText(0, strName);
}


/* -------------------- CWizCategoryViewSearchRootItem -------------------- */
CWizCategoryViewSearchRootItem::CWizCategoryViewSearchRootItem(CWizExplorerApp& app,
                                                               const QString& strName)
    : CWizCategoryViewItemBase(app, strName)
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "search_normal"),
                 QSize(16, 16), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "search_selected"),
                 QSize(16, 16), QIcon::Selected);
    setIcon(0, icon);
    setText(0, strName);
}


/* ------------------------------ CWizCategoryViewAllFoldersItem ------------------------------ */

CWizCategoryViewAllFoldersItem::CWizCategoryViewAllFoldersItem(CWizExplorerApp& app,
                                                               const QString& strName,
                                                               const QString& strKbGUID)
    : CWizCategoryViewItemBase(app, strName, strKbGUID)
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "folders_normal"),
                 QSize(16, 16), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "folders_selected"),
                 QSize(16, 16), QIcon::Selected);
    setIcon(0, icon);
    setText(0, strName);
}

void CWizCategoryViewAllFoldersItem::getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    COleDateTime t = ::WizGetCurrentTime();
    t = t.addDays(-60);

    db.GetRecentDocumentsByCreatedTime(t, arrayDocument);
}

bool CWizCategoryViewAllFoldersItem::accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
{
    if (db.IsInDeletedItems(data.strLocation)) {
        return false;
    }

    COleDateTime t = data.tCreated;
    if (t.addDays(60) >= WizGetCurrentTime() && data.strKbGUID == kbGUID()) {
        return true;
    }

    return false;
}

QString CWizCategoryViewAllFoldersItem::getSectionName()
{
    return WIZ_CATEGORY_SECTION_PERSONAL;
}


void CWizCategoryViewAllFoldersItem::showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
{
    if (CWizCategoryView* view = dynamic_cast<CWizCategoryView *>(pCtrl)) {
        view->showFolderRootContextMenu(pos);
    }
}


/* ------------------------------ CWizCategoryViewFolderItem ------------------------------ */

CWizCategoryViewFolderItem::CWizCategoryViewFolderItem(CWizExplorerApp& app,
                                                       const QString& strLocation,
                                                       const QString& strKbGUID)
    : CWizCategoryViewItemBase(app, strLocation, strKbGUID)
{
    QIcon icon;
    if (::WizIsPredefinedLocation(strLocation) && strLocation == "/My Journals/") {
        icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "folder_diary_normal"),
                     QSize(16, 16), QIcon::Normal);
        icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "folder_diary_selected"),
                     QSize(16, 16), QIcon::Selected);
    } else {
        icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "folder_normal"),
                     QSize(16, 16), QIcon::Normal);
        icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "folder_selected"),
                     QSize(16, 16), QIcon::Selected);
    }
    setIcon(0, icon);
    setText(0, CWizDatabase::GetLocationDisplayName(strLocation));
}

QTreeWidgetItem* CWizCategoryViewFolderItem::clone() const
{
    return new CWizCategoryViewFolderItem(m_app, m_strName, m_strKbGUID);
}

void CWizCategoryViewFolderItem::getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    db.GetDocumentsByLocation(m_strName, arrayDocument);
}

bool CWizCategoryViewFolderItem::accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
{
    Q_UNUSED(db);

    if (m_strName == data.strLocation && data.strKbGUID == kbGUID())
        return true;

    return false;
}

bool CWizCategoryViewFolderItem::acceptDrop(const WIZDOCUMENTDATA& data) const
{
    Q_UNUSED(data);

    return true;
}

void CWizCategoryViewFolderItem::drop(const WIZDOCUMENTDATA& data, bool forceCopy)
{
   if (!acceptDrop(data))
       return;

   CWizDatabase& myDb = CWizDatabaseManager::instance()->db(kbGUID());

   if (!forceCopy && kbGUID() == data.strKbGUID)
   {
       CWizFolder folder(myDb, location());
       CWizDocument doc(myDb, data);
       doc.MoveDocument(&folder);
   }
   else
   {
       CWizDatabase& sourceDb = CWizDatabaseManager::instance()->db(data.strKbGUID);
       Internal::MainWindow* window = qobject_cast<Internal::MainWindow *>(m_app.mainWindow());
       //QString strLocation = (location() == LOCATION_DELETED_ITEMS) ? LOCATION_DEFAULT : location();
       QString strLocation = location();
       WIZTAGDATA tagEmpty;
       QString strNewDocGUID;
       sourceDb.CopyDocumentTo(data.strGUID, myDb, strLocation, tagEmpty, strNewDocGUID, window->downloaderHost());
   }
}

void CWizCategoryViewFolderItem::showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
{
    if (CWizCategoryView* view = dynamic_cast<CWizCategoryView *>(pCtrl)) {
        view->showFolderContextMenu(pos);
    }
}

QString CWizCategoryViewFolderItem::name() const
{
    return CWizDatabase::GetLocationName(m_strName);
}

bool CWizCategoryViewFolderItem::operator < (const QTreeWidgetItem &other) const
{
    const CWizCategoryViewFolderItem* pOther = dynamic_cast<const CWizCategoryViewFolderItem*>(&other);
    if (!pOther) {
        return false;
    }

    int nThis = 0, nOther = 0;
    if (!pOther->location().isEmpty()) {
        QSettings* setting = ExtensionSystem::PluginManager::settings();
        nOther = setting->value("FolderPosition/" + pOther->location()).toInt();
        nThis = setting->value("FolderPosition/" + location()).toInt();
    }

    return nThis < nOther;
}



/* ------------------------------ CWizCategoryViewAllTagsItem ------------------------------ */

CWizCategoryViewAllTagsItem::CWizCategoryViewAllTagsItem(CWizExplorerApp& app,
                                                         const QString& strName,
                                                         const QString& strKbGUID)
    : CWizCategoryViewItemBase(app, strName, strKbGUID)
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "tags_normal"),
                 QSize(16, 16), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "tags_selected"),
                 QSize(16, 16), QIcon::Selected);
    setIcon(0, icon);
    setText(0, strName);
}

void CWizCategoryViewAllTagsItem::showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
{
    if (CWizCategoryView* view = dynamic_cast<CWizCategoryView *>(pCtrl)) {
        view->showTagRootContextMenu(pos);
    }
}

void CWizCategoryViewAllTagsItem::getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    Q_UNUSED(db);
    Q_UNUSED(arrayDocument);
    // no deleted
    //db.getDocumentsNoTag(arrayDocument);
}

bool CWizCategoryViewAllTagsItem::accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
{
    CString strTagGUIDs = db.GetDocumentTagGUIDsString(data.strGUID);
    if (strTagGUIDs.isEmpty() && data.strKbGUID == kbGUID()) {
        return true;
    }

    return false;
}

QString CWizCategoryViewAllTagsItem::getSectionName()
{
    return WIZ_CATEGORY_SECTION_PERSONAL;
}

/* ------------------------------ CWizCategoryViewTagItem ------------------------------ */

CWizCategoryViewTagItem::CWizCategoryViewTagItem(CWizExplorerApp& app,
                                                 const WIZTAGDATA& tag,
                                                 const QString& strKbGUID)
    : CWizCategoryViewItemBase(app, tag.strName, strKbGUID)
    , m_tag(tag)
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "tag_normal"),
                 QSize(16, 16), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "tag_selected"),
                 QSize(16, 16), QIcon::Selected);
    setIcon(0, icon);
    setText(0, CWizDatabase::TagNameToDisplayName(tag.strName));
}

QTreeWidgetItem* CWizCategoryViewTagItem::clone() const
{
    return new CWizCategoryViewTagItem(m_app, m_tag, m_strKbGUID);
}

void CWizCategoryViewTagItem::getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    db.GetDocumentsByTag(m_tag, arrayDocument);
}

bool CWizCategoryViewTagItem::accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
{
    if (data.strKbGUID == kbGUID()) {
        QString strTagGUIDs = db.GetDocumentTagGUIDsString(data.strGUID);
        if (strTagGUIDs.indexOf(m_tag.strGUID) != -1)
            return true;
    }

    return false;
}

bool CWizCategoryViewTagItem::acceptDrop(const WIZDOCUMENTDATA& data) const
{
    // only accept drop from user db
    if (data.strKbGUID == kbGUID()) {
        return true;
    }

    return false;
}

void CWizCategoryViewTagItem::drop(const WIZDOCUMENTDATA& data, bool forceCopy)
{
    Q_UNUSED(forceCopy);

    if (!acceptDrop(data)) {
        return;
    }

    // skip
    CWizDatabase& db = CWizDatabaseManager::instance()->db(kbGUID());
    QString strTagGUIDs = db.GetDocumentTagGUIDsString(data.strGUID);
    if (strTagGUIDs.indexOf(m_tag.strGUID) != -1)
        return;

    CWizDocument doc(db, data);
    doc.AddTag(tag());
}

void CWizCategoryViewTagItem::showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
{
    if (CWizCategoryView* view = dynamic_cast<CWizCategoryView *>(pCtrl)) {
        view->showTagContextMenu(pos);
    }
}

void CWizCategoryViewTagItem::reload(CWizDatabase& db)
{
    db.TagFromGUID(m_tag.strGUID, m_tag);
    setText(0, m_tag.strName);
}


/* --------------------- CWizCategoryViewStyleRootItem --------------------- */
CWizCategoryViewStyleRootItem::CWizCategoryViewStyleRootItem(CWizExplorerApp& app,
                                                             const QString& strName)
    : CWizCategoryViewItemBase(app, strName)
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "style_normal"),
                 QSize(16, 16), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "style_selected"),
                 QSize(16, 16), QIcon::Selected);
    setIcon(0, icon);
    setText(0, strName);
}

QString CWizCategoryViewStyleRootItem::getSectionName()
{
    return WIZ_CATEGORY_SECTION_PERSONAL;
}

/* ---------------------------- CWizCategoryViewGroupsRootItem ---------------------------- */

CWizCategoryViewGroupsRootItem::CWizCategoryViewGroupsRootItem(CWizExplorerApp& app, const QString& strName)
    : CWizCategoryViewItemBase(app, strName, "")
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "group_normal"),
                 QSize(16, 16), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "group_selected"),
                 QSize(16, 16), QIcon::Selected);
    setIcon(0, icon);
    setText(0, strName);
}

void CWizCategoryViewGroupsRootItem::showContextMenu(CWizCategoryBaseView *pCtrl, QPoint pos)
{
    if (CWizCategoryView* view = dynamic_cast<CWizCategoryView *>(pCtrl)) {
        view->showNormalGroupRootContextMenu(pos);
    }
}

void CWizCategoryViewGroupsRootItem::getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    Q_UNUSED(db);

    for (int i = 0; i < childCount(); i++) {
        CWizCategoryViewGroupRootItem* pGroup = dynamic_cast<CWizCategoryViewGroupRootItem*>(child(i));
        Q_ASSERT(pGroup);
        if (!pGroup)
            return;

        CWizDatabase& db = CWizDatabaseManager::instance()->db(pGroup->kbGUID());

        CWizDocumentDataArray arrayDoc;
        if (db.GetDocumentsByTime(QDateTime::currentDateTime().addDays(-3), arrayDocument)) {
            arrayDocument.insert(arrayDocument.begin(), arrayDoc.begin(), arrayDoc.end());
        }
    }
}

bool CWizCategoryViewGroupsRootItem::accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
{
    Q_UNUSED(db);

    QDateTime t(QDateTime::currentDateTime().addDays(-3));
    for (int i = 0; i < childCount(); i++) {
        CWizCategoryViewGroupRootItem* pGroup = dynamic_cast<CWizCategoryViewGroupRootItem*>(child(i));
        Q_ASSERT(pGroup);
        if (!pGroup)
            continue;

        if (pGroup->kbGUID() == data.strKbGUID) {
            if (data.tDataModified > t || data.tInfoModified > t || data.tParamModified > t)
                return true;
        }
    }

    return false;
}
QString CWizCategoryViewGroupsRootItem::getSectionName()
{
    return WIZ_CATEGORY_SECTION_GROUPS;
}


/* ------------------------------ CWizCategoryViewGroupRootItem ------------------------------ */
CWizCategoryViewBizGroupRootItem::CWizCategoryViewBizGroupRootItem(CWizExplorerApp& app,
                                                                   const WIZBIZDATA& biz)
    : CWizCategoryViewGroupsRootItem(app, biz.bizName)
    , m_biz(biz)
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "group_biz_normal"),
                 QSize(16, 16), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "group_biz_selected"),
                 QSize(16, 16), QIcon::Selected);
    setIcon(0, icon);
}

void CWizCategoryViewBizGroupRootItem::showContextMenu(CWizCategoryBaseView *pCtrl, QPoint pos)
{
    if (CWizCategoryView* view = dynamic_cast<CWizCategoryView *>(pCtrl)) {
        if(isHr())
        {
            view->showAdminBizGroupRootContextMenu(pos);
        }
        else
        {
//            view->showNormalBizGroupRootContextMenu(pos);
            view->showAdminBizGroupRootContextMenu(pos, false);
        }
    }
}

bool CWizCategoryViewBizGroupRootItem::isOwner()
{
    return m_biz.bizUserRole == WIZ_BIZROLE_OWNER;
}
bool CWizCategoryViewBizGroupRootItem::isAdmin()
{
    return m_biz.bizUserRole == WIZ_BIZROLE_ADMIN;
}

bool CWizCategoryViewBizGroupRootItem::isHr()
{
    return m_biz.bizUserRole <= WIZ_BIZROLE_HR;
}

CWizCategoryViewOwnGroupRootItem::CWizCategoryViewOwnGroupRootItem(CWizExplorerApp& app)
    : CWizCategoryViewGroupsRootItem(app, CATEGORY_OWN_GROUPS)
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "group_normal"),
                 QSize(16, 16), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "group_selected"),
                 QSize(16, 16), QIcon::Selected);
    setIcon(0, icon);
}

void CWizCategoryViewOwnGroupRootItem::showContextMenu(CWizCategoryBaseView *pCtrl, QPoint pos)
{
    Q_UNUSED(pCtrl)
    Q_UNUSED(pos)
//    if (CWizCategoryView* view = dynamic_cast<CWizCategoryView *>(pCtrl)) {
//        view->showOwnGroupRootContextMenu(pos);
//    }
}


CWizCategoryViewJionedGroupRootItem::CWizCategoryViewJionedGroupRootItem(CWizExplorerApp& app)
    : CWizCategoryViewGroupsRootItem(app, CATEGORY_OTHER_GROUPS)
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "group_normal"),
                 QSize(16, 16), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "group_selected"),
                 QSize(16, 16), QIcon::Selected);
    setIcon(0, icon);
}


QString CWizCategoryViewCreateGroupLinkItem::getSectionName()
{
    return WIZ_CATEGORY_SECTION_GROUPS;
}

/* ------------------------------ CWizCategoryViewGroupRootItem ------------------------------ */

CWizCategoryViewGroupRootItem::CWizCategoryViewGroupRootItem(CWizExplorerApp& app,
                                                             const WIZGROUPDATA& group)
    : CWizCategoryViewItemBase(app, group.strGroupName, group.strGroupGUID)
    , m_group(group)
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "group_normal"),
                 QSize(16, 16), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "group_selected"),
                 QSize(16, 16), QIcon::Selected);
    setIcon(0, icon);
    setText(0, m_strName);
}

void CWizCategoryViewGroupRootItem::showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
{
    if (CWizCategoryView* view = dynamic_cast<CWizCategoryView *>(pCtrl)) {
        if(isOwner(m_app.databaseManager().db(m_strKbGUID)))
        {
            view->showOwnerGroupRootContextMenu(pos);
        }
        else if(isAdmin(m_app.databaseManager().db(m_strKbGUID)))
        {
            view->showAdminGroupRootContextMenu(pos);
        }
        else
        {
            view->showNormalGroupRootContextMenu(pos);
        }
    }
}

void CWizCategoryViewGroupRootItem::getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    db.getLastestDocuments(arrayDocument);
}

bool CWizCategoryViewGroupRootItem::accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
{
    if (db.IsInDeletedItems(data.strLocation))
        return false;

    if (data.strKbGUID == kbGUID())
        return true;

    return false;
}

bool CWizCategoryViewGroupRootItem::acceptDrop(const WIZDOCUMENTDATA &data) const
{
    Q_UNUSED(data);

    CWizDatabase& db = CWizDatabaseManager::instance()->db(kbGUID());
    if (WIZ_USERGROUP_AUTHOR >= db.permission())
        return true;

    return false;
}

void CWizCategoryViewGroupRootItem::drop(const WIZDOCUMENTDATA &data, bool forceCopy)
{
    if (!acceptDrop(data))
        return;

    CWizDatabase& myDb = CWizDatabaseManager::instance()->db(kbGUID());

    if (!forceCopy && data.strKbGUID == m_strKbGUID)
    {
        CWizDocument doc(myDb, data);
        if (data.strLocation == LOCATION_DELETED_ITEMS)
        {
            CWizFolder folder(myDb, myDb.GetDefaultNoteLocation());
            doc.MoveDocument(&folder);
        }

        CWizTagDataArray arrayTag;
        myDb.GetDocumentTags(data.strGUID, arrayTag);
        if (arrayTag.size() > 0)
        {
            for (CWizTagDataArray::const_iterator it = arrayTag.begin(); it != arrayTag.end(); it++)
            {
                doc.RemoveTag(*it);
            }
        }
    }
    else
    {
        CWizDatabase& sourceDb = CWizDatabaseManager::instance()->db(data.strKbGUID);
        QString strLocation = myDb.GetDefaultNoteLocation();
        Internal::MainWindow* window = qobject_cast<Internal::MainWindow *>(m_app.mainWindow());
        QString strNewDocGUID;
        WIZTAGDATA tagEmpty;
        sourceDb.CopyDocumentTo(data.strGUID, myDb, strLocation, tagEmpty, strNewDocGUID, window->downloaderHost());
    }
}

void CWizCategoryViewGroupRootItem::reload(CWizDatabase& db)
{
    m_strName = db.name();
    setText(0, db.name());
}

bool CWizCategoryViewGroupRootItem::isAdmin(CWizDatabase& db)
{
    if (isBizGroup())
    {
        if (CWizCategoryViewBizGroupRootItem* pBiz = dynamic_cast<CWizCategoryViewBizGroupRootItem *>(parent()))
        {
            if (pBiz->isAdmin())
                return true;
        }
    }
    //
    return db.IsGroupAdmin();
}

bool CWizCategoryViewGroupRootItem::isOwner(CWizDatabase& db)
{
    return db.IsGroupOwner();
}

bool CWizCategoryViewGroupRootItem::isBizGroup() const
{
    return m_group.IsBiz();
}

QString CWizCategoryViewGroupRootItem::bizGUID() const
{
    return m_group.bizGUID;
}

/* --------------------- CWizCategoryViewGroupNoTagItem --------------------- */
CWizCategoryViewGroupNoTagItem::CWizCategoryViewGroupNoTagItem(CWizExplorerApp& app,
                                                               const QString& strKbGUID)
    : CWizCategoryViewItemBase(app, PREDEFINED_UNCLASSIFIED, strKbGUID)
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "folder_normal"),
                 QSize(16, 16), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "folder_selected"),
                 QSize(16, 16), QIcon::Selected);
    setIcon(0, icon);
    setText(0, PREDEFINED_UNCLASSIFIED);
}

void CWizCategoryViewGroupNoTagItem::getDocuments(CWizDatabase& db,
                                                  CWizDocumentDataArray& arrayDocument)
{
    db.getDocumentsNoTag(arrayDocument);
}

bool CWizCategoryViewGroupNoTagItem::accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
{
    if (db.IsInDeletedItems(data.strLocation))
        return false;

    QString strTagGUIDs = db.GetDocumentTagGUIDsString(data.strGUID);
    if (strTagGUIDs.isEmpty())
        return true;

    return false;
}


/* ------------------------------ CWizCategoryViewGroupItem ------------------------------ */

CWizCategoryViewGroupItem::CWizCategoryViewGroupItem(CWizExplorerApp& app,
                                                     const WIZTAGDATA& tag,
                                                     const QString& strKbGUID)
    : CWizCategoryViewItemBase(app, tag.strName, strKbGUID)
    , m_tag(tag)
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "folder_normal"),
                 QSize(16, 16), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "folder_selected"),
                 QSize(16, 16), QIcon::Selected);
    setIcon(0, icon);
    setText(0, CWizDatabase::TagNameToDisplayName(tag.strName));
}

void CWizCategoryViewGroupItem::showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
{
    if (CWizCategoryView* view = dynamic_cast<CWizCategoryView *>(pCtrl)) {
        view->showGroupContextMenu(pos);
    }
}

void CWizCategoryViewGroupItem::getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    db.GetDocumentsByTag(m_tag, arrayDocument);
}

bool CWizCategoryViewGroupItem::accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
{
    if (db.IsInDeletedItems(data.strLocation))
        return false;

    QString strTagGUIDs = db.GetDocumentTagGUIDsString(data.strGUID);
    if (strTagGUIDs.indexOf(m_tag.strGUID) != -1 && data.strKbGUID == kbGUID())
        return true;

    return false;
}

bool CWizCategoryViewGroupItem::acceptDrop(const WIZDOCUMENTDATA& data) const
{
    Q_UNUSED(data);

    CWizDatabase& db = CWizDatabaseManager::instance()->db(kbGUID());
    if (WIZ_USERGROUP_AUTHOR >= db.permission()) {
        return true;
    }

    return false;
}

void CWizCategoryViewGroupItem::drop(const WIZDOCUMENTDATA& data, bool forceCopy)
{
    if (!acceptDrop(data))
        return;

    CWizDatabase& myDb = CWizDatabaseManager::instance()->db(kbGUID());

    if (!forceCopy && data.strKbGUID == m_strKbGUID)
    {
        CWizDocument doc(myDb, data);
        if (data.strLocation == LOCATION_DELETED_ITEMS)
        {
            CWizFolder folder(myDb, myDb.GetDefaultNoteLocation());
            doc.MoveDocument(&folder);
        }

        CWizTagDataArray arrayTag;
        myDb.GetDocumentTags(data.strGUID, arrayTag);
        if (arrayTag.size() > 0)
        {
            for (CWizTagDataArray::const_iterator it = arrayTag.begin(); it != arrayTag.end(); it++)
            {
                doc.RemoveTag(*it);
            }
        }
        doc.AddTag(tag());

    }
    else
    {
        CWizDatabase& sourceDb = CWizDatabaseManager::instance()->db(data.strKbGUID);
        QString strLocation = myDb.GetDefaultNoteLocation();
        Internal::MainWindow* window = qobject_cast<Internal::MainWindow *>(m_app.mainWindow());
        QString strNewDocGUID;
        sourceDb.CopyDocumentTo(data.strGUID, myDb, strLocation, m_tag, strNewDocGUID, window->downloaderHost());
    }
}

void CWizCategoryViewGroupItem::reload(CWizDatabase& db)
{
    db.TagFromGUID(m_tag.strGUID, m_tag);
    setText(0, m_tag.strName);
}

/* ------------------------------ CWizCategoryViewTrashItem ------------------------------ */

CWizCategoryViewTrashItem::CWizCategoryViewTrashItem(CWizExplorerApp& app,
                                                     const QString& strKbGUID)
    : CWizCategoryViewFolderItem(app, "/Deleted Items/", strKbGUID)
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "trash_normal"),
                 QSize(16, 16), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "trash_selected"),
                 QSize(16, 16), QIcon::Selected);
    setIcon(0, icon);
    setText(0, PREDEFINED_TRASH);
}

void CWizCategoryViewTrashItem::showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
{
    if (CWizCategoryView* view = dynamic_cast<CWizCategoryView *>(pCtrl)) {
        view->showTrashContextMenu(pos);
    }
}

void CWizCategoryViewTrashItem::getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    db.GetDocumentsByLocation(db.GetDeletedItemsLocation(), arrayDocument, true);
}

bool CWizCategoryViewTrashItem::accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
{
    return db.IsInDeletedItems(data.strLocation);
}

bool CWizCategoryViewTrashItem::acceptDrop(const WIZDOCUMENTDATA &data) const
{
    Q_UNUSED(data);

    CWizCategoryViewGroupRootItem* parentItem = dynamic_cast<CWizCategoryViewGroupRootItem*>(parent());
    if (parentItem)
        return false;

    return true;
}


/* ------------------------------ CWizCategoryViewSearchItem ------------------------------ */

//CWizCategoryViewSearchItem::CWizCategoryViewSearchItem(CWizExplorerApp& app, const QString& keywords)
//    : CWizCategoryViewItemBase(app, keywords)
//{
//    setKeywords(keywords);
//    setIcon(0, WizLoadSkinIcon(app.userSettings().skin(), QColor(0xff, 0xff, 0xff), "search"));
//}

//bool CWizCategoryViewSearchItem::accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
//{
//    Q_UNUSED(db);

//    if (m_strName.isEmpty())
//        return false;

//    return -1 != ::WizStrStrI_Pos(data.strTitle, m_strName);
//}

//void CWizCategoryViewSearchItem::setKeywords(const QString& keywords)
//{
//    m_strName = keywords;

//    QString strText = QObject::tr("Search for %1").arg(m_strName);

//    setText(0, strText);
//}
