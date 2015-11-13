#include "wizCategoryViewItem.h"

#include <QTextCodec>
#include <QPainter>
#include <cstring>
#include <QFile>
#include <QStyle>
#include <QDebug>

#include <extensionsystem/pluginmanager.h>
#include "utils/pinyin.h"
#include "utils/stylehelper.h"
#include "utils/notify.h"
#include "utils/logger.h"
#include "utils/misc.h"

#include "wizCategoryView.h"
#include "wizmainwindow.h"
#include "wizDocumentTransitionView.h"
#include "share/wizObjectDataDownloader.h"
#include "share/wizObjectOperator.h"
#include "wizProgressDialog.h"
#include "widgets/wizTipsWidget.h"

#include "wizdef.h"
#include "share/wizsettings.h"
#include "wiznotestyle.h"
#include "share/wizDatabaseManager.h"
#include "share/wizmisc.h"

#define PREDEFINED_TRASH            QObject::tr("Trash")
#define PREDEFINED_UNCLASSIFIED     QObject::tr("Unclassified")

#define CATEGORY_OWN_GROUPS      QObject::tr("My Groups")
#define CATEGORY_OTHER_GROUPS      QObject::tr("Other Groups")


#define WIZ_CATEGORY_SECTION_GENERAL QObject::tr("General")
#define WIZ_CATEGORY_SECTION_PERSONAL QObject::tr("Personal Notes")
#define WIZ_CATEGORY_SECTION_GROUPS QObject::tr("Team & Groups")

#define WIZ_CATEGORY_SHOTCUT_PLACEHOLD QObject::tr("Drag doucment form document list")


const int nNumberButtonHeight = 14;
const int nNumberButtonHorizontalMargin = 3;

using namespace Core;

/* ------------------------------ CWizCategoryViewItemBase ------------------------------ */

CWizCategoryViewItemBase::CWizCategoryViewItemBase(CWizExplorerApp& app,
                                                   const QString& strName,
                                                   const QString& strKbGUID, int type)
    : QTreeWidgetItem(type)
    , m_app(app)
    , m_strName(strName)
    , m_strKbGUID(strKbGUID)
    , m_extraButtonIconPressed(false)
{
}

