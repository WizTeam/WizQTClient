#include "wizCategoryViewItem.h"

#include <QDebug>
#include <QTextCodec>
#include <CString>

#include <extensionsystem/pluginmanager.h>

#include "utils/pinyin.h"

#include "wizdef.h"
#include "wizCategoryView.h"
#include "share/wizsettings.h"
#include "wiznotestyle.h"
#include "share/wizDatabaseManager.h"

#define PREDEFINED_TRASH            QObject::tr("Trash")
#define PREDEFINED_UNCLASSIFIED     QObject::tr("Unclassified")

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
    QLocale local = QLocale::system();
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


    QString strThis = text(0);
    QString strOther = pOther->text(0);
    //
    static bool isSimpChinese = IsSimpChinese();
    if (isSimpChinese)
    {
        QByteArray arrThis = QTextCodec::codecForName("GBK")->fromUnicode(strThis);
        QByteArray arrOther = QTextCodec::codecForName("GBK")->fromUnicode(strOther);
        //
        std::string strThisA(arrThis.data(), arrThis.size());
        std::string strOtherA(arrOther.data(), arrOther.size());
        //
        return strThisA.compare(strOtherA.c_str()) < 0;
    }
    else
    {
        return strThis.compare(strOther) < 0;
    }
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

    if (nCurrent == -1) {
        countString = QString("(%1)").arg(nTotal);
    } else {
        countString = QString("(%1/%2)").arg(nCurrent).arg(nTotal);
    }
}

void CWizCategoryViewItemBase::draw(QPainter* p, const QStyleOptionViewItemV4 *vopt) const
{
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
    QString strCount = pItem->countString;
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


/* ------------------------------ CWizCategoryViewSpacerItem ------------------------------ */

CWizCategoryViewSpacerItem::CWizCategoryViewSpacerItem(CWizExplorerApp& app)
    : CWizCategoryViewItemBase(app)
{
    setFlags(Qt::NoItemFlags); // user can not interact with it.
    setText(0, "=");
}

int CWizCategoryViewSpacerItem::getItemHeight(int hintHeight) const
{
    Q_UNUSED(hintHeight);
    return 12;
}

/* ------------------------------ CWizCategoryViewSeparatorItem ------------------------------ */

CWizCategoryViewSeparatorItem::CWizCategoryViewSeparatorItem(CWizExplorerApp& app)
    : CWizCategoryViewItemBase(app)
{
    setFlags(Qt::NoItemFlags); // user can not interact with it.
    setText(0, "-");
}

int CWizCategoryViewSeparatorItem::getItemHeight(int nHeight) const
{
    Q_UNUSED(nHeight);
    return 12;
}


/* --------------------- CWizCategoryViewCategoryItem --------------------- */
CWizCategoryViewCategoryItem::CWizCategoryViewCategoryItem(CWizExplorerApp& app,
                                                           const QString& strName)
    : CWizCategoryViewItemBase(app, strName)
{
    setFlags(Qt::NoItemFlags); // user can not interact with it.
    setText(0, strName);
}


/* -------------------- CWizCategoryViewMessageRootItem -------------------- */
CWizCategoryViewMessageItem::CWizCategoryViewMessageItem(CWizExplorerApp& app,
                                                                 const QString& strName, int nFilterType)
    : CWizCategoryViewItemBase(app, strName)
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

void CWizCategoryViewMessageItem::getDocuments(CWizDatabase& db,
                                                   CWizDocumentDataArray& arrayDocument)
{
    CWizMessageDataArray arrayMsg;
    db.getLastestMessages(arrayMsg);

    for (CWizMessageDataArray::const_iterator it = arrayMsg.begin();
         it != arrayMsg.end();
         it++) {
        const WIZMESSAGEDATA& msg = *it;

        WIZDOCUMENTDATAEX doc;
        doc.strKbGUID = msg.kbGUID;
        doc.strGUID = msg.documentGUID;
        doc.strTitle = msg.title;

        // CWizCategoryView responsible for converting to full field message data
        // refer to CWizCategoryView::setDocuments()
        arrayDocument.push_back(doc);
    }
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

    if (m_strName == data.strLocation && data.strKbGUID == kbGUID()) {
        return true;
    }

    return false;
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
    CString strTagGUIDs = db.GetDocumentTagGUIDsString(data.strGUID);

    if (strTagGUIDs.Find(m_tag.strGUID) != -1 && data.strKbGUID == kbGUID()) {
        return true;
    }

    return false;
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

/* ---------------------------- CWizCategoryViewAllGroupsRootItem ---------------------------- */

CWizCategoryViewAllGroupsRootItem::CWizCategoryViewAllGroupsRootItem(CWizExplorerApp& app,
                                                                     const QString& strName,
                                                                     const QString& strKbGUID)
    : CWizCategoryViewItemBase(app, strName, strKbGUID)
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "group_normal"),
                 QSize(16, 16), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "group_selected"),
                 QSize(16, 16), QIcon::Selected);
    setIcon(0, icon);
    setText(0, strName);
}

/* ------------------------------ CWizCategoryViewGroupRootItem ------------------------------ */
CWizCategoryViewBizGroupRootItem::CWizCategoryViewBizGroupRootItem(CWizExplorerApp& app,
                                                                   const QString& strName,
                                                                   const QString& strKbGUID)
    : CWizCategoryViewAllGroupsRootItem(app, strName, strKbGUID)
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "group_biz_normal"),
                 QSize(16, 16), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "group_biz_selected"),
                 QSize(16, 16), QIcon::Selected);
    setIcon(0, icon);
    setText(0, strName);
}

/* ------------------------------ CWizCategoryViewGroupRootItem ------------------------------ */

CWizCategoryViewGroupRootItem::CWizCategoryViewGroupRootItem(CWizExplorerApp& app,
                                                             const QString& strName,
                                                             const QString& strKbGUID)
    : CWizCategoryViewItemBase(app, strName, strKbGUID)
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "group_normal"),
                 QSize(16, 16), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "group_selected"),
                 QSize(16, 16), QIcon::Selected);
    setIcon(0, icon);
    setText(0, strName);
}

void CWizCategoryViewGroupRootItem::showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
{
    if (CWizCategoryView* view = dynamic_cast<CWizCategoryView *>(pCtrl)) {
            view->showGroupRootContextMenu(pos);
    }
}

void CWizCategoryViewGroupRootItem::getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    db.getLastestDocuments(arrayDocument);
}

bool CWizCategoryViewGroupRootItem::accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
{
    if (db.IsInDeletedItems(data.strLocation)) {
        return false;
    }

    QString strTagGUIDs = db.GetDocumentTagGUIDsString(data.strGUID);
    if (strTagGUIDs.isEmpty() && data.strKbGUID == kbGUID()) {
        return true;
    }

    return false;
}

void CWizCategoryViewGroupRootItem::reload(CWizDatabase& db)
{
    setText(0, db.name());
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
    if (db.IsInDeletedItems(data.strLocation)) {
        return false;
    }

    QString strTagGUIDs = db.GetDocumentTagGUIDsString(data.strGUID);
    if (strTagGUIDs.indexOf(m_tag.strGUID) != -1 && data.strKbGUID == kbGUID()) {
        return true;
    }

    return false;
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
