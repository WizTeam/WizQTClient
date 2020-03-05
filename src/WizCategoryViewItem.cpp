#include "WizCategoryViewItem.h"

#include <QTextCodec>
#include <QPainter>
#include <cstring>
#include <QFile>
#include <QStyle>
#include <QDebug>

#include "utils/WizPinyin.h"
#include "utils/WizStyleHelper.h"
#include "utils/WizNotify.h"
#include "utils/WizLogger.h"
#include "utils/WizMisc.h"

#include "WizCategoryView.h"
#include "WizMainWindow.h"
#include "WizDocumentTransitionView.h"
#include "share/WizObjectDataDownloader.h"
#include "share/WizObjectOperator.h"
#include "WizProgressDialog.h"
#include "widgets/WizTipsWidget.h"

#include "WizDef.h"
#include "WizNoteStyle.h"

#include "share/WizSettings.h"
#include "share/WizGlobal.h"
#include "share/WizDatabaseManager.h"
#include "share/WizMisc.h"

#define PREDEFINED_TRASH            QObject::tr("Trash")
#define PREDEFINED_UNCLASSIFIED     QObject::tr("Unclassified")

#define CATEGORY_OWN_GROUPS      QObject::tr("My Groups")
#define CATEGORY_OTHER_GROUPS      QObject::tr("Other Groups")


#define WIZ_CATEGORY_SECTION_GENERAL QObject::tr("General")
#define WIZ_CATEGORY_SECTION_PERSONAL QObject::tr("Personal Notes")
#define WIZ_CATEGORY_SECTION_GROUPS QObject::tr("Team Notes")

#define WIZ_CATEGORY_SHOTCUT_PLACEHOLD QObject::tr("Drag note form note list")


const int nNumberButtonHeight = 14;
const int nNumberButtonHorizontalMargin = 3;

static const WizIconOptions ICON_OPTIONS("#FFFFFF", "#FFFFFF", "#FFFFFF");


/* ------------------------------ CWizCategoryViewItemBase ------------------------------ */

WizCategoryViewItemBase::WizCategoryViewItemBase(WizExplorerApp& app,
                                                   const QString& strName,
                                                   const QString& strKbGUID, int type)
    : QTreeWidgetItem(type)
    , m_app(app)
    , m_strName(strName)
    , m_strKbGUID(strKbGUID)
    , m_extraButtonIconPressed(false)
    , m_bWillBeDeleted(false)
{
}

void WizCategoryViewItemBase::drawItemBody(QPainter *p, const QStyleOptionViewItem *vopt) const
{
    bool bSelected = vopt->state.testFlag(QStyle::State_Selected);

    QRect rcIcon = treeWidget()->style()->subElementRect(QStyle::SE_ItemViewItemDecoration, vopt, treeWidget());
    if (!vopt->icon.isNull()) {
        const int iconSize = WizSmartScaleUI(16);
        rcIcon.adjust(-6, 0, 0, 0);
        rcIcon.setTop(vopt->rect.top() + (vopt->rect.height() - iconSize) / 2);
        rcIcon.setWidth(iconSize);
        rcIcon.setHeight(iconSize);
        Utils::WizStyleHelper::drawTreeViewItemIcon(p, rcIcon, vopt->icon, bSelected);
    }

    QFont f;
    Utils::WizStyleHelper::fontCategoryItem(f);

    QFont fontCount;
    Utils::WizStyleHelper::fontExtend(fontCount);

    //通过subElementRect获取范围会产生不同的结果。此处通过icon进行计算
//    QRect rcText = subElementRect(SE_ItemViewItemText, vopt, view);
    QRect rcText(rcIcon.right() + 8, vopt->rect.top(), vopt->rect.right() - rcIcon.right() - 20,
                 vopt->rect.height());
    QString strCount = countString();

    QString strText = vopt->text;
    if (!strText.isEmpty()) {
        bool secondLevelFolder = (parent() && (parent()->type() == Category_GroupItem
                                                     || parent()->type() == Category_FolderItem));
        QColor colorText = Utils::WizStyleHelper::treeViewItemText(bSelected, secondLevelFolder);
        colorText.setAlpha(240);
        p->setPen(colorText);
        f.setStyleStrategy(QFont::PreferBitmap);
        QFontMetrics fm(f);
        strText = fm.elidedText(strText, Qt::ElideRight, rcText.width());
        int right = Utils::WizStyleHelper::drawSingleLineText(p, rcText, strText, Qt::AlignVCenter, colorText, f);
        //
        if (right != -1) {
            rcText.setLeft(right + 4);
        }
    }

    if (!strCount.isEmpty() && (rcText.width() > 10)) {
        QRect rcCount = rcText;
        rcCount.setTop(rcCount.top() + 1);  //add extra 1 pixel for vcenter / 2
        QColor colorCount = Utils::WizStyleHelper::treeViewItemTextExtend(bSelected);
        Utils::WizStyleHelper::drawSingleLineText(p, rcCount, strCount, Qt::AlignVCenter, colorCount, fontCount);
    }
}

bool WizCategoryViewItemBase::operator < (const QTreeWidgetItem &other) const
{
    const WizCategoryViewItemBase* pOther = dynamic_cast<const WizCategoryViewItemBase*>(&other);
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
    bool ret = WizToolsSmartCompare(strThis, strOther) < 0;
    //
    if (strThis == "drds_ks" && strOther[0] == 'h') {
        qDebug() << ret;
    }
    return ret;
}

QVariant WizCategoryViewItemBase::data(int column, int role) const
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

int WizCategoryViewItemBase::getItemHeight(int /*hintHeight*/) const
{
    return Utils::WizStyleHelper::treeViewItemHeight();
}


QString WizCategoryViewItemBase::id() const
{
    return ::WizMd5StringNoSpaceJava(QString(text(0) + m_strKbGUID).toUtf8());
}

void WizCategoryViewItemBase::setDocumentsCount(int nCurrent, int nTotal)
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

void WizCategoryViewItemBase::setExtraButtonIcon(const QString& file)
{
    if (file.isEmpty()) {
        m_extraButtonIcon = QPixmap();
    } else {
        m_extraButtonIcon = Utils::WizStyleHelper::loadPixmap(file);
    }
}

bool WizCategoryViewItemBase::getExtraButtonIcon(QPixmap &ret) const
{
    ret = m_extraButtonIcon;
    return !m_extraButtonIcon.isNull();
}

const int EXTRABUTTONRIGHTMARGIN = 10;