void CWizCategoryViewItemBase::drawItemBody(QPainter *p, const QStyleOptionViewItemV4 *vopt) const
{
    bool bSelected = vopt->state.testFlag(QStyle::State_Selected);

    QRect rcIcon = treeWidget()->style()->subElementRect(QStyle::SE_ItemViewItemDecoration, vopt, treeWidget());
    if (!vopt->icon.isNull()) {
        const int iconSize = 14;
        rcIcon.adjust(-6, 0, 0, 0);
        rcIcon.setTop(vopt->rect.top() + (vopt->rect.height() - iconSize) / 2);
        rcIcon.setWidth(iconSize);
        rcIcon.setHeight(iconSize);
        Utils::StyleHelper::drawTreeViewItemIcon(p, rcIcon, vopt->icon, bSelected);
    }

    QFont f;
    Utils::StyleHelper::fontCategoryItem(f);

    QFont fontCount;
    Utils::StyleHelper::fontExtend(fontCount);

    //通过subElementRect获取范围会产生不同的结果。此处通过icon进行计算
//    QRect rcText = subElementRect(SE_ItemViewItemText, vopt, view);
    QRect rcText(rcIcon.right() + 8, vopt->rect.top(), vopt->rect.right() - rcIcon.right() - 20,
                 vopt->rect.height());
    QString strCount = countString();

    QString strText = vopt->text;
    if (!strText.isEmpty()) {
        bool secondLevelFolder = (parent() && (parent()->type() == Category_GroupItem
                                                     || parent()->type() == Category_FolderItem));
        QColor colorText = Utils::StyleHelper::treeViewItemText(bSelected, secondLevelFolder);
        colorText.setAlpha(240);
        p->setPen(colorText);
        f.setStyleStrategy(QFont::PreferBitmap);
        QFontMetrics fm(f);
        strText = fm.elidedText(strText, Qt::ElideRight, rcText.width());
        int right = Utils::StyleHelper::drawSingleLineText(p, rcText, strText, Qt::AlignVCenter, colorText, f);
        //
        if (right != -1) {
            rcText.setLeft(right + 4);
        }
    }

    if (!strCount.isEmpty() && (rcText.width() > 10)) {
        QRect rcCount = rcText;
        rcCount.setTop(rcCount.top() + 1);  //add extra 1 pixel for vcenter / 2
        QColor colorCount = Utils::StyleHelper::treeViewItemTextExtend(bSelected);
        Utils::StyleHelper::drawSingleLineText(p, rcCount, strCount, Qt::AlignVCenter, colorCount, fontCount);
    }
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
    static bool isSimpChinese = Utils::Misc::isChinese();
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

int CWizCategoryViewItemBase::getItemHeight(int /*hintHeight*/) const
{
    return Utils::StyleHelper::treeViewItemHeight();
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

void CWizCategoryViewItemBase::setExtraButtonIcon(const QString& file)
{
    if (WizIsHighPixel())
    {
        int nIndex = file.lastIndexOf('.');
        QString strFile = file.left(nIndex) + "@2x" + file.right(file.length() - nIndex);
        if (QFile::exists(strFile))
        {
            m_extraButtonIcon = QPixmap(strFile);
            return;
        }
    }

    m_extraButtonIcon = QPixmap(file);
}

bool CWizCategoryViewItemBase::getExtraButtonIcon(QPixmap &ret) const
{
    ret = m_extraButtonIcon;
    return !m_extraButtonIcon.isNull();
}

const int EXTRABUTTONRIGHTMARGIN = 10;

QRect CWizCategoryViewItemBase::getExtraButtonRect(const QRect &rcItemBorder, bool ignoreIconExist) const
{
    QSize szBtn(16, 16);
    if (!m_extraButtonIcon.isNull())
    {
        szBtn = m_extraButtonIcon.size();
        WizScaleIconSizeForRetina(szBtn);
    }
    else if (!ignoreIconExist)
    {
        return QRect(0, 0, 0, 0);
    }
    int nWidth = szBtn.width();
    int nHeight = szBtn.height();
    //
    int nTop = rcItemBorder.y() + (rcItemBorder.height() - nHeight) / 2;
    QRect rcb(rcItemBorder.right() - nWidth - EXTRABUTTONRIGHTMARGIN, nTop, nWidth, nHeight);
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

QString CWizCategoryViewItemBase::getExtraButtonToolTip() const
{
    return "";
}

void CWizCategoryViewItemBase::drawExtraBadge(QPainter* p, const QStyleOptionViewItemV4* vopt) const
{


    QPixmap pixmap;
    if(getExtraButtonIcon(pixmap) && !pixmap.isNull())
    {
        p->save();

        QRect rcb = getExtraButtonRect(vopt->rect);
        p->setRenderHint(QPainter::Antialiasing);
        p->setClipRect(rcb);
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
    : CWizCategoryViewItemBase(app, strName, "", Category_SectionItem)
    , m_sortOrder(sortOrder)
{
    setFlags(Qt::NoItemFlags); // user can not interact with it.
    setText(0, strName);
}

int CWizCategoryViewSectionItem::getItemHeight(int /*nHeight*/) const
{    
    return 32;
}
void CWizCategoryViewSectionItem::reset(const QString& sectionName, int sortOrder)
{
    m_strName = sectionName;
    m_sortOrder = sortOrder;
    //
    setText(0, sectionName);
}

void CWizCategoryViewSectionItem::drawItemBody(QPainter *p, const QStyleOptionViewItemV4 *vopt) const
{
    QString str = vopt->text;
    QRect rc(vopt->rect);
    rc.adjust(-12, 2, 0, 0);
    QFont font = p->font();
    Utils::StyleHelper::fontSection(font);
    Utils::StyleHelper::drawSingleLineText(p, rc, str, Qt::AlignVCenter, Utils::StyleHelper::treeViewSectionItemText(), font);
}

void CWizCategoryViewSectionItem::drawExtraBadge(QPainter* p, const QStyleOptionViewItemV4* vopt) const
{
//    QRect rc = vopt->rect;
//    rc.setTop(rc.bottom());
//    p->fillRect(rc, Utils::StyleHelper::treeViewItemBottomLine());

    CWizCategoryViewItemBase::drawExtraBadge(p, vopt);
}


/* -------------------- CWizCategoryViewMessageRootItem -------------------- */
CWizCategoryViewMessageItem::CWizCategoryViewMessageItem(CWizExplorerApp& app,
                                                                 const QString& strName, int nFilterType)
    : CWizCategoryViewItemBase(app, strName, "", Category_MessageItem)
    , m_nUnread(0)    
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_messages_normal"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "messages_selected"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Selected);
    setIcon(0, icon);
    setText(0, strName);

    m_nFilter = nFilterType;
}

void CWizCategoryViewMessageItem::getMessages(CWizDatabase& db, const QString& userGUID, CWizMessageDataArray& arrayMsg)
{
    if (hitTestUnread() && m_nUnread) {
        if (userGUID.isEmpty()) {
            db.getUnreadMessages(arrayMsg);
        } else {
            db.unreadMessageFromUserGUID(userGUID, arrayMsg);
        }
    } else {
        if (userGUID.isEmpty()) {
            db.getLastestMessages(arrayMsg);
        } else {
            db.messageFromUserGUID(userGUID, arrayMsg);
        }
    }
}

void CWizCategoryViewMessageItem::setUnreadCount(int nCount)
{
   m_nUnread = nCount;

#ifdef Q_OS_MAC
    Utils::Notify::setDockBadge(nCount);
#endif

   m_nUnread = nCount;
   CWizCategoryBaseView* view = dynamic_cast<CWizCategoryBaseView*>(treeWidget());
   Q_ASSERT(view);
   //
   if (m_nUnread > 0)
   {
       QFont f;
       Utils::StyleHelper::fontNormal(f);
       QFontMetrics fm(f);
       //
       QSize szText = fm.size(0, unreadString());
       int textWidth = szText.width();
//       int textHeight = szText.height();
       //
       //int nMargin = textHeight / 4;
       //
       int nWidth = textWidth + nNumberButtonHorizontalMargin * 2;
       int nHeight = nNumberButtonHeight;
//       int nHeight = textHeight + 2;
       if (nWidth < nHeight)
           nWidth = nHeight;
       //
       QRect rcIemBorder = view->visualItemRect(this);
       QRect rcExtButton = getExtraButtonRect(rcIemBorder, true);
       //
       int nTop = rcIemBorder.y() + (rcIemBorder.height() - nHeight) / 2;
       int nLeft = rcExtButton.right() - nWidth;
       QRect rcb(nLeft, nTop, nWidth, nHeight);

       m_szUnreadSize = rcb.size();
   }

   view->updateItem(this);

}

QString unreadNumToString(int unread)
{
    if (unread <= 0)
        return "";
    else if (unread > 99)
        return "99+";
    else
        return QString::number(unread);
}

QString CWizCategoryViewMessageItem::unreadString() const
{
    return unreadNumToString(m_nUnread);
}

bool CWizCategoryViewMessageItem::hitTestUnread()
{
    if (m_nUnread == 0)
        return false;

    CWizCategoryBaseView* view = dynamic_cast<CWizCategoryBaseView*>(treeWidget());
    Q_ASSERT(view);

    QRect rcItem = view->visualItemRect(this);
    QPoint pt = view->hitPoint();
    //
    QRect rcRect = getExtraButtonRect(rcItem, true);
    return rcRect.contains(pt);
}

QString CWizCategoryViewMessageItem::getSectionName()
{
    return WIZ_CATEGORY_SECTION_GENERAL;
}

QRect CWizCategoryViewMessageItem::getExtraButtonRect(const QRect& itemBorder, bool ignoreIconExist) const
{
    if (!m_nUnread && !ignoreIconExist)
        return QRect();

    int nButtonWidth = 26;
    int nButtonHeight = 14;
    QRect rc(itemBorder.right() - EXTRABUTTONRIGHTMARGIN - nButtonWidth, itemBorder.y() + (itemBorder.height() - nButtonHeight) / 2,
             nButtonWidth, nButtonHeight);
    return rc;
}

//#define CATEGORYMESSAGEITEMTIPSCHECKED "CategoryMessageItemTipsChecked"

//void CWizCategoryViewMessageItem::showCoachingTips()
//{
//    bool showTips = false;
//    if (Core::Internal::MainWindow* mainWindow = Core::Internal::MainWindow::instance())
//    {
//        showTips = mainWindow->userSettings().get(CATEGORYMESSAGEITEMTIPSCHECKED).toInt() == 0;
//    }

//    if (showTips)
//    {
//        CWizTipListManager* manager = CWizTipListManager::instance();
//        if (manager->tipsWidgetExists(CATEGORYMESSAGEITEMTIPSCHECKED))
//            return;

//        CWizTipsWidget* tipWidget = new CWizTipsWidget(CATEGORYMESSAGEITEMTIPSCHECKED, this);
//        tipWidget->setAttribute(Qt::WA_DeleteOnClose, true);
//        tipWidget->setText(tr("More tool items"), tr("Use to show or hide extra tool items."));
//        tipWidget->setSizeHint(QSize(280, 60));
//        tipWidget->setButtonVisible(false);
//        tipWidget->bindFunction([](){
//            if (Core::Internal::MainWindow* mainWindow = Core::Internal::MainWindow::instance())
//            {
//                mainWindow->userSettings().set(CATEGORYMESSAGEITEMTIPSCHECKED, "1");
//            }
//        });
//        //
//        tipWidget->addToTipListManager(m_btnShowExtra, 0, -6);
//    }
//}

void drawClickableUnreadButton(QPainter* p, const QRect& rcd, const QString& text, bool isPressed)
{
    QFont f;
    f.setPixelSize(10);
    p->setFont(f);
    p->setPen("999999");
    //
    QRect rcb = rcd;
    if (isPressed)
    {
        rcb.adjust(0, 0, 0, 2);
        QPixmap pixBg(Utils::StyleHelper::skinResourceFileName("category_unreadButton_selected", true));
        p->drawPixmap(rcb, pixBg);
        rcb.adjust(0, 0, 0, -2);
        p->drawText(rcb, Qt::AlignCenter, text);
    }
    else
    {
        rcb.adjust(0, 0, 0, 2);
        QPixmap pixBg(Utils::StyleHelper::skinResourceFileName("category_unreadButton", true));
        p->drawPixmap(rcb, pixBg);
        rcb.adjust(0, 0, 0, -2);
        p->drawText(rcb, Qt::AlignCenter, text);
    }
}

void CWizCategoryViewMessageItem::drawExtraBadge(QPainter* p, const QStyleOptionViewItemV4 *vopt) const
{
    if (!m_nUnread)
        return;
    //
    QString text = unreadString();
    if (text.isEmpty())
        return;

    p->save();

    //    
    QRect rcb = getExtraButtonRect(vopt->rect, true);
    p->setRenderHint(QPainter::Antialiasing);
    drawClickableUnreadButton(p, rcb, text, m_extraButtonIconPressed);
    //

    p->restore();
}

void CWizCategoryViewMessageItem::mousePressed(const QPoint& pos)
{
    QRect rcBorder = treeWidget()->visualItemRect(this);
    QRect rcIcon = getExtraButtonRect(rcBorder, true);
    if (rcIcon.contains(pos))
    {
        m_extraButtonIconPressed = true;
    }
}

void CWizCategoryViewMessageItem::mouseReleased(const QPoint& pos)
{
    m_extraButtonIconPressed = false;
}

/* -------------------- CWizCategoryViewShortcutRootItem -------------------- */
CWizCategoryViewShortcutRootItem::CWizCategoryViewShortcutRootItem(CWizExplorerApp& app,
                                                                   const QString& strName)
    : CWizCategoryViewItemBase(app, strName, "", Category_ShortcutRootItem)
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_shortcut_normal"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_shortcut_selected"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Selected);
    setIcon(0, icon);
    setText(0, strName);
}

void CWizCategoryViewShortcutRootItem::getDocuments(CWizDatabase& /*db*/, CWizDocumentDataArray& arrayDocument)
{
    for (int i = 0; i < childCount(); i++)
    {
        CWizCategoryViewShortcutItem *pItem = dynamic_cast<CWizCategoryViewShortcutItem*>(child(i));
        if (pItem && !pItem->guid().isEmpty())
        {
            CWizDatabase &db = m_app.databaseManager().db(pItem->kbGUID());
            WIZDOCUMENTDATA doc;
            if (db.DocumentFromGUID(pItem->guid(), doc))
            {
                arrayDocument.push_back(doc);
            }
        }
    }
}

bool CWizCategoryViewShortcutRootItem::accept(CWizDatabase& /*db*/, const WIZDOCUMENTDATA& data)
{
    for (int i = 0; i < childCount(); i++)
    {
        CWizCategoryViewShortcutItem *pItem = dynamic_cast<CWizCategoryViewShortcutItem*>(child(i));
        if (pItem)
        {
            if (pItem->guid() == data.strGUID)
                return true;
        }
    }
    return false;
}

void CWizCategoryViewShortcutRootItem::drop(const CWizDocumentDataArray& arrayDocument, bool /*forceCopy*/)
{
    bool changed = false;
    for (WIZDOCUMENTDATA document : arrayDocument)
    {        
        CWizCategoryViewShortcutItem *pItem = addDocumentToShortcuts(document);
        if (pItem)
        {
            changed = true;
        }
    }

    if (changed)
    {
#if QT_VERSION < 0x050400
        CWizCategoryView* categoryView = dynamic_cast<CWizCategoryView*>(treeWidget());
        QTimer::singleShot(200, categoryView, SLOT(saveShortcutState()));
#else
        QTimer::singleShot(200, [this]() {
            CWizCategoryView* categoryView = dynamic_cast<CWizCategoryView*>(treeWidget());
            Q_ASSERT(categoryView);
            categoryView->saveShortcutState();
        });
#endif
    }
}

void CWizCategoryViewShortcutRootItem::drop(const CWizCategoryViewItemBase* pItem)
{
    CWizCategoryViewShortcutItem* newItem = addItemToShortcuts(pItem);
    if (!newItem)
        return;
    //
    treeWidget()->blockSignals(true);
    treeWidget()->setCurrentItem(newItem);
    treeWidget()->blockSignals(false);
    sortChildren(0, Qt::AscendingOrder);

    CWizCategoryView* categoryView = dynamic_cast<CWizCategoryView*>(treeWidget());
    QTimer::singleShot(200, categoryView, SLOT(saveShortcutState()));
}

bool CWizCategoryViewShortcutRootItem::acceptDrop(const CWizCategoryViewItemBase* pItem) const
{
    if (!pItem)
        return false;

    if (pItem->type() == Category_FolderItem || pItem->type() == Category_TagItem || pItem->type() == Category_GroupItem)
        return true;

    return false;
}

CWizCategoryViewShortcutItem* CWizCategoryViewShortcutRootItem::addItemToShortcuts(const CWizCategoryViewItemBase* pItem)
{
    CWizCategoryViewShortcutItem* newItem = nullptr;
    if (pItem->type() == Category_FolderItem)
    {
        const CWizCategoryViewFolderItem* folderItem = dynamic_cast<const CWizCategoryViewFolderItem*>(pItem);
        newItem = new CWizCategoryViewShortcutItem(m_app, CWizDatabase::GetLocationName(folderItem->location()),
                                                                                 CWizCategoryViewShortcutItem::PersonalFolder, "", "", folderItem->location());
    }
    else if (pItem->type() == Category_TagItem)
    {
        const CWizCategoryViewTagItem* tagItem = dynamic_cast<const CWizCategoryViewTagItem*>(pItem);
        newItem = new CWizCategoryViewShortcutItem(m_app, tagItem->tag().strName, CWizCategoryViewShortcutItem::PersonalTag,
                                                    tagItem->tag().strKbGUID, tagItem->tag().strGUID, "");
    }
    else if (pItem->type() == Category_GroupItem)
    {
        const CWizCategoryViewGroupItem* groupItem = dynamic_cast<const CWizCategoryViewGroupItem*>(pItem);
        newItem = new CWizCategoryViewShortcutItem(m_app, groupItem->tag().strName, CWizCategoryViewShortcutItem::GroupTag,
                                                    groupItem->tag().strKbGUID, groupItem->tag().strGUID, "");
    }
    //
    for (int i = 0; i < childCount(); i++)
    {
        CWizCategoryViewShortcutItem *shortcutItem = dynamic_cast<CWizCategoryViewShortcutItem*>(child(i));
        if (shortcutItem)
        {
            switch (shortcutItem->shortcutType()) {
            case CWizCategoryViewShortcutItem::PersonalTag:
            case CWizCategoryViewShortcutItem::GroupTag:
            {
                if (shortcutItem->guid() == newItem->guid())
                {
                    delete newItem;
                    return nullptr;
                }
            }
                break;
            case CWizCategoryViewShortcutItem::PersonalFolder:
            {
                if (shortcutItem->location() == newItem->location())
                {
                    delete newItem;
                    return nullptr;
                }
            }
                break;
            default:
                continue;
            }
        }
    }

    //
    addChild(newItem);
    sortChildren(0, Qt::AscendingOrder);
    if (isContainsPlaceHoldItem())
        removePlaceHoldItem();

    return newItem;
}

CWizCategoryViewShortcutItem*CWizCategoryViewShortcutRootItem::addDocumentToShortcuts(const WIZDOCUMENTDATA& document)
{
    for (int i = 0; i < childCount(); i++)
    {
        CWizCategoryViewShortcutItem *pItem = dynamic_cast<CWizCategoryViewShortcutItem*>(child(i));
        if (pItem)
        {
            if (pItem->guid() == document.strGUID)
                return nullptr;
        }
    }

    if (isContainsPlaceHoldItem())
        removePlaceHoldItem();

    bool isEncrypted = document.nProtected == 1;
    CWizCategoryViewShortcutItem *pItem = new CWizCategoryViewShortcutItem(m_app,
                                                                           document.strTitle, CWizCategoryViewShortcutItem::Document,
                                                                           document.strKbGUID, document.strGUID, document.strLocation, isEncrypted);

    addChild(pItem);
    sortChildren(0, Qt::AscendingOrder);
    return pItem;
}

QString CWizCategoryViewShortcutRootItem::getSectionName()
{
    return WIZ_CATEGORY_SECTION_GENERAL;
}

void CWizCategoryViewShortcutRootItem::addPlaceHoldItem()
{
    CWizCategoryViewShortcutPlaceHoldItem *item = new
            CWizCategoryViewShortcutPlaceHoldItem(m_app, WIZ_CATEGORY_SHOTCUT_PLACEHOLD);
    item->setText(0, WIZ_CATEGORY_SHOTCUT_PLACEHOLD);
    addChild(item);
}

bool CWizCategoryViewShortcutRootItem::isContainsPlaceHoldItem()
{
    if (childCount() < 1)
        return false;

    QTreeWidgetItem *item = child(0);
    return item->text(0) == WIZ_CATEGORY_SHOTCUT_PLACEHOLD;
}

void CWizCategoryViewShortcutRootItem::removePlaceHoldItem()
{
    if (isContainsPlaceHoldItem())
    {
        removeChild(child(0));
    }
}


/* -------------------- CWizCategoryViewSearchRootItem -------------------- */
CWizCategoryViewSearchRootItem::CWizCategoryViewSearchRootItem(CWizExplorerApp& app,
                                                               const QString& strName)
    : CWizCategoryViewItemBase(app, strName, "", Category_QuickSearchRootItem)
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_search_normal"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_search_selected"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Selected);
    setIcon(0, icon);
    setText(0, strName);
}

void CWizCategoryViewSearchRootItem::showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
{
    if (CWizCategoryView* view = dynamic_cast<CWizCategoryView *>(pCtrl)) {
        view->showCustomSearchContextMenu(pos, false);
    }
}

QString CWizCategoryViewSearchRootItem::getSectionName()
{
    return WIZ_CATEGORY_SECTION_GENERAL;
}


/* ------------------------------ CWizCategoryViewAllFoldersItem ------------------------------ */

CWizCategoryViewAllFoldersItem::CWizCategoryViewAllFoldersItem(CWizExplorerApp& app,
                                                               const QString& strName,
                                                               const QString& strKbGUID)
    : CWizCategoryViewItemBase(app, strName, strKbGUID, Category_AllFoldersItem)
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_folders_normal"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_folders_selected"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Selected);
    setIcon(0, icon);
    setText(0, strName);
}

void CWizCategoryViewAllFoldersItem::getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
//    db.GetAllDocuments(arrayDocument);
    db.GetDocumentsBySQLWhere("DOCUMENT_LOCATION not like '/Deleted Items/%'", arrayDocument);
}

bool CWizCategoryViewAllFoldersItem::accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
{
    if (db.IsInDeletedItems(data.strLocation)) {
        return false;
    }

    return !db.IsGroup();
}