QRect WizCategoryViewItemBase::getExtraButtonRect(const QRect &rcItemBorder, bool ignoreIconExist) const
{
    QSize szBtn(WizSmartScaleUI(16), WizSmartScaleUI(16));
    if (!m_extraButtonIcon.isNull())
    {
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

bool WizCategoryViewItemBase::extraButtonClickTest()
{
    QPixmap pixmap;
    if(!getExtraButtonIcon(pixmap) || pixmap.isNull())
        return false;

    WizCategoryBaseView* view = dynamic_cast<WizCategoryBaseView*>(treeWidget());
    Q_ASSERT(view);

    QRect rcIemBorder = view->visualItemRect(this);
    QRect btnRect = getExtraButtonRect(rcIemBorder);
    int nClickDist = 2;
    btnRect.adjust(-nClickDist, -nClickDist, nClickDist, nClickDist);

    return btnRect.contains(view->hitPoint());
}

QString WizCategoryViewItemBase::getExtraButtonToolTip() const
{
    return "";
}

void WizCategoryViewItemBase::drawExtraBadge(QPainter* p, const QStyleOptionViewItem* vopt) const
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

WizCategoryViewSectionItem::WizCategoryViewSectionItem(WizExplorerApp& app, const QString& strName, int sortOrder)
    : WizCategoryViewItemBase(app, strName, "", Category_SectionItem)
    , m_sortOrder(sortOrder)
{
    setFlags(Qt::NoItemFlags); // user can not interact with it.
    setText(0, strName);
}

int WizCategoryViewSectionItem::getItemHeight(int /*nHeight*/) const
{    
    return WizSmartScaleUI(32);
}
void WizCategoryViewSectionItem::reset(const QString& sectionName, int sortOrder)
{
    m_strName = sectionName;
    m_sortOrder = sortOrder;
    //
    setText(0, sectionName);
}

void WizCategoryViewSectionItem::drawItemBody(QPainter *p, const QStyleOptionViewItem *vopt) const
{
    QString str = vopt->text;
    QRect rc(vopt->rect);
    rc.adjust(-12, 2, 0, 0);
    QFont font = p->font();
    Utils::WizStyleHelper::fontSection(font);
    Utils::WizStyleHelper::drawSingleLineText(p, rc, str, Qt::AlignVCenter, Utils::WizStyleHelper::treeViewSectionItemText(), font);
}

void WizCategoryViewSectionItem::drawExtraBadge(QPainter* p, const QStyleOptionViewItem* vopt) const
{
//    QRect rc = vopt->rect;
//    rc.setTop(rc.bottom());
//    p->fillRect(rc, Utils::StyleHelper::treeViewItemBottomLine());

    WizCategoryViewItemBase::drawExtraBadge(p, vopt);
}


/* -------------------- CWizCategoryViewMessageRootItem -------------------- */
WizCategoryViewMessageItem::WizCategoryViewMessageItem(WizExplorerApp& app,
                                                                 const QString& strName, int nFilterType,
                                                       int unread, QSize unreadSize)
    : WizCategoryViewItemBase(app, strName, "", Category_MessageItem)
    , m_nUnread(unread)
    , m_szUnreadSize(unreadSize)
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "category_messages", QSize(), ICON_OPTIONS);
    setIcon(0, icon);
    setText(0, strName);

    m_nFilter = nFilterType;
}

void WizCategoryViewMessageItem::getMessages(WizDatabase& db, const QString& userGUID, CWizMessageDataArray& arrayMsg)
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

void WizCategoryViewMessageItem::setUnreadCount(int nCount)
{
   m_nUnread = nCount;

#ifdef Q_OS_MAC
    Utils::WizNotify::setDockBadge(nCount);
#endif

   m_nUnread = nCount;
   WizCategoryBaseView* view = dynamic_cast<WizCategoryBaseView*>(treeWidget());
   Q_ASSERT(view);
   //
   if (m_nUnread > 0)
   {
       QFont f;
       Utils::WizStyleHelper::fontNormal(f);
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

QString WizCategoryViewMessageItem::unreadString() const
{
    return unreadNumToString(m_nUnread);
}

bool WizCategoryViewMessageItem::hitTestUnread()
{
    if (m_nUnread == 0)
        return false;

    WizCategoryBaseView* view = dynamic_cast<WizCategoryBaseView*>(treeWidget());
    Q_ASSERT(view);

    QRect rcItem = view->visualItemRect(this);
    QPoint pt = view->hitPoint();
    //
    QRect rcRect = getExtraButtonRect(rcItem, true);
    return rcRect.contains(pt);
}

QString WizCategoryViewMessageItem::getSectionName()
{
    return WIZ_CATEGORY_SECTION_GENERAL;
}

QRect WizCategoryViewMessageItem::getExtraButtonRect(const QRect& itemBorder, bool ignoreIconExist) const
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
//    if (MainWindow* mainWindow = MainWindow::instance())
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
//            if (MainWindow* mainWindow = MainWindow::instance())
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
    f.setPixelSize(::WizSmartScaleUI(10));
    p->setFont(f);
    p->setPen("999999");
    //
    QRect rcb = rcd;
    if (isPressed)
    {
        rcb.adjust(0, 0, 0, 2);
        QPixmap pixBg(Utils::WizStyleHelper::loadPixmap("category_unreadButton_selected"));
        p->drawPixmap(rcb, pixBg);
        rcb.adjust(0, 0, 0, -2);
        p->drawText(rcb, Qt::AlignCenter, text);
    }
    else
    {
        rcb.adjust(0, 0, 0, 2);
        QPixmap pixBg(Utils::WizStyleHelper::loadPixmap("category_unreadButton"));
        p->drawPixmap(rcb, pixBg);
        rcb.adjust(0, 0, 0, -2);
        p->drawText(rcb, Qt::AlignCenter, text);
    }
}

void WizCategoryViewMessageItem::drawExtraBadge(QPainter* p, const QStyleOptionViewItem *vopt) const
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

void WizCategoryViewMessageItem::mousePressed(const QPoint& pos)
{
    QRect rcBorder = treeWidget()->visualItemRect(this);
    QRect rcIcon = getExtraButtonRect(rcBorder, true);
    if (rcIcon.contains(pos))
    {
        m_extraButtonIconPressed = true;
    }
}

void WizCategoryViewMessageItem::mouseReleased(const QPoint& pos)
{
    m_extraButtonIconPressed = false;
}

/* -------------------- CWizCategoryViewShortcutRootItem -------------------- */
WizCategoryViewShortcutRootItem::WizCategoryViewShortcutRootItem(WizExplorerApp& app,
                                                                   const QString& strName)
    : WizCategoryViewItemBase(app, strName, "", Category_ShortcutRootItem)
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "category_shortcut", QSize(), ICON_OPTIONS);
    setIcon(0, icon);
    setText(0, strName);
}

void WizCategoryViewShortcutRootItem::getDocuments(WizDatabase& /*db*/, CWizDocumentDataArray& arrayDocument)
{
    for (int i = 0; i < childCount(); i++)
    {
        WizCategoryViewShortcutItem *pItem = dynamic_cast<WizCategoryViewShortcutItem*>(child(i));
        if (pItem && !pItem->guid().isEmpty())
        {
            WizDatabase &db = m_app.databaseManager().db(pItem->kbGUID());
            WIZDOCUMENTDATA doc;
            if (db.documentFromGuid(pItem->guid(), doc))
            {
                arrayDocument.push_back(doc);
            }
        }
    }
}

bool WizCategoryViewShortcutRootItem::accept(WizDatabase& /*db*/, const WIZDOCUMENTDATA& data)
{
    for (int i = 0; i < childCount(); i++)
    {
        WizCategoryViewShortcutItem *pItem = dynamic_cast<WizCategoryViewShortcutItem*>(child(i));
        if (pItem)
        {
            if (pItem->guid() == data.strGUID)
                return true;
        }
    }
    return false;
}

void WizCategoryViewShortcutRootItem::drop(const CWizDocumentDataArray& arrayDocument, bool /*forceCopy*/)
{
    bool changed = false;
    for (WIZDOCUMENTDATA document : arrayDocument)
    {        
        WizCategoryViewShortcutItem *pItem = addDocumentToShortcuts(document);
        if (pItem)
        {
            changed = true;
        }
    }

    if (changed)
    {
        QTimer::singleShot(200, [this]() {
            WizCategoryView* categoryView = dynamic_cast<WizCategoryView*>(treeWidget());
            Q_ASSERT(categoryView);
            categoryView->saveShortcutState();
        });
    }
}

void WizCategoryViewShortcutRootItem::drop(const WizCategoryViewItemBase* pItem)
{
    WizCategoryViewShortcutItem* newItem = addItemToShortcuts(pItem);
    if (!newItem)
        return;
    //
    treeWidget()->blockSignals(true);
    treeWidget()->setCurrentItem(newItem);
    treeWidget()->blockSignals(false);
    sortChildren(0, Qt::AscendingOrder);

    WizCategoryView* categoryView = dynamic_cast<WizCategoryView*>(treeWidget());
    QTimer::singleShot(200, categoryView, SLOT(saveShortcutState()));
}