bool CWizCategoryViewAllFoldersItem::acceptDrop(const CWizCategoryViewItemBase* pItem) const
{
//    return pItem->type() == Category_FolderItem | pItem->type() == Category_GroupItem;

    const CWizCategoryViewFolderItem* item = dynamic_cast<const CWizCategoryViewFolderItem*>(pItem);
    return NULL != item;
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
    : CWizCategoryViewItemBase(app, strLocation, strKbGUID, Category_FolderItem)
{
    QIcon icon;
    if (::WizIsPredefinedLocation(strLocation) && strLocation == "/My Journals/") {
        icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_folder_diary_normal"),
                     Utils::StyleHelper::treeViewItemIconSize(), QIcon::Normal);
        icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_folder_diary_selected"),
                     Utils::StyleHelper::treeViewItemIconSize(), QIcon::Selected);
    } else {
        icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_folder_normal"),
                     Utils::StyleHelper::treeViewItemIconSize(), QIcon::Normal);
        icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_folder_selected"),
                     Utils::StyleHelper::treeViewItemIconSize(), QIcon::Selected);
    }
    setIcon(0, icon);
    setText(0, CWizDatabase::GetLocationDisplayName(strLocation));
}

QTreeWidgetItem* CWizCategoryViewFolderItem::clone() const
{
    return new CWizCategoryViewFolderItem(m_app, m_strName, m_strKbGUID);
}

QString CWizCategoryViewFolderItem::id() const
{
    return ::WizMd5StringNoSpaceJava(QString(m_strName + m_strKbGUID).toUtf8());
}

void CWizCategoryViewFolderItem::setLocation(const QString& strLocation)
{
    m_strName = strLocation;
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

    if (kbGUID().isEmpty() && !db.IsGroup())
        return true;

    return false;
}

bool CWizCategoryViewFolderItem::acceptDrop(const WIZDOCUMENTDATA& data) const
{
    Q_UNUSED(data);

    return true;
}

bool CWizCategoryViewFolderItem::acceptDrop(const CWizCategoryViewItemBase* pItem) const
{
//    return pItem->type() == Category_FolderItem | pItem->type() == Category_GroupItem;

    const CWizCategoryViewFolderItem* item = dynamic_cast<const CWizCategoryViewFolderItem*>(pItem);
    return NULL != item;
}

void CWizCategoryViewFolderItem::drop(const CWizDocumentDataArray& arrayDocument, bool forceCopy)
{
    CWizDocumentDataArray arrayOp;
    bool needCopy = false;
    for (WIZDOCUMENTDATAEX doc : arrayDocument)
    {
        if (!acceptDrop(doc))
            continue;

        arrayOp.push_back(doc);
        if (forceCopy || kbGUID() != doc.strKbGUID)
        {
            needCopy = true;
        }
    }

    if (arrayOp.empty())
        return;

    Internal::MainWindow* window = qobject_cast<Internal::MainWindow *>(m_app.mainWindow());
    CWizDocumentOperator* documentOperator = new CWizDocumentOperator(m_app.databaseManager());
    CWizProgressDialog* progress = window->progressDialog();
    documentOperator->bindSignalsToProgressDialog(progress);
    if (needCopy)
    {
        progress->setWindowTitle(QObject::tr("Copy note to %1").arg(location()));
        documentOperator->copyDocumentsToPersonalFolder(arrayOp, location(), false, true, window->downloaderHost());
        progress->exec();
    }
    else
    {
        progress->setWindowTitle(QObject::tr("Move note to %1").arg(location()));
        documentOperator->moveDocumentsToPersonalFolder(arrayOp, location(), window->downloaderHost());
        progress->exec();
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

//    qDebug() << "compare, this : " << name() << " , other : " << pOther->name();

    if (getSortOrder() != pOther->getSortOrder())
    {
        bool result  = getSortOrder() < pOther->getSortOrder();
//        qDebug() << "sortoder different : " << result;
        return result;
    }

    // sort by folder pos
    if (m_app.userSettings().isManualSortingEnabled())
    {
        int nThis = 0, nOther = 0;
        if (!pOther->location().isEmpty()) {
            QSettings* setting = ExtensionSystem::PluginManager::settings();
//            qDebug() << "pother location : " << pOther->location() << "  this location : " << location();
            nOther = setting->value("FolderPosition/" + pOther->location()).toInt();
            nThis = setting->value("FolderPosition/" + location()).toInt();
        }

//        qDebug() << "manual sort enable, this folder pos : " << nThis << "  other sort pos : " << nOther;

        if (nThis != nOther)
        {
            if (nThis > 0 && nOther > 0)
            {
                bool result  =  nThis < nOther;
//                qDebug() << "folder position different : " << result;
                return result;
            }
        }
    }

    //
    return CWizCategoryViewItemBase::operator <(other);
}



/* ------------------------------ CWizCategoryViewAllTagsItem ------------------------------ */

CWizCategoryViewAllTagsItem::CWizCategoryViewAllTagsItem(CWizExplorerApp& app,
                                                         const QString& strName,
                                                         const QString& strKbGUID)
    : CWizCategoryViewItemBase(app, strName, strKbGUID, Category_AllTagsItem)
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_tags_normal"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_tags_selected"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Selected);
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

bool CWizCategoryViewAllTagsItem::acceptDrop(const CWizCategoryViewItemBase* pItem) const
{
    const CWizCategoryViewTagItem* item = dynamic_cast<const CWizCategoryViewTagItem*>(pItem);
    return NULL != item;
}

QString CWizCategoryViewAllTagsItem::getSectionName()
{
    return WIZ_CATEGORY_SECTION_PERSONAL;
}

/* ------------------------------ CWizCategoryViewTagItem ------------------------------ */

CWizCategoryViewTagItem::CWizCategoryViewTagItem(CWizExplorerApp& app,
                                                 const WIZTAGDATA& tag,
                                                 const QString& strKbGUID)
    : CWizCategoryViewItemBase(app, tag.strName, strKbGUID, Category_TagItem)
    , m_tag(tag)
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_tagItem_normal"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_tagItem_selected"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Selected);
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