bool WizCategoryViewShortcutRootItem::acceptDrop(const WizCategoryViewItemBase* pItem) const
{
    if (!pItem)
        return false;

    if (pItem->type() == Category_FolderItem || pItem->type() == Category_TagItem || pItem->type() == Category_GroupItem)
        return true;

    return false;
}

WizCategoryViewShortcutItem* WizCategoryViewShortcutRootItem::addItemToShortcuts(const WizCategoryViewItemBase* pItem)
{
    WizCategoryViewShortcutItem* newItem = nullptr;
    if (pItem->type() == Category_FolderItem)
    {
        const WizCategoryViewFolderItem* folderItem = dynamic_cast<const WizCategoryViewFolderItem*>(pItem);
        newItem = new WizCategoryViewShortcutItem(m_app, WizDatabase::getLocationName(folderItem->location()),
                                                                                 WizCategoryViewShortcutItem::PersonalFolder, "", "", folderItem->location());
    }
    else if (pItem->type() == Category_TagItem)
    {
        const WizCategoryViewTagItem* tagItem = dynamic_cast<const WizCategoryViewTagItem*>(pItem);
        newItem = new WizCategoryViewShortcutItem(m_app, tagItem->tag().strName, WizCategoryViewShortcutItem::PersonalTag,
                                                    tagItem->tag().strKbGUID, tagItem->tag().strGUID, "");
    }
    else if (pItem->type() == Category_GroupItem)
    {
        const WizCategoryViewGroupItem* groupItem = dynamic_cast<const WizCategoryViewGroupItem*>(pItem);
        newItem = new WizCategoryViewShortcutItem(m_app, groupItem->tag().strName, WizCategoryViewShortcutItem::GroupTag,
                                                    groupItem->tag().strKbGUID, groupItem->tag().strGUID, "");
    }
    else
    {
        return nullptr;
    }
    //
    for (int i = 0; i < childCount(); i++)
    {
        WizCategoryViewShortcutItem *shortcutItem = dynamic_cast<WizCategoryViewShortcutItem*>(child(i));
        if (shortcutItem)
        {
            switch (shortcutItem->shortcutType()) {
            case WizCategoryViewShortcutItem::PersonalTag:
            case WizCategoryViewShortcutItem::GroupTag:
            {
                if (shortcutItem->guid() == newItem->guid())
                {
                    delete newItem;
                    return nullptr;
                }
            }
                break;
            case WizCategoryViewShortcutItem::PersonalFolder:
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

WizCategoryViewShortcutItem*WizCategoryViewShortcutRootItem::addDocumentToShortcuts(const WIZDOCUMENTDATA& document)
{
    for (int i = 0; i < childCount(); i++)
    {
        WizCategoryViewShortcutItem *pItem = dynamic_cast<WizCategoryViewShortcutItem*>(child(i));
        if (pItem)
        {
            if (pItem->guid() == document.strGUID)
                return nullptr;
        }
    }

    if (isContainsPlaceHoldItem())
        removePlaceHoldItem();

    bool isEncrypted = document.nProtected == 1;
    WizCategoryViewShortcutItem *pItem = new WizCategoryViewShortcutItem(m_app,
                                                                           document.strTitle, WizCategoryViewShortcutItem::Document,
                                                                           document.strKbGUID, document.strGUID, document.strLocation, isEncrypted);

    addChild(pItem);
    sortChildren(0, Qt::AscendingOrder);
    return pItem;
}

QString WizCategoryViewShortcutRootItem::getSectionName()
{
    return WIZ_CATEGORY_SECTION_GENERAL;
}

void WizCategoryViewShortcutRootItem::addPlaceHoldItem()
{
    WizCategoryViewShortcutPlaceHoldItem *item = new
            WizCategoryViewShortcutPlaceHoldItem(m_app, WIZ_CATEGORY_SHOTCUT_PLACEHOLD);
    item->setText(0, WIZ_CATEGORY_SHOTCUT_PLACEHOLD);
    addChild(item);
}

bool WizCategoryViewShortcutRootItem::isContainsPlaceHoldItem()
{
    if (childCount() < 1)
        return false;

    QTreeWidgetItem *item = child(0);
    return item->text(0) == WIZ_CATEGORY_SHOTCUT_PLACEHOLD;
}

void WizCategoryViewShortcutRootItem::removePlaceHoldItem()
{
    if (isContainsPlaceHoldItem())
    {
        removeChild(child(0));
    }
}


/* -------------------- CWizCategoryViewSearchRootItem -------------------- */
WizCategoryViewSearchRootItem::WizCategoryViewSearchRootItem(WizExplorerApp& app,
                                                               const QString& strName)
    : WizCategoryViewItemBase(app, strName, "", Category_QuickSearchRootItem)
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "category_search", QSize(), ICON_OPTIONS);
    setIcon(0, icon);
    setText(0, strName);
}

void WizCategoryViewSearchRootItem::showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
{
    if (WizCategoryView* view = dynamic_cast<WizCategoryView *>(pCtrl)) {
        view->showCustomSearchContextMenu(pos, false);
    }
}

QString WizCategoryViewSearchRootItem::getSectionName()
{
    return WIZ_CATEGORY_SECTION_GENERAL;
}


/* ------------------------------ CWizCategoryViewAllFoldersItem ------------------------------ */

WizCategoryViewAllFoldersItem::WizCategoryViewAllFoldersItem(WizExplorerApp& app,
                                                               const QString& strName,
                                                               const QString& strKbGUID)
    : WizCategoryViewItemBase(app, strName, strKbGUID, Category_AllFoldersItem)
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "category_folders", QSize(), ICON_OPTIONS);
    setIcon(0, icon);
    setText(0, strName);
}

void WizCategoryViewAllFoldersItem::getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    db.getDocumentsBySQLWhere("DOCUMENT_LOCATION not like '/Deleted Items/%' order by DT_DATA_MODIFIED desc limit 1000", arrayDocument);
}

bool WizCategoryViewAllFoldersItem::accept(WizDatabase& db, const WIZDOCUMENTDATA& data)
{
    if (db.isInDeletedItems(data.strLocation)) {
        return false;
    }

    return !db.isGroup();
}

bool WizCategoryViewAllFoldersItem::acceptDrop(const WizCategoryViewItemBase* pItem) const
{
//    return pItem->type() == Category_FolderItem | pItem->type() == Category_GroupItem;

    const WizCategoryViewFolderItem* item = dynamic_cast<const WizCategoryViewFolderItem*>(pItem);
    return NULL != item;
}

QString WizCategoryViewAllFoldersItem::getSectionName()
{
    return WIZ_CATEGORY_SECTION_PERSONAL;
}


void WizCategoryViewAllFoldersItem::showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
{
    if (WizCategoryView* view = dynamic_cast<WizCategoryView *>(pCtrl)) {
        view->showFolderRootContextMenu(pos);
    }
}


/* ------------------------------ CWizCategoryViewFolderItem ------------------------------ */

WizCategoryViewFolderItem::WizCategoryViewFolderItem(WizExplorerApp& app,
                                                       const QString& strLocation,
                                                       const QString& strKbGUID)
    : WizCategoryViewItemBase(app, strLocation, strKbGUID, Category_FolderItem)
{
    QIcon icon;
    if (::WizIsPredefinedLocation(strLocation) && strLocation == "/My Journals/") {
        icon = WizLoadSkinIcon(app.userSettings().skin(), "category_folder_diary", QSize(), ICON_OPTIONS);
    } else {
        icon = WizLoadSkinIcon(app.userSettings().skin(), "category_folder", QSize(), ICON_OPTIONS);
    }
    setIcon(0, icon);
    setText(0, WizDatabase::getLocationDisplayName(strLocation));
}