bool CWizCategoryViewTagItem::acceptDrop(const CWizCategoryViewItemBase* pItem) const
{
    return pItem && pItem->type() == Category_TagItem;
}

void CWizCategoryViewTagItem::drop(const CWizDocumentDataArray& arrayDocument, bool forceCopy)
{
    Q_UNUSED(forceCopy);

    CWizDatabase& db = CWizDatabaseManager::instance()->db(kbGUID());
    for (WIZDOCUMENTDATA document : arrayDocument)
    {
        if (!acceptDrop(document))
            continue;

        // skip
        QString strTagGUIDs = db.GetDocumentTagGUIDsString(document.strGUID);
        if (strTagGUIDs.indexOf(m_tag.strGUID) != -1)
            continue;

        CWizDocument doc(db, document);
        doc.AddTag(tag());
    }
}

void CWizCategoryViewTagItem::drop(const CWizCategoryViewItemBase* pItem)
{
    if (pItem && pItem->type() == Category_TagItem)
    {
        const CWizCategoryViewTagItem* childItem = dynamic_cast<const CWizCategoryViewTagItem*>(pItem);
        WIZTAGDATA childTag = childItem->tag();
        childTag.strParentGUID = tag().strGUID;
        m_app.databaseManager().db().ModifyTag(childTag);
    }
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
    m_strName = m_tag.strName;
}

void CWizCategoryViewTagItem::setTagPosition(int nPos)
{
    m_tag.nPostion = nPos;
}


/* --------------------- CWizCategoryViewStyleRootItem --------------------- */
CWizCategoryViewStyleRootItem::CWizCategoryViewStyleRootItem(CWizExplorerApp& app,
                                                             const QString& strName)
    : CWizCategoryViewItemBase(app, strName)
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "style_normal"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "style_selected"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Selected);
    setIcon(0, icon);
    setText(0, strName);
}

QString CWizCategoryViewStyleRootItem::getSectionName()
{
    return WIZ_CATEGORY_SECTION_PERSONAL;
}

/* ---------------------------- CWizCategoryViewGroupsRootItem ---------------------------- */

CWizCategoryViewGroupsRootItem::CWizCategoryViewGroupsRootItem(CWizExplorerApp& app, const QString& strName)
    : CWizCategoryViewItemBase(app, strName, "", Category_GroupsRootItem)
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_group_normal"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_group_selected"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Selected);
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

//bool CWizCategoryViewGroupsRootItem::acceptDrop(const CWizCategoryViewItemBase* pItem) const
//{
//    const CWizCategoryViewGroupItem* item = dynamic_cast<const CWizCategoryViewGroupItem*>(pItem);
//    return NULL != item;
//}
QString CWizCategoryViewGroupsRootItem::getSectionName()
{
    return WIZ_CATEGORY_SECTION_GROUPS;
}


/* ------------------------------ CWizCategoryViewGroupRootItem ------------------------------ */
CWizCategoryViewBizGroupRootItem::CWizCategoryViewBizGroupRootItem(CWizExplorerApp& app,
                                                                   const WIZBIZDATA& biz)
    : CWizCategoryViewGroupsRootItem(app, biz.bizName)
    , m_biz(biz)
    , m_unReadCount(0)
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_biz_normal"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_biz_selected"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Selected);
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

void CWizCategoryViewBizGroupRootItem::getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    if (isUnreadButtonUseable() && hitTestUnread())
    {
        for (int i = 0; i < childCount(); i++)
        {
            CWizCategoryViewGroupRootItem* pGroup = dynamic_cast<CWizCategoryViewGroupRootItem*>(child(i));
            Q_ASSERT(pGroup);
            if (!pGroup)
                return;

            CWizDatabase& db = CWizDatabaseManager::instance()->db(pGroup->kbGUID());

            CWizDocumentDataArray arrayDoc;
            if (db.getGroupUnreadDocuments(arrayDoc))
            {
                arrayDocument.insert(arrayDocument.begin(), arrayDoc.begin(), arrayDoc.end());
            }
        }
        updateUnreadCount();
    }
    else
    {
        CWizCategoryViewGroupsRootItem::getDocuments(db, arrayDocument);
    }
}

void CWizCategoryViewBizGroupRootItem::drawExtraBadge(QPainter* p, const QStyleOptionViewItemV4* vopt) const
{
    if (isUnreadButtonUseable())
    {
        //
        QString text = unreadString();
        if (text.isEmpty())
            return;

        p->save();
        //
        QRect rcb = getExtraButtonRect(vopt->rect, true);
        p->setRenderHint(QPainter::Antialiasing);
        drawClickableUnreadButton(p, rcb, text, m_extraButtonIconPressed);
        //
        p->restore();
    }
    else
    {
        CWizCategoryViewGroupsRootItem::drawExtraBadge(p, vopt);
    }
}

void CWizCategoryViewBizGroupRootItem::mousePressed(const QPoint& pos)
{
    QRect rcBorder = treeWidget()->visualItemRect(this);
    QRect rcIcon = getExtraButtonRect(rcBorder, true);
    if (rcIcon.contains(pos))
    {
        m_extraButtonIconPressed = true;
    }
}

void CWizCategoryViewBizGroupRootItem::mouseReleased(const QPoint& pos)
{
    m_extraButtonIconPressed = false;
}

QString CWizCategoryViewBizGroupRootItem::getExtraButtonToolTip() const
{
    if (m_unReadCount > 0 && isUnreadButtonUseable())
        return QObject::tr("You have %1 unread notes").arg(m_unReadCount);

    if (m_extraButtonIcon.isNull() || !isExtraButtonUseable())
        return "";

    return QObject::tr("Your enterprise services has expired");
}

QRect CWizCategoryViewBizGroupRootItem::getExtraButtonRect(const QRect& itemBorder, bool ignoreIconExist) const
{
    if (!isUnreadButtonUseable())
        return CWizCategoryViewItemBase::getExtraButtonRect(itemBorder, ignoreIconExist);

    if (!m_unReadCount && !ignoreIconExist)
        return QRect();

    int nButtonWidth = 26;
    int nButtonHeight = 14;
    QRect rc(itemBorder.right() - EXTRABUTTONRIGHTMARGIN - nButtonWidth, itemBorder.y() + (itemBorder.height() - nButtonHeight) / 2,
             nButtonWidth, nButtonHeight);
    return rc;
}

bool CWizCategoryViewBizGroupRootItem::isExtraButtonUseable() const
{
    return !isUnreadButtonUseable();
}

bool CWizCategoryViewBizGroupRootItem::isUnreadButtonUseable() const
{
     bool bUseable = (!isExpanded()) && (m_unReadCount > 0);
     return bUseable;
}

void CWizCategoryViewBizGroupRootItem::updateUnreadCount()
{
    m_unReadCount = 0;
    int nChildCount = childCount();
    for (int i = 0; i < nChildCount; i++)
    {
        CWizCategoryViewGroupRootItem* childItem = dynamic_cast<CWizCategoryViewGroupRootItem*>(child(i));
        if (childItem)
        {
            m_unReadCount += childItem->getUnreadCount();
        }
    }

    CWizCategoryBaseView* view = dynamic_cast<CWizCategoryBaseView*>(treeWidget());
    if (m_unReadCount > 0)
    {
        //
        QFont f;
        Utils::StyleHelper::fontNormal(f);
        QFontMetrics fm(f);
        //
        QSize szText = fm.size(0, unreadString());
        int textWidth = szText.width();
//        int textHeight = szText.height();
        //
        //int nMargin = textHeight / 4;
        //
        int nWidth = textWidth  + nNumberButtonHorizontalMargin * 2;
        int nHeight = nNumberButtonHeight;// textHeight + 2;
        if (nWidth < nHeight)
            nWidth = nHeight;
        //
        Q_ASSERT(view);

        // use parent height, group root could be unvisible
        QRect rcIemBorder = view->visualItemRect(this);
        QRect rcExtButton = getExtraButtonRect(rcIemBorder, true);
        //
        int nTop = rcIemBorder.y() + (rcIemBorder.height() - nHeight) / 2;
        int nLeft = rcExtButton.right() - nWidth;
        QRect rcb(nLeft, nTop, nWidth, nHeight);

        m_szUnreadSize = rcb.size();
    }

    view->updateItem(this);
}