QTreeWidgetItem* WizCategoryViewFolderItem::clone() const
{
    return new WizCategoryViewFolderItem(m_app, m_strName, m_strKbGUID);
}

QString WizCategoryViewFolderItem::id() const
{
    return ::WizMd5StringNoSpaceJava(QString(m_strName + m_strKbGUID).toUtf8());
}

void WizCategoryViewFolderItem::setLocation(const QString& strLocation)
{
    m_strName = strLocation;
}

void WizCategoryViewFolderItem::getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    db.getDocumentsByLocation(m_strName, arrayDocument, m_app.userSettings().showSubFolderDocuments());
}

bool WizCategoryViewFolderItem::accept(WizDatabase& db, const WIZDOCUMENTDATA& data)
{
    Q_UNUSED(db);

    if (m_strName == data.strLocation && data.strKbGUID == kbGUID())
        return true;

    if (kbGUID().isEmpty() && !db.isGroup())
        return true;

    return false;
}

bool WizCategoryViewFolderItem::acceptDrop(const WIZDOCUMENTDATA& data) const
{
    Q_UNUSED(data);

    return true;
}

bool WizCategoryViewFolderItem::acceptDrop(const WizCategoryViewItemBase* pItem) const
{
//    return pItem->type() == Category_FolderItem | pItem->type() == Category_GroupItem;

    const WizCategoryViewFolderItem* item = dynamic_cast<const WizCategoryViewFolderItem*>(pItem);
    return NULL != item;
}

void WizCategoryViewFolderItem::drop(const CWizDocumentDataArray& arrayDocument, bool forceCopy)
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

    WizDocumentOperator documentOperator(m_app.databaseManager());
    if (needCopy)
    {
        documentOperator.copyDocumentsToPersonalFolder(arrayOp, location(), false, true, true);
    }
    else
    {
        documentOperator.moveDocumentsToPersonalFolder(arrayOp, location(), false);
    }    
}

void WizCategoryViewFolderItem::showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
{
    if (WizCategoryView* view = dynamic_cast<WizCategoryView *>(pCtrl)) {
        view->showFolderContextMenu(pos);
    }
}

QString WizCategoryViewFolderItem::name() const
{
    return WizDatabase::getLocationName(m_strName);
}

bool WizCategoryViewFolderItem::operator < (const QTreeWidgetItem &other) const
{
    const WizCategoryViewFolderItem* pOther = dynamic_cast<const WizCategoryViewFolderItem*>(&other);
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
            QSettings* setting = WizGlobal::settings();
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
    return WizCategoryViewItemBase::operator <(other);
}



/* ------------------------------ CWizCategoryViewAllTagsItem ------------------------------ */

WizCategoryViewAllTagsItem::WizCategoryViewAllTagsItem(WizExplorerApp& app,
                                                         const QString& strName,
                                                         const QString& strKbGUID)
    : WizCategoryViewItemBase(app, strName, strKbGUID, Category_AllTagsItem)
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "category_tags", QSize(), ICON_OPTIONS);
    setIcon(0, icon);
    setText(0, strName);
}

void WizCategoryViewAllTagsItem::showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
{
    if (WizCategoryView* view = dynamic_cast<WizCategoryView *>(pCtrl)) {
        view->showTagRootContextMenu(pos);
    }
}

void WizCategoryViewAllTagsItem::getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    Q_UNUSED(db);
    Q_UNUSED(arrayDocument);
    // no deleted
    //db.getDocumentsNoTag(arrayDocument);
}

bool WizCategoryViewAllTagsItem::accept(WizDatabase& db, const WIZDOCUMENTDATA& data)
{
    CString strTagGUIDs = db.getDocumentTagGuidsString(data.strGUID);
    if (strTagGUIDs.isEmpty() && data.strKbGUID == kbGUID()) {
        return true;
    }

    return false;
}

bool WizCategoryViewAllTagsItem::acceptDrop(const WizCategoryViewItemBase* pItem) const
{
    const WizCategoryViewTagItem* item = dynamic_cast<const WizCategoryViewTagItem*>(pItem);
    return NULL != item;
}

QString WizCategoryViewAllTagsItem::getSectionName()
{
    return WIZ_CATEGORY_SECTION_PERSONAL;
}

/* ------------------------------ CWizCategoryViewTagItem ------------------------------ */

WizCategoryViewTagItem::WizCategoryViewTagItem(WizExplorerApp& app,
                                                 const WIZTAGDATA& tag,
                                                 const QString& strKbGUID)
    : WizCategoryViewItemBase(app, tag.strName, strKbGUID, Category_TagItem)
    , m_tag(tag)
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "category_tagItem", QSize(), ICON_OPTIONS);
    setIcon(0, icon);
    setText(0, WizDatabase::tagNameToDisplayName(tag.strName));
}

QTreeWidgetItem* WizCategoryViewTagItem::clone() const
{
    return new WizCategoryViewTagItem(m_app, m_tag, m_strKbGUID);
}

void WizCategoryViewTagItem::getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    db.getDocumentsByTag(m_tag, arrayDocument);
}

bool WizCategoryViewTagItem::accept(WizDatabase& db, const WIZDOCUMENTDATA& data)
{
    if (data.strKbGUID == kbGUID()) {
        QString strTagGUIDs = db.getDocumentTagGuidsString(data.strGUID);
        if (strTagGUIDs.indexOf(m_tag.strGUID) != -1)
            return true;
    }

    return false;
}

bool WizCategoryViewTagItem::acceptDrop(const WIZDOCUMENTDATA& data) const
{
    // only accept drop from user db
    if (data.strKbGUID == kbGUID()) {
        return true;
    }

    return false;
}

bool WizCategoryViewTagItem::acceptDrop(const WizCategoryViewItemBase* pItem) const
{
    return pItem && pItem->type() == Category_TagItem;
}

void WizCategoryViewTagItem::drop(const CWizDocumentDataArray& arrayDocument, bool forceCopy)
{
    Q_UNUSED(forceCopy);

    WizDatabase& db = WizDatabaseManager::instance()->db(kbGUID());
    for (WIZDOCUMENTDATA document : arrayDocument)
    {
        if (!acceptDrop(document))
            continue;

        // skip
        QString strTagGUIDs = db.getDocumentTagGuidsString(document.strGUID);
        if (strTagGUIDs.indexOf(m_tag.strGUID) != -1)
            continue;

        WizDocument doc(db, document);
        doc.addTag(tag());
    }
}

void WizCategoryViewTagItem::drop(const WizCategoryViewItemBase* pItem)
{
    if (pItem && pItem->type() == Category_TagItem)
    {
        const WizCategoryViewTagItem* childItem = dynamic_cast<const WizCategoryViewTagItem*>(pItem);
        WIZTAGDATA childTag = childItem->tag();
        childTag.strParentGUID = tag().strGUID;
        m_app.databaseManager().db().modifyTag(childTag);
    }
}

void WizCategoryViewTagItem::showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
{
    if (WizCategoryView* view = dynamic_cast<WizCategoryView *>(pCtrl)) {
        view->showTagContextMenu(pos);
    }
}

void WizCategoryViewTagItem::reload(WizDatabase& db)
{
    db.tagFromGuid(m_tag.strGUID, m_tag);
    setText(0, m_tag.strName);
    m_strName = m_tag.strName;
}

void WizCategoryViewTagItem::setTagPosition(int nPos)
{
    m_tag.nPosition = nPos;
}


/* --------------------- CWizCategoryViewStyleRootItem --------------------- */
WizCategoryViewStyleRootItem::WizCategoryViewStyleRootItem(WizExplorerApp& app,
                                                             const QString& strName)
    : WizCategoryViewItemBase(app, strName)
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "style", QSize(), ICON_OPTIONS);
    setIcon(0, icon);
    setText(0, strName);
}

QString WizCategoryViewStyleRootItem::getSectionName()
{
    return WIZ_CATEGORY_SECTION_PERSONAL;
}

/* ---------------------------- CWizCategoryViewGroupsRootItem ---------------------------- */

WizCategoryViewGroupsRootItem::WizCategoryViewGroupsRootItem(WizExplorerApp& app, const QString& strName)
    : WizCategoryViewItemBase(app, strName, "", Category_GroupsRootItem)
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "category_group", QSize(), ICON_OPTIONS);
    setIcon(0, icon);
    setText(0, strName);
}

void WizCategoryViewGroupsRootItem::showContextMenu(WizCategoryBaseView *pCtrl, QPoint pos)
{
    if (WizCategoryView* view = dynamic_cast<WizCategoryView *>(pCtrl)) {
        view->showNormalGroupRootContextMenu(pos);
    }
}

void WizCategoryViewGroupsRootItem::getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    Q_UNUSED(db);

    for (int i = 0; i < childCount(); i++) {
        WizCategoryViewGroupRootItem* pGroup = dynamic_cast<WizCategoryViewGroupRootItem*>(child(i));
        Q_ASSERT(pGroup);
        if (!pGroup)
            return;

        WizDatabase& db = WizDatabaseManager::instance()->db(pGroup->kbGUID());

        CWizDocumentDataArray arrayDoc;
        if (db.getDocumentsByTime(QDateTime::currentDateTime().addDays(-3), arrayDocument)) {
            arrayDocument.insert(arrayDocument.begin(), arrayDoc.begin(), arrayDoc.end());
        }
    }
}

bool WizCategoryViewGroupsRootItem::accept(WizDatabase& db, const WIZDOCUMENTDATA& data)
{
    Q_UNUSED(db);

    QDateTime t(QDateTime::currentDateTime().addDays(-3));
    for (int i = 0; i < childCount(); i++) {
        WizCategoryViewGroupRootItem* pGroup = dynamic_cast<WizCategoryViewGroupRootItem*>(child(i));
        Q_ASSERT(pGroup);
        if (!pGroup)
            continue;

        if (pGroup->kbGUID() == data.strKbGUID) {
            if (data.tDataModified > t)
                return true;
        }
    }
    //
    return false;
}

//bool CWizCategoryViewGroupsRootItem::acceptDrop(const CWizCategoryViewItemBase* pItem) const
//{
//    const CWizCategoryViewGroupItem* item = dynamic_cast<const CWizCategoryViewGroupItem*>(pItem);
//    return NULL != item;
//}
QString WizCategoryViewGroupsRootItem::getSectionName()
{
    return WIZ_CATEGORY_SECTION_GROUPS;
}


/* ------------------------------ CWizCategoryViewGroupRootItem ------------------------------ */
WizCategoryViewBizGroupRootItem::WizCategoryViewBizGroupRootItem(WizExplorerApp& app,
                                                                   const WIZBIZDATA& biz)
    : WizCategoryViewGroupsRootItem(app, biz.bizName)
    , m_biz(biz)
    , m_unReadCount(0)
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "category_biz", QSize(), ICON_OPTIONS);
    setIcon(0, icon);
}

void WizCategoryViewBizGroupRootItem::showContextMenu(WizCategoryBaseView *pCtrl, QPoint pos)
{
    if (WizCategoryView* view = dynamic_cast<WizCategoryView *>(pCtrl)) {
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

void WizCategoryViewBizGroupRootItem::getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    if (isUnreadButtonUseable() && hitTestUnread())
    {
        for (int i = 0; i < childCount(); i++)
        {
            WizCategoryViewGroupRootItem* pGroup = dynamic_cast<WizCategoryViewGroupRootItem*>(child(i));
            Q_ASSERT(pGroup);
            if (!pGroup)
                return;

            WizDatabase& db = WizDatabaseManager::instance()->db(pGroup->kbGUID());

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
        WizCategoryViewGroupsRootItem::getDocuments(db, arrayDocument);
    }
}

void WizCategoryViewBizGroupRootItem::drawExtraBadge(QPainter* p, const QStyleOptionViewItem* vopt) const
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
        WizCategoryViewGroupsRootItem::drawExtraBadge(p, vopt);
    }
}

void WizCategoryViewBizGroupRootItem::mousePressed(const QPoint& pos)
{
    QRect rcBorder = treeWidget()->visualItemRect(this);
    QRect rcIcon = getExtraButtonRect(rcBorder, true);
    if (rcIcon.contains(pos))
    {
        m_extraButtonIconPressed = true;
    }
}

void WizCategoryViewBizGroupRootItem::mouseReleased(const QPoint& pos)
{
    m_extraButtonIconPressed = false;
}

QString WizCategoryViewBizGroupRootItem::getExtraButtonToolTip() const
{
    if (m_unReadCount > 0 && isUnreadButtonUseable())
        return QObject::tr("You have %1 unread notes").arg(m_unReadCount);

    if (m_extraButtonIcon.isNull() || !isExtraButtonUseable())
        return "";

    return QObject::tr("Your enterprise services has expired");
}

QRect WizCategoryViewBizGroupRootItem::getExtraButtonRect(const QRect& itemBorder, bool ignoreIconExist) const
{
    if (!isUnreadButtonUseable())
        return WizCategoryViewItemBase::getExtraButtonRect(itemBorder, ignoreIconExist);

    if (!m_unReadCount && !ignoreIconExist)
        return QRect();

    int nButtonWidth = 26;
    int nButtonHeight = 14;
    QRect rc(itemBorder.right() - EXTRABUTTONRIGHTMARGIN - nButtonWidth, itemBorder.y() + (itemBorder.height() - nButtonHeight) / 2,
             nButtonWidth, nButtonHeight);
    return rc;
}

bool WizCategoryViewBizGroupRootItem::isExtraButtonUseable() const
{
    return !isUnreadButtonUseable();
}

bool WizCategoryViewBizGroupRootItem::isUnreadButtonUseable() const
{
     bool bUseable = (!isExpanded()) && (m_unReadCount > 0);
     return bUseable;
}

void WizCategoryViewBizGroupRootItem::updateUnreadCount()
{
    m_unReadCount = 0;
    int nChildCount = childCount();
    for (int i = 0; i < nChildCount; i++)
    {
        WizCategoryViewGroupRootItem* childItem = dynamic_cast<WizCategoryViewGroupRootItem*>(child(i));
        if (childItem)
        {
            m_unReadCount += childItem->getUnreadCount();
        }
    }

    WizCategoryBaseView* view = dynamic_cast<WizCategoryBaseView*>(treeWidget());
    if (m_unReadCount > 0)
    {
        //
        QFont f;
        Utils::WizStyleHelper::fontNormal(f);
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

QString WizCategoryViewBizGroupRootItem::unreadString() const
{
    return unreadNumToString(m_unReadCount);
}

bool WizCategoryViewBizGroupRootItem::hitTestUnread()
{
    WizCategoryBaseView* view = dynamic_cast<WizCategoryBaseView*>(treeWidget());
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

bool WizCategoryViewBizGroupRootItem::isOwner()
{
    return m_biz.bizUserRole == WIZ_BIZROLE_OWNER;
}
bool WizCategoryViewBizGroupRootItem::isAdmin()
{
    return m_biz.bizUserRole == WIZ_BIZROLE_ADMIN;
}

bool WizCategoryViewBizGroupRootItem::isHr()
{
    return m_biz.bizUserRole <= WIZ_BIZROLE_HR;
}

WizCategoryViewOwnGroupRootItem::WizCategoryViewOwnGroupRootItem(WizExplorerApp& app)
    : WizCategoryViewGroupsRootItem(app, CATEGORY_OWN_GROUPS)
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "category_group", QSize(), ICON_OPTIONS);
    setIcon(0, icon);
}

void WizCategoryViewOwnGroupRootItem::showContextMenu(WizCategoryBaseView *pCtrl, QPoint pos)
{
    Q_UNUSED(pCtrl)
    Q_UNUSED(pos)
//    if (CWizCategoryView* view = dynamic_cast<CWizCategoryView *>(pCtrl)) {
//        view->showOwnGroupRootContextMenu(pos);
//    }
}


WizCategoryViewJionedGroupRootItem::WizCategoryViewJionedGroupRootItem(WizExplorerApp& app)
    : WizCategoryViewGroupsRootItem(app, CATEGORY_OTHER_GROUPS)
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "category_group", QSize(), ICON_OPTIONS);
    setIcon(0, icon);
}