QString CWizCategoryViewBizGroupRootItem::unreadString() const
{
    return unreadNumToString(m_unReadCount);
}

bool CWizCategoryViewBizGroupRootItem::hitTestUnread()
{
    CWizCategoryBaseView* view = dynamic_cast<CWizCategoryBaseView*>(treeWidget());
    Q_ASSERT(view);

    QRect rcItem = view->visualItemRect(this);
    QPoint pt = view->hitPoint();
    //
    int nMargin = 4;
    QRect rcRect = getExtraButtonRect(rcItem, true);
    QRect rcb = QRect(rcRect.right() - m_szUnreadSize.width() + 1, rcRect.y() + (rcRect.height() - m_szUnreadSize.height())/2,
                      m_szUnreadSize.width(), m_szUnreadSize.height());
    rcb.adjust(-nMargin, -nMargin, nMargin, nMargin);

    return rcb.contains(pt);
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
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_group_normal"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_group_selected"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Selected);
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
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_group_normal"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_group_selected"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Selected);
    setIcon(0, icon);
}


QString CWizCategoryViewCreateGroupLinkItem::getSectionName()
{
    return WIZ_CATEGORY_SECTION_GROUPS;
}

/* ------------------------------ CWizCategoryViewGroupRootItem ------------------------------ */

CWizCategoryViewGroupRootItem::CWizCategoryViewGroupRootItem(CWizExplorerApp& app,
                                                             const WIZGROUPDATA& group)
    : CWizCategoryViewItemBase(app, group.strGroupName, group.strGroupGUID, Category_GroupRootItem)
    , m_group(group)
    , m_nUnread(0)
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_group_normal"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_group_selected"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Selected);
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
    if (hitTestUnread() && m_nUnread)
    {
        db.getGroupUnreadDocuments(arrayDocument);
    }
    else
    {
        db.getLastestDocuments(arrayDocument);
    }
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

bool CWizCategoryViewGroupRootItem::acceptDrop(const CWizCategoryViewItemBase* pItem) const
{
//    return pItem->type() == Category_FolderItem | pItem->type() == Category_GroupItem;

    const CWizCategoryViewGroupItem* item = dynamic_cast<const CWizCategoryViewGroupItem*>(pItem);
    return item && item->kbGUID() == kbGUID();
}

bool CWizCategoryViewGroupRootItem::acceptDrop(const QString& urls) const
{
    Q_UNUSED(urls);
    CWizDatabase& db = m_app.databaseManager().db(kbGUID());

    return WIZ_USERGROUP_AUTHOR >= db.permission();
}

void CWizCategoryViewGroupRootItem::drop(const CWizDocumentDataArray& arrayDocument, bool forceCopy)
{
    CWizDocumentDataArray arrayOp;
    bool needCopy = false;
    for (WIZDOCUMENTDATA doc : arrayDocument)
    {
        if (!acceptDrop(doc))
            continue;

        if (forceCopy || doc.strKbGUID != m_strKbGUID)
            needCopy = true;

        arrayOp.push_back(doc);
    }

    if (arrayOp.empty())
        return;

    Internal::MainWindow* window = qobject_cast<Internal::MainWindow *>(m_app.mainWindow());
    CWizDocumentOperator* documentOperator = new CWizDocumentOperator(m_app.databaseManager());
    CWizProgressDialog* progress = window->progressDialog();
    documentOperator->bindSignalsToProgressDialog(progress);

    if (needCopy)
    {
        progress->setWindowTitle(QObject::tr("Copy note to %1").arg(name()));
        WIZTAGDATA tag;
        tag.strKbGUID = m_strKbGUID;
        documentOperator->copyDocumentsToGroupFolder(arrayOp, tag, false, window->downloaderHost());
        progress->exec();
    }
    else
    {
        progress->setWindowTitle(QObject::tr("Move note to %1").arg(name()));
        WIZTAGDATA tag;
        tag.strKbGUID = m_strKbGUID;
        documentOperator->moveDocumentsToGroupFolder(arrayOp, tag, window->downloaderHost());
        progress->exec();
    }
}

void CWizCategoryViewGroupRootItem::drawExtraBadge(QPainter* p, const QStyleOptionViewItemV4* vopt) const
{
    //
    if (m_nUnread > 0)
    {
        QString text = unreadString();
        p->save();
        //
        QRect rcb = getExtraButtonRect(vopt->rect, true);
        p->setRenderHint(QPainter::Antialiasing);
        drawClickableUnreadButton(p, rcb, text, m_extraButtonIconPressed);
        //
        p->restore();
    }
    else
    {
        CWizCategoryViewItemBase::drawExtraBadge(p, vopt);
    }
}

void CWizCategoryViewGroupRootItem::reload(CWizDatabase& db)
{
    m_strName = db.name();
    setText(0, db.name());
}

void CWizCategoryViewGroupRootItem::mousePressed(const QPoint& pos)
{
    QRect rcBorder = treeWidget()->visualItemRect(this);
    QRect rcIcon = getExtraButtonRect(rcBorder, true);
    if (rcIcon.contains(pos))
    {
        m_extraButtonIconPressed = true;
    }
}

void CWizCategoryViewGroupRootItem::mouseReleased(const QPoint& pos)
{
    m_extraButtonIconPressed = false;
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

void CWizCategoryViewGroupRootItem::setUnreadCount(int nCount)
{
    m_nUnread = nCount;
    CWizCategoryBaseView* view = dynamic_cast<CWizCategoryBaseView*>(treeWidget());
    //
    if (m_nUnread > 0)
    {
        QFont f;
        Utils::StyleHelper::fontNormal(f);
        QFontMetrics fm(f);
        //
        QSize szText = fm.size(0, unreadString());
        int textWidth = szText.width();
//        int textHeight = szText.height();
        //
        //int nMargin = textHeight / 4;
        //
        int nWidth = textWidth + nNumberButtonHorizontalMargin * 2;
        int nHeight = nNumberButtonHeight;//  textHeight + 2;
        if (nWidth < nHeight)
            nWidth = nHeight;
        //

        Q_ASSERT(view);

        // use parent height, group root could be unvisible
        QRect rcIemBorder = view->visualItemRect(this->parent());
        QRect rcExtButton = getExtraButtonRect(rcIemBorder, true);
        //
        int nTop = rcIemBorder.y() + (rcIemBorder.height() - nHeight) / 2;
        int nLeft = rcExtButton.right() - nWidth;
        QRect rcb(nLeft, nTop, nWidth, nHeight);

        m_szUnreadSize = rcb.size();
    }

    view->updateItem(this);
}

int CWizCategoryViewGroupRootItem::getUnreadCount()
{
    return m_nUnread;
}

QString CWizCategoryViewGroupRootItem::unreadString() const
{
    return unreadNumToString(m_nUnread);
}

bool CWizCategoryViewGroupRootItem::hitTestUnread()
{
    CWizCategoryBaseView* view = dynamic_cast<CWizCategoryBaseView*>(treeWidget());
    Q_ASSERT(view);

    QRect rcItem = view->visualItemRect(this);
    QPoint pt = view->hitPoint();
    //
    int nMargin = 4;
    QRect rcRect = getExtraButtonRect(rcItem, true);
    QRect rcb = QRect(rcRect.right() - m_szUnreadSize.width() + 1, rcRect.y() + (rcRect.height() - m_szUnreadSize.height())/2,
                      m_szUnreadSize.width(), m_szUnreadSize.height());
    rcb.adjust(-nMargin, -nMargin, nMargin, nMargin);

    return rcb.contains(pt);
}

QString CWizCategoryViewGroupRootItem::getExtraButtonToolTip() const
{
    if (m_nUnread > 0)
        return QObject::tr("You have %1 unread notes").arg(m_nUnread);

    if (m_extraButtonIcon.isNull())
        return "";

    return QObject::tr("Your group is in the abnormal state");
}

QRect CWizCategoryViewGroupRootItem::getExtraButtonRect(const QRect& itemBorder, bool ignoreIconExist) const
{
    if (m_nUnread == 0)
        return CWizCategoryViewItemBase::getExtraButtonRect(itemBorder, ignoreIconExist);

    if (!m_nUnread && !ignoreIconExist)
        return QRect();

    int nButtonWidth = 26;
    int nButtonHeight = 14;
    QRect rc(itemBorder.right() - EXTRABUTTONRIGHTMARGIN - nButtonWidth, itemBorder.y() + (itemBorder.height() - nButtonHeight) / 2,
             nButtonWidth, nButtonHeight);
    return rc;
}

/* --------------------- CWizCategoryViewGroupNoTagItem --------------------- */
CWizCategoryViewGroupNoTagItem::CWizCategoryViewGroupNoTagItem(CWizExplorerApp& app,
                                                               const QString& strKbGUID)
    : CWizCategoryViewItemBase(app, PREDEFINED_UNCLASSIFIED, strKbGUID, Category_GroupNoTagItem)
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_unclassified_normal"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_unclassified_selected"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Selected);
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
    if (kbGUID() != data.strKbGUID)
        return false;

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
    : CWizCategoryViewItemBase(app, tag.strName, strKbGUID, Category_GroupItem)
    , m_tag(tag)
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_folder_normal"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_folder_selected"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Selected);
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