QString WizCategoryViewCreateGroupLinkItem::getSectionName()
{
    return WIZ_CATEGORY_SECTION_GROUPS;
}

/* ------------------------------ CWizCategoryViewGroupRootItem ------------------------------ */

WizCategoryViewGroupRootItem::WizCategoryViewGroupRootItem(WizExplorerApp& app,
                                                             const WIZGROUPDATA& group)
    : WizCategoryViewItemBase(app, group.strGroupName, group.strGroupGUID, Category_GroupRootItem)
    , m_group(group)
    , m_nUnread(0)
{
    QIcon icon;
    if (group.bEncryptData)
    {
        icon = WizLoadSkinIcon(app.userSettings().skin(), "category_group_enc", QSize(), ICON_OPTIONS);
    }
    else
    {
        icon = WizLoadSkinIcon(app.userSettings().skin(), "category_group", QSize(), ICON_OPTIONS);
    }
    setIcon(0, icon);
    setText(0, m_strName);
}

void WizCategoryViewGroupRootItem::showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
{
    if (WizCategoryView* view = dynamic_cast<WizCategoryView *>(pCtrl)) {
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

void WizCategoryViewGroupRootItem::getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    if (hitTestUnread() && m_nUnread)
    {
        db.getGroupUnreadDocuments(arrayDocument);
    }
    else
    {
        db.getLastestDocuments(arrayDocument, 1000);
    }
}

bool WizCategoryViewGroupRootItem::accept(WizDatabase& db, const WIZDOCUMENTDATA& data)
{
    if (db.isInDeletedItems(data.strLocation))
        return false;

    if (data.strKbGUID == kbGUID())
        return true;

    return false;
}

bool WizCategoryViewGroupRootItem::acceptDrop(const WIZDOCUMENTDATA &data) const
{
    Q_UNUSED(data);

    WizDatabase& db = WizDatabaseManager::instance()->db(kbGUID());
    if (WIZ_USERGROUP_AUTHOR >= db.permission())
        return true;

    return false;
}

bool WizCategoryViewGroupRootItem::acceptDrop(const WizCategoryViewItemBase* pItem) const
{
//    return pItem->type() == Category_FolderItem | pItem->type() == Category_GroupItem;

    const WizCategoryViewGroupItem* item = dynamic_cast<const WizCategoryViewGroupItem*>(pItem);
    if (!item) {
        return false;
    }

    if (item->kbGUID() != kbGUID()) {
        return false;
    }
    //
    if (WizCategoryView* view = dynamic_cast<WizCategoryView*>(treeWidget())) {
        //
        QPoint pos = QCursor::pos();
        pos = view->mapFromGlobal(pos);
        auto index = view->indexFromItem(this);
        //
        int dropIndicatorPosition = view->position(pos, view->visualRect(index), index);
        if (1 == dropIndicatorPosition) { //above
            qDebug() << pos << view->visualRect(index);
            return false;
        }
    }
    return true;
}

bool WizCategoryViewGroupRootItem::acceptDrop(const QString& urls) const
{
    Q_UNUSED(urls);
    WizDatabase& db = m_app.databaseManager().db(kbGUID());

    return WIZ_USERGROUP_AUTHOR >= db.permission();
}

void WizCategoryViewGroupRootItem::drop(const CWizDocumentDataArray& arrayDocument, bool forceCopy)
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

    WizDocumentOperator documentOperator(m_app.databaseManager());

    if (needCopy)
    {
        WIZTAGDATA tag;
        tag.strKbGUID = m_strKbGUID;
        documentOperator.copyDocumentsToGroupFolder(arrayOp, tag, true);
    }
    else
    {
        WIZTAGDATA tag;
        tag.strKbGUID = m_strKbGUID;
        documentOperator.moveDocumentsToGroupFolder(arrayOp, tag, true);
    }
}

void WizCategoryViewGroupRootItem::drawExtraBadge(QPainter* p, const QStyleOptionViewItem* vopt) const
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
        WizCategoryViewItemBase::drawExtraBadge(p, vopt);
    }
}

void WizCategoryViewGroupRootItem::reload(WizDatabase& db)
{
    m_strName = db.name();
    setText(0, db.name());
}

void WizCategoryViewGroupRootItem::mousePressed(const QPoint& pos)
{
    QRect rcBorder = treeWidget()->visualItemRect(this);
    QRect rcIcon = getExtraButtonRect(rcBorder, true);
    if (rcIcon.contains(pos))
    {
        m_extraButtonIconPressed = true;
    }
}

void WizCategoryViewGroupRootItem::mouseReleased(const QPoint& pos)
{
    m_extraButtonIconPressed = false;
}

bool WizCategoryViewGroupRootItem::operator<(const QTreeWidgetItem& other) const
{
    if (other.type() != Category_GroupRootItem)
        return QTreeWidgetItem::operator <(other);

    const WizCategoryViewGroupRootItem* pItem = dynamic_cast<const WizCategoryViewGroupRootItem*>(&other);
    //
    return WizToolsSmartCompare(m_group.strGroupName, pItem->m_group.strGroupName) < 0;
}

bool WizCategoryViewGroupRootItem::isAdmin(WizDatabase& db)
{
    if (isBizGroup())
    {
        if (WizCategoryViewBizGroupRootItem* pBiz = dynamic_cast<WizCategoryViewBizGroupRootItem *>(parent()))
        {
            if (pBiz->isAdmin())
                return true;
        }
    }
    //
    return db.isGroupAdmin();
}

bool WizCategoryViewGroupRootItem::isOwner(WizDatabase& db)
{
    return db.isGroupOwner();
}

bool WizCategoryViewGroupRootItem::isBizGroup() const
{
    return m_group.isBiz();
}

QString WizCategoryViewGroupRootItem::bizGUID() const
{
    return m_group.bizGUID;
}

void WizCategoryViewGroupRootItem::setUnreadCount(int nCount)
{
    m_nUnread = nCount;
    WizCategoryBaseView* view = dynamic_cast<WizCategoryBaseView*>(treeWidget());
    //
    if (m_nUnread > 0)
    {
        QFont f;
        Utils::WizStyleHelper::fontNormal(f);
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

int WizCategoryViewGroupRootItem::getUnreadCount()
{
    return m_nUnread;
}

QString WizCategoryViewGroupRootItem::unreadString() const
{
    return unreadNumToString(m_nUnread);
}

bool WizCategoryViewGroupRootItem::hitTestUnread()
{
    WizCategoryBaseView* view = dynamic_cast<WizCategoryBaseView*>(treeWidget());
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

QString WizCategoryViewGroupRootItem::getExtraButtonToolTip() const
{
    if (m_nUnread > 0)
        return QObject::tr("You have %1 unread notes").arg(m_nUnread);

    if (m_extraButtonIcon.isNull())
        return "";

    return QObject::tr("Your group is in the abnormal state");
}

QRect WizCategoryViewGroupRootItem::getExtraButtonRect(const QRect& itemBorder, bool ignoreIconExist) const
{
    if (m_nUnread == 0)
        return WizCategoryViewItemBase::getExtraButtonRect(itemBorder, ignoreIconExist);

    if (!m_nUnread && !ignoreIconExist)
        return QRect();

    int nButtonWidth = 26;
    int nButtonHeight = 14;
    QRect rc(itemBorder.right() - EXTRABUTTONRIGHTMARGIN - nButtonWidth, itemBorder.y() + (itemBorder.height() - nButtonHeight) / 2,
             nButtonWidth, nButtonHeight);
    return rc;
}

/* --------------------- CWizCategoryViewGroupNoTagItem --------------------- */
WizCategoryViewGroupNoTagItem::WizCategoryViewGroupNoTagItem(WizExplorerApp& app,
                                                               const QString& strKbGUID)
    : WizCategoryViewItemBase(app, PREDEFINED_UNCLASSIFIED, strKbGUID, Category_GroupNoTagItem)
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "category_unclassified", QSize(), ICON_OPTIONS);
    setIcon(0, icon);
    setText(0, PREDEFINED_UNCLASSIFIED);
}

void WizCategoryViewGroupNoTagItem::getDocuments(WizDatabase& db,
                                                  CWizDocumentDataArray& arrayDocument)
{
    db.getDocumentsNoTag(arrayDocument);
}

bool WizCategoryViewGroupNoTagItem::accept(WizDatabase& db, const WIZDOCUMENTDATA& data)
{
    if (kbGUID() != data.strKbGUID)
        return false;

    if (db.isInDeletedItems(data.strLocation))
        return false;

    QString strTagGUIDs = db.getDocumentTagGuidsString(data.strGUID);
    if (strTagGUIDs.isEmpty())
        return true;

    return false;
}


/* ------------------------------ CWizCategoryViewGroupItem ------------------------------ */

WizCategoryViewGroupItem::WizCategoryViewGroupItem(WizExplorerApp& app,
                                                     const WIZTAGDATA& tag,
                                                     const QString& strKbGUID)
    : WizCategoryViewItemBase(app, tag.strName, strKbGUID, Category_GroupItem)
    , m_tag(tag)
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "category_folder", QSize(), ICON_OPTIONS);
    setIcon(0, icon);
    setText(0, WizDatabase::tagNameToDisplayName(tag.strName));
}

void WizCategoryViewGroupItem::showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
{
    if (WizCategoryView* view = dynamic_cast<WizCategoryView *>(pCtrl)) {
        view->showGroupContextMenu(pos);
    }
}

void WizCategoryViewGroupItem::getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    db.getDocumentsByTag(m_tag, arrayDocument);
}

bool WizCategoryViewGroupItem::accept(WizDatabase& db, const WIZDOCUMENTDATA& data)
{
    if (db.isInDeletedItems(data.strLocation))
        return false;

    QString strTagGUIDs = db.getDocumentTagGuidsString(data.strGUID);
    if (strTagGUIDs.indexOf(m_tag.strGUID) != -1 && data.strKbGUID == kbGUID())
        return true;

    return false;
}

bool WizCategoryViewGroupItem::acceptDrop(const WizCategoryViewItemBase* pItem) const
{
//    return pItem->type() == Category_FolderItem | pItem->type() == Category_GroupItem;

    if (pItem->kbGUID() != kbGUID()) {
        return false;
    }
    //
    return true;
}

bool WizCategoryViewGroupItem::acceptDrop(const WIZDOCUMENTDATA& data) const
{
    Q_UNUSED(data);

    WizDatabase& db = WizDatabaseManager::instance()->db(kbGUID());
    if (WIZ_USERGROUP_AUTHOR >= db.permission()) {
        return true;
    }
    //

    return false;
}

bool WizCategoryViewGroupItem::acceptDrop(const QString& urls) const
{
    Q_UNUSED(urls);
    WizDatabase& db = m_app.databaseManager().db(kbGUID());

    return WIZ_USERGROUP_AUTHOR >= db.permission();
}

void WizCategoryViewGroupItem::drop(const CWizDocumentDataArray& arrayDocument, bool forceCopy)
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


    WizDocumentOperator documentOperator(m_app.databaseManager());

    if (needCopy)
    {
        documentOperator.copyDocumentsToGroupFolder(arrayOp, m_tag, false, true);
    }
    else
    {
        documentOperator.moveDocumentsToGroupFolder(arrayOp, m_tag, true);
    }
}

QString WizCategoryViewGroupItem::id() const
{
    return ::WizMd5StringNoSpaceJava(QString(text(0) + m_tag.strGUID).toUtf8());
}

bool WizCategoryViewGroupItem::operator<(const QTreeWidgetItem& other) const
{
    if (m_app.userSettings().isManualSortingEnabled())
    {
        const WizCategoryViewGroupItem* pOther = dynamic_cast<const WizCategoryViewGroupItem*>(&other);
        if (pOther)
        {
            if (m_tag.nPosition == pOther->m_tag.nPosition || m_tag.nPosition == 0 || pOther->m_tag.nPosition == 0)
                return WizCategoryViewItemBase::operator <(other);

            return m_tag.nPosition < pOther->m_tag.nPosition;
        }
    }

    return WizCategoryViewItemBase::operator <(other);
}

void WizCategoryViewGroupItem::reload(WizDatabase& db)
{
    db.tagFromGuid(m_tag.strGUID, m_tag);
    setText(0, m_tag.strName);
    m_strName = m_tag.strName;
}

void WizCategoryViewGroupItem::setTagPosition(int nPos)
{
    m_tag.nPosition = nPos;
}

/* ------------------------------ CWizCategoryViewTrashItem ------------------------------ */

WizCategoryViewTrashItem::WizCategoryViewTrashItem(WizExplorerApp& app,
                                                     const QString& strKbGUID)
    : WizCategoryViewFolderItem(app, "/Deleted Items/", strKbGUID)
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "category_trash", QSize(), ICON_OPTIONS);
    setIcon(0, icon);
    setText(0, PREDEFINED_TRASH);
}

void WizCategoryViewTrashItem::showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
{
    if (WizCategoryView* view = dynamic_cast<WizCategoryView *>(pCtrl)) {
        //no menu
        //view->showTrashContextMenu(pos);
    }
}

void WizCategoryViewTrashItem::getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    db.getDocumentsByLocation(db.getDeletedItemsLocation(), arrayDocument, true);
}

bool WizCategoryViewTrashItem::accept(WizDatabase& db, const WIZDOCUMENTDATA& data)
{
    if (kbGUID() != data.strKbGUID)
        return false;
    //
    return db.isInDeletedItems(data.strLocation);
}

bool WizCategoryViewTrashItem::acceptDrop(const WIZDOCUMENTDATA &data) const
{
    Q_UNUSED(data);

    WizCategoryViewGroupRootItem* parentItem = dynamic_cast<WizCategoryViewGroupRootItem*>(parent());
    if (parentItem)
        return false;

    return true;
}

bool WizCategoryViewTrashItem::acceptDrop(const WizCategoryViewItemBase* pItem) const
{
    return false;
}

void WizCategoryViewTrashItem::drop(const CWizDocumentDataArray& arrayDocument, bool forceCopy)
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

    WizDocumentOperator documentOperator(m_app.databaseManager());
    documentOperator.deleteDocuments(arrayOp);
}