bool CWizCategoryViewGroupItem::acceptDrop(const CWizCategoryViewItemBase* pItem) const
{
//    return pItem->type() == Category_FolderItem | pItem->type() == Category_GroupItem;

    return pItem->kbGUID() == kbGUID();
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

bool CWizCategoryViewGroupItem::acceptDrop(const QString& urls) const
{
    Q_UNUSED(urls);
    CWizDatabase& db = m_app.databaseManager().db(kbGUID());

    return WIZ_USERGROUP_AUTHOR >= db.permission();
}

void CWizCategoryViewGroupItem::drop(const CWizDocumentDataArray& arrayDocument, bool forceCopy)
{
    CWizDocumentDataArray arrayOp;
    bool needCopy = false;
    for (WIZDOCUMENTDATA doc : arrayDocument)
    {
        if (!acceptDrop(doc))
            continue;

        if (forceCopy || doc.strKbGUID != m_strKbGUID)
            needCopy = true;

        arrayOp.push_back(doc);
    }

    if (arrayOp.empty())
        return;

    Internal::MainWindow* window = qobject_cast<Internal::MainWindow *>(m_app.mainWindow());
    CWizDocumentOperator* documentOperator = new CWizDocumentOperator(m_app.databaseManager());
    CWizProgressDialog* progress = window->progressDialog();
    documentOperator->bindSignalsToProgressDialog(progress);

    if (needCopy)
    {
        progress->setWindowTitle(QObject::tr("Copy note to %1").arg(name()));
        documentOperator->copyDocumentsToGroupFolder(arrayOp, m_tag, false, window->downloaderHost());
        progress->exec();
    }
    else
    {
        progress->setWindowTitle(QObject::tr("Move note to %1").arg(name()));
        documentOperator->moveDocumentsToGroupFolder(arrayOp, m_tag, window->downloaderHost());
        progress->exec();
    }
}

QString CWizCategoryViewGroupItem::id() const
{
    return ::WizMd5StringNoSpaceJava(QString(text(0) + m_tag.strGUID).toUtf8());
}

bool CWizCategoryViewGroupItem::operator<(const QTreeWidgetItem& other) const
{
    if (m_app.userSettings().isManualSortingEnabled())
    {
        const CWizCategoryViewGroupItem* pOther = dynamic_cast<const CWizCategoryViewGroupItem*>(&other);
        if (pOther)
        {
            if (m_tag.nPostion == pOther->m_tag.nPostion || m_tag.nPostion == 0 || pOther->m_tag.nPostion == 0)
                return CWizCategoryViewItemBase::operator <(other);

            return m_tag.nPostion < pOther->m_tag.nPostion;
        }
    }

    return CWizCategoryViewItemBase::operator <(other);
}

void CWizCategoryViewGroupItem::reload(CWizDatabase& db)
{
    db.TagFromGUID(m_tag.strGUID, m_tag);
    setText(0, m_tag.strName);
    m_strName = m_tag.strName;
}

void CWizCategoryViewGroupItem::setTagPosition(int nPos)
{
    m_tag.nPostion = nPos;
}

/* ------------------------------ CWizCategoryViewTrashItem ------------------------------ */

CWizCategoryViewTrashItem::CWizCategoryViewTrashItem(CWizExplorerApp& app,
                                                     const QString& strKbGUID)
    : CWizCategoryViewFolderItem(app, "/Deleted Items/", strKbGUID)
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "trash_normal"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "trash_selected"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Selected);
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

bool CWizCategoryViewTrashItem::acceptDrop(const CWizCategoryViewItemBase* pItem) const
{
    return false;
}

void CWizCategoryViewTrashItem::drop(const CWizDocumentDataArray& arrayDocument, bool forceCopy)
{
    CWizDocumentDataArray arrayOp;
    for (WIZDOCUMENTDATA doc : arrayDocument)
    {
        if (!acceptDrop(doc))
            continue;

        arrayOp.push_back(doc);
    }

    if (arrayOp.empty())
        return;

    Internal::MainWindow* window = qobject_cast<Internal::MainWindow *>(m_app.mainWindow());
    CWizDocumentOperator* documentOperator = new CWizDocumentOperator(m_app.databaseManager());
    CWizProgressDialog* progress = window->progressDialog();
    documentOperator->bindSignalsToProgressDialog(progress);
    progress->setWindowTitle(QObject::tr("Delete note"));
    documentOperator->deleteDocuments(arrayOp);
    progress->exec();
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


CWizCategoryViewShortcutItem::CWizCategoryViewShortcutItem(CWizExplorerApp& app,
                                                           const QString& strName, ShortcutType type, const QString& strKbGuid,
                                                           const QString& strGuid, const QString& location, bool bEncrypted)
    : CWizCategoryViewItemBase(app, strName, strKbGuid, Category_ShortcutItem)
    , m_strGuid(strGuid)
    , m_type(type)
    , m_location(location)
{
    QIcon icon;
    switch (type) {
    case Document:
    {
        if (bEncrypted)
        {
            icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "document_badge_encrypted"),
                         Utils::StyleHelper::treeViewItemIconSize(), QIcon::Normal);
            icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "document_badge_encrypted_selected"),
                         Utils::StyleHelper::treeViewItemIconSize(), QIcon::Selected);
        }
        else
        {
            icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "document_badge"),
                         Utils::StyleHelper::treeViewItemIconSize(), QIcon::Normal);
            icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "document_badge_selected"),
                         Utils::StyleHelper::treeViewItemIconSize(), QIcon::Selected);
        }
    }
        break;
    case PersonalFolder:
    case GroupTag:
    {
        icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_folder_normal"),
                     Utils::StyleHelper::treeViewItemIconSize(), QIcon::Normal);
        icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_folder_selected"),
                     Utils::StyleHelper::treeViewItemIconSize(), QIcon::Selected);
    }
        break;
    case PersonalTag:
    {
        icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_tag_normal"),
                     Utils::StyleHelper::treeViewItemIconSize(), QIcon::Normal);
        icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_tag_selected"),
                     Utils::StyleHelper::treeViewItemIconSize(), QIcon::Selected);
    }
        break;
    }

    //
    setIcon(0, icon);
    setText(0, strName);
}