WizCategoryViewShortcutItem::WizCategoryViewShortcutItem(WizExplorerApp& app,
                                                           const QString& strName, ShortcutType type, const QString& strKbGuid,
                                                           const QString& strGuid, const QString& location, bool bEncrypted)
    : WizCategoryViewItemBase(app, strName, strKbGuid, Category_ShortcutItem)
    , m_strGuid(strGuid)
    , m_type(type)
    , m_location(location)
    , m_bEncrypted(bEncrypted)
{
    QIcon icon;
    switch (type) {
    case Document:
    {
        if (bEncrypted)
        {
            icon = WizLoadSkinIcon(app.userSettings().skin(), "document_badge_encrypted", QSize(), ICON_OPTIONS);
        }
        else
        {
            icon = WizLoadSkinIcon(app.userSettings().skin(), "document_badge", QSize(), ICON_OPTIONS);
        }
    }
        break;
    case PersonalFolder:
    case GroupTag:
    {
        icon = WizLoadSkinIcon(app.userSettings().skin(), "category_folder", QSize(), ICON_OPTIONS);
    }
        break;
    case PersonalTag:
    {
        icon = WizLoadSkinIcon(app.userSettings().skin(), "category_tag", QSize(), ICON_OPTIONS);
    }
        break;
    }

    //
    setIcon(0, icon);
    setText(0, strName);
}

void WizCategoryViewShortcutItem::showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
{
    if (WizCategoryView* view = dynamic_cast<WizCategoryView *>(pCtrl)) {
        view->showShortcutContextMenu(pos);
    }
}

bool WizCategoryViewShortcutItem::accept(WizDatabase& db, const WIZDOCUMENTDATA& data)
{
    switch (m_type) {
    case Document:
        //现在笔记列表会显示快捷方式中笔记所在文件夹的所有笔记，所以允许该文件夹下所有笔记
    {
        if(db.isGroup())
        {
            CWizStdStringArray arrayTag1;
            db.getDocumentTags(data.strGUID, arrayTag1);
            CWizStdStringArray arrayTag2;
            m_app.databaseManager().db(m_strKbGUID).getDocumentTags(m_strGuid, arrayTag2);

            return (arrayTag1.size() == 1 && arrayTag2.size() == 1 && arrayTag1.front() == arrayTag2.front());
        }
        else
        {
            WIZDOCUMENTDATA doc;
            m_app.databaseManager().db(m_strKbGUID).documentFromGuid(m_strGuid, doc);
            return doc.strLocation == data.strLocation;
        }
    }
        break;
    case PersonalFolder:
        return data.strLocation == m_location;
        break;
    case PersonalTag:
    {
        CWizStdStringArray arrayTag;
        m_app.databaseManager().db().getDocumentTags(data.strGUID, arrayTag);
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
        m_app.databaseManager().db(data.strKbGUID).getDocumentTags(data.strGUID, arrayTag);
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


WizCategoryViewShortcutPlaceHoldItem::WizCategoryViewShortcutPlaceHoldItem(
        WizExplorerApp& app, const QString& strName)
    : WizCategoryViewItemBase(app, strName, "", Category_ShortcutPlaceHoldItem)
{

}

int WizCategoryViewShortcutPlaceHoldItem::getItemHeight(int /*hintHeight*/) const
{
    return WizSmartScaleUI(20);
}

void WizCategoryViewShortcutPlaceHoldItem::drawItemBody(QPainter *p, const QStyleOptionViewItem *vopt) const
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
        f.setPixelSize(::WizSmartScaleUI(10));
        f.setStyleStrategy(QFont::PreferBitmap);
        QFontMetrics fm(f);
        strText = fm.elidedText(strText, Qt::ElideRight, rcText.width());
        int right = Utils::WizStyleHelper::drawSingleLineText(p, rcText, strText, Qt::AlignVCenter, colorText, f);
        //
        if (right != -1) {
            rcText.setLeft(right + 4);
        }
    }
}


WizCategoryViewSearchItem::WizCategoryViewSearchItem(WizExplorerApp& app,
                                                       const QString& strName, int type)
    : WizCategoryViewItemBase(app, strName, "", type)
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "category_searchItem", QSize(), ICON_OPTIONS);
    setIcon(0, icon);
    setText(0, strName);    
}

void WizCategoryViewSearchItem::showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
{
    if (WizCategoryView* view = dynamic_cast<WizCategoryView *>(pCtrl)) {
        view->showCustomSearchContextMenu(pos, false);
    }
}

bool WizCategoryViewTimeSearchItem::operator<(const QTreeWidgetItem& other) const
{
    const WizCategoryViewTimeSearchItem* pOther = dynamic_cast<const WizCategoryViewTimeSearchItem*>(&other);
    if (!pOther) {
        return false;
    }

    return m_dateInterval < pOther->m_dateInterval;
}


WizCategoryViewTimeSearchItem::WizCategoryViewTimeSearchItem(WizExplorerApp& app,
                                                               const QString& strName, const QString strSelectParam, DateInterval interval)
    : WizCategoryViewSearchItem(app, strName)
    , m_dateInterval(interval)
    , m_strSelectParam(strSelectParam)
{
}

QString WizCategoryViewTimeSearchItem::getSQLWhere()
{
    WizOleDateTime dt;
    switch (m_dateInterval) {
    case DateInterval_Today:
        dt = dt.addDays(-1);
        break;
    case DateInterval_Yesterday:
        dt = dt.addDays(-2);
        break;
    case DateInterval_TheDayBeforeYesterday:
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


WizCategoryViewCustomSearchItem::WizCategoryViewCustomSearchItem(WizExplorerApp& app,
                                                                   const QString& strName, const QString strSelectParam,
                                                                   const QString strSqlWhere, const QString& strGuid,
                                                                   const QString& keyword, int searchScope)
    : WizCategoryViewSearchItem(app, strName, Category_QuickSearchCustomItem)
    , m_strSelectParam(strSelectParam)
    , m_strSQLWhere(strSqlWhere)
    , m_strKeywrod(keyword)
    , m_nSearchScope(searchScope)
{
    m_strKbGUID = strGuid;
}

void WizCategoryViewCustomSearchItem::showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
{
    if (WizCategoryView* view = dynamic_cast<WizCategoryView *>(pCtrl)) {
        view->showCustomSearchContextMenu(pos, true);
    }
}

QString WizCategoryViewCustomSearchItem::getSQLWhere()
{
    return m_strSQLWhere;
}

void WizCategoryViewCustomSearchItem::setSQLWhere(const QString& strSql)
{
    m_strSQLWhere = strSql;
}

QString WizCategoryViewCustomSearchItem::getSelectParam()
{
    return m_strSelectParam;
}

void WizCategoryViewCustomSearchItem::setSelectParam(const QString& strParam)
{
    m_strSelectParam = strParam;
}

void WizCategoryViewCustomSearchItem::setKeyword(const QString& strKeyword)
{
    m_strKeywrod = strKeyword;
}

QString WizCategoryViewCustomSearchItem::getKeyword()
{
    return m_strKeywrod;
}
int WizCategoryViewCustomSearchItem::searchScope() const
{
    return m_nSearchScope;
}

void WizCategoryViewCustomSearchItem::setSearchScope(int nSearchScope)
{
    m_nSearchScope = nSearchScope;
}

void WizCategoryViewLinkItem::drawItemBody(QPainter *p, const QStyleOptionViewItem *vopt) const
{
    QString str = vopt->text;
    QRect rc(vopt->rect);
    rc.setLeft(rc.left() + 16);
    QFont fontLink = p->font();
    //fontLink.setItalic(true);
    fontLink.setPixelSize(::WizSmartScaleUI(12));
    Utils::WizStyleHelper::drawSingleLineText(p, rc, str, Qt::AlignTop, Utils::WizStyleHelper::treeViewItemLinkText(), fontLink);
}


WizCategoryViewMySharesItem::WizCategoryViewMySharesItem(WizExplorerApp& app, const QString& strName)
    : WizCategoryViewItemBase(app, strName, "", Category_MySharesItem)
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "category_share", QSize(), ICON_OPTIONS);
    setIcon(0, icon);
    setText(0, strName);
}

QString WizCategoryViewMySharesItem::getSectionName()
{
    return WIZ_CATEGORY_SECTION_GENERAL;
}