void CWizCategoryViewShortcutItem::showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
{
    if (CWizCategoryView* view = dynamic_cast<CWizCategoryView *>(pCtrl)) {
        view->showShortcutContextMenu(pos);
    }
}

bool CWizCategoryViewShortcutItem::accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
{
    switch (m_type) {
    case Document:
        return data.strGUID == m_strGuid;
        break;
    case PersonalFolder:
        return data.strLocation == m_location;
        break;
    case PersonalTag:
    {
        CWizStdStringArray arrayTag;
        m_app.databaseManager().db().GetDocumentTags(data.strGUID, arrayTag);
        for (CString tag : arrayTag)
        {
            if (tag == m_strGuid)
                return true;
        }
    }
        break;
    case GroupTag:
    {
        if (data.strKbGUID != m_strKbGUID)
            return false;

        CWizStdStringArray arrayTag;
        m_app.databaseManager().db(data.strKbGUID).GetDocumentTags(data.strGUID, arrayTag);
        for (CString tag : arrayTag)
        {
            if (tag == m_strGuid)
                return true;
        }
    }
        break;
    default:
        break;
    }
    return false;
}


CWizCategoryViewShortcutPlaceHoldItem::CWizCategoryViewShortcutPlaceHoldItem(
        CWizExplorerApp& app, const QString& strName)
    : CWizCategoryViewItemBase(app, strName, "", Category_ShortcutPlaceHoldItem)
{

}

int CWizCategoryViewShortcutPlaceHoldItem::getItemHeight(int /*hintHeight*/) const
{
    return 20;
}

void CWizCategoryViewShortcutPlaceHoldItem::drawItemBody(QPainter *p, const QStyleOptionViewItemV4 *vopt) const
{
    QRect rcIcon = treeWidget()->style()->subElementRect(QStyle::SE_ItemViewItemDecoration, vopt, treeWidget());
    QRect rcText(rcIcon.right() + 8, vopt->rect.top(), vopt->rect.right() - rcIcon.right() - 24,
                 vopt->rect.height());

    QString strText = vopt->text;
    if (!strText.isEmpty()) {
        bool isSelected = vopt->state & QStyle::State_Selected;
        QColor colorText(isSelected ? "#FFFFFF" : "#BEBEBE");
        colorText.setAlpha(240);
        p->setPen(colorText);
        QFont f;
        f.setPixelSize(10);
        f.setStyleStrategy(QFont::PreferBitmap);
        QFontMetrics fm(f);
        strText = fm.elidedText(strText, Qt::ElideRight, rcText.width());
        int right = Utils::StyleHelper::drawSingleLineText(p, rcText, strText, Qt::AlignVCenter, colorText, f);
        //
        if (right != -1) {
            rcText.setLeft(right + 4);
        }
    }
}


CWizCategoryViewSearchItem::CWizCategoryViewSearchItem(CWizExplorerApp& app,
                                                       const QString& strName, int type)
    : CWizCategoryViewItemBase(app, strName, "", type)
{
    QIcon icon;
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_searchItem_normal"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Normal);
    icon.addFile(WizGetSkinResourceFileName(app.userSettings().skin(), "category_searchItem_selected"),
                 Utils::StyleHelper::treeViewItemIconSize(), QIcon::Selected);
    setIcon(0, icon);
    setText(0, strName);    
}

void CWizCategoryViewSearchItem::showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
{
    if (CWizCategoryView* view = dynamic_cast<CWizCategoryView *>(pCtrl)) {
        view->showCustomSearchContextMenu(pos, false);
    }
}

bool CWizCategoryViewTimeSearchItem::operator<(const QTreeWidgetItem& other) const
{
    const CWizCategoryViewTimeSearchItem* pOther = dynamic_cast<const CWizCategoryViewTimeSearchItem*>(&other);
    if (!pOther) {
        return false;
    }

    return m_dateInterval < pOther->m_dateInterval;
}


CWizCategoryViewTimeSearchItem::CWizCategoryViewTimeSearchItem(CWizExplorerApp& app,
                                                               const QString& strName, const QString strSelectParam, DateInterval interval)
    : CWizCategoryViewSearchItem(app, strName)
    , m_dateInterval(interval)
    , m_strSelectParam(strSelectParam)
{
}

QString CWizCategoryViewTimeSearchItem::getSQLWhere()
{
    COleDateTime dt;
    switch (m_dateInterval) {
    case DateInterval_Today:
        dt = dt.addDays(-1);
        break;
    case DateInterval_Yestoday:
        dt = dt.addDays(-2);
        break;
    case DateInterval_TheDayBeforeYestoday:
        dt = dt.addDays(-3);
        break;
    case DateInterval_LastWeek:
        dt = dt.addDays(-8);
        break;
    case DateInterval_LastMonth:
        dt = dt.addMonths(-1);
        break;
    case DateInterval_LastYear:
        dt = dt.addYears(-1);
        break;
    default:
        break;
    }
    QString str = m_strSelectParam;
    str.replace("%1", TIME2SQL(dt));
    return str;
}


CWizCategoryViewCustomSearchItem::CWizCategoryViewCustomSearchItem(CWizExplorerApp& app,
                                                                   const QString& strName, const QString strSelectParam,
                                                                   const QString strSqlWhere, const QString& strGuid,
                                                                   const QString& keyword, int searchScope)
    : CWizCategoryViewSearchItem(app, strName, Category_QuickSearchCustomItem)
    , m_strSelectParam(strSelectParam)
    , m_strSQLWhere(strSqlWhere)
    , m_strKeywrod(keyword)
    , m_nSearchScope(searchScope)
{
    m_strKbGUID = strGuid;
}

void CWizCategoryViewCustomSearchItem::showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos)
{
    if (CWizCategoryView* view = dynamic_cast<CWizCategoryView *>(pCtrl)) {
        view->showCustomSearchContextMenu(pos, true);
    }
}

QString CWizCategoryViewCustomSearchItem::getSQLWhere()
{
    return m_strSQLWhere;
}

void CWizCategoryViewCustomSearchItem::setSQLWhere(const QString& strSql)
{
    m_strSQLWhere = strSql;
}

QString CWizCategoryViewCustomSearchItem::getSelectParam()
{
    return m_strSelectParam;
}

void CWizCategoryViewCustomSearchItem::setSelectParam(const QString& strParam)
{
    m_strSelectParam = strParam;
}

void CWizCategoryViewCustomSearchItem::setKeyword(const QString& strKeyword)
{
    m_strKeywrod = strKeyword;
}

QString CWizCategoryViewCustomSearchItem::getKeyword()
{
    return m_strKeywrod;
}
int CWizCategoryViewCustomSearchItem::searchScope() const
{
    return m_nSearchScope;
}

void CWizCategoryViewCustomSearchItem::setSearchScope(int nSearchScope)
{
    m_nSearchScope = nSearchScope;
}

void CWizCategoryViewLinkItem::drawItemBody(QPainter *p, const QStyleOptionViewItemV4 *vopt) const
{
    QString str = vopt->text;
    QRect rc(vopt->rect);
    rc.setLeft(rc.left() + 16);
    QFont fontLink = p->font();
    fontLink.setItalic(true);
    fontLink.setPixelSize(12);
    Utils::StyleHelper::drawSingleLineText(p, rc, str, Qt::AlignTop, Utils::StyleHelper::treeViewItemLinkText(), fontLink);
}
