#ifndef WIZCATEGORYVIEWITEM_H
#define WIZCATEGORYVIEWITEM_H

#include <QTreeWidgetItem>

#include "share/WizObject.h"

enum ItemType
{
    Category_WizNoneItem = QTreeWidgetItem::UserType + 1,
    Category_MessageRootItem,
    Category_MessageItem,
    Category_ShortcutRootItem,
    Category_ShortcutPlaceHoldItem,
    Category_ShortcutItem,
    Category_QuickSearchRootItem,
    Category_QuickSearchItem,
    Category_QuickSearchCustomItem,
    Category_AllFoldersItem,
    Category_FolderItem,
    Category_AllTagsItem,
    Category_TagItem,
    Category_GroupsRootItem,
    Category_BizGroupRootItem,
    Category_OwnGroupRootItem,
    Category_JoinedGroupRootItem,
    Category_GroupRootItem,
    Category_GroupItem,
    Category_GroupNoTagItem,
    Category_SectionItem,
    Category_MySharesItem,
};

enum DateInterval{
    DateInterval_Today,
    DateInterval_Yesterday,
    DateInterval_TheDayBeforeYesterday,
    DateInterval_LastWeek,
    DateInterval_LastMonth,
    DateInterval_LastYear
};

class WizDatabase;
class WizExplorerApp;
class WizCategoryBaseView;

class WizCategoryViewItemBase : public QTreeWidgetItem
{
public:
    WizCategoryViewItemBase(WizExplorerApp& app, const QString& strName = "", const QString& strKbGUID = "", int type = Type);
    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos) = 0;
    virtual void getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument) = 0;
    virtual bool accept(WizDatabase& db, const WIZDOCUMENTDATA& data) { Q_UNUSED(data); return false; }
    virtual bool acceptDrop(const WizCategoryViewItemBase* pItem) const { Q_UNUSED(pItem); return false;}
    virtual bool acceptDrop(const WIZDOCUMENTDATA& data) const { Q_UNUSED(data); return false; }
    virtual bool acceptDrop(const QString& urls) const { Q_UNUSED(urls); return false; }
    virtual bool dragAble() const { return false; }
    virtual void drop(const CWizDocumentDataArray& arrayDocument, bool forceCopy = false) { Q_UNUSED(arrayDocument); Q_UNUSED(forceCopy);}
    virtual void drop(const WizCategoryViewItemBase* pItem) { Q_UNUSED(pItem); }

    virtual bool acceptMousePressedInfo() { return false; }
    virtual void mousePressed(const QPoint& pos) { Q_UNUSED(pos); }
    virtual void mouseReleased(const QPoint& pos) { Q_UNUSED(pos); }

    virtual void drawItemBody(QPainter* p, const QStyleOptionViewItem* vopt) const;
    virtual void drawExtraBadge(QPainter* p, const QStyleOptionViewItem* vopt) const;

    virtual QVariant data(int column, int role) const;
    virtual int getItemHeight(int hintHeight) const;
    virtual bool operator<(const QTreeWidgetItem &other) const;

    const QString& kbGUID() const { return m_strKbGUID; }
    const QString& name() const { return m_strName; }

    virtual QString id() const;

    void setDocumentsCount(int nCurrent, int nTotal);

    //
    virtual int getSortOrder() const { return 0; }

    //
    virtual QString getSectionName() { return QString(); }

    //Extra Button
    virtual void setExtraButtonIcon(const QString &file);
    virtual bool getExtraButtonIcon(QPixmap &ret) const;
    virtual QRect getExtraButtonRect(const QRect &itemBorder, bool ignoreIconExist = false) const;
    virtual bool extraButtonClickTest();
    virtual QString getExtraButtonToolTip() const;
    //
    virtual bool isWebView() const { return false; }
    //
    virtual QString countString() const { return m_countString; }

protected:
    WizExplorerApp& m_app;
    QString m_strName;
    QString m_strKbGUID;
    QPixmap m_extraButtonIcon;
    QString m_countString;
    bool m_extraButtonIconPressed;
};



class WizCategoryViewSectionItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewSectionItem(WizExplorerApp& app, const QString& strName, int sortOrder);
    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos) { Q_UNUSED(pCtrl); Q_UNUSED(pos); }
    virtual void getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument) {  Q_UNUSED(arrayDocument); }
    virtual int getItemHeight(int nHeight) const;
    virtual int getSortOrder() const { return m_sortOrder; }
    void reset(const QString& sectionName, int sortOrder);

    virtual void drawItemBody(QPainter* p, const QStyleOptionViewItem* vopt) const;
    virtual void drawExtraBadge(QPainter* p, const QStyleOptionViewItem* vopt) const;
protected:
    int m_sortOrder;
};

class WizCategoryViewMessageItem : public WizCategoryViewItemBase
{
public:
    enum FilterType {
        All,
        SendToMe,
        ModifyNote,
        comment,
        SendFromMe
    };

    WizCategoryViewMessageItem(WizExplorerApp& app, const QString& strName, int nFilter);
    virtual void drawExtraBadge(QPainter* p, const QStyleOptionViewItem *vopt) const;

    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
    { Q_UNUSED(pCtrl); Q_UNUSED(pos); }

    virtual void getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument)
    {  Q_UNUSED(arrayDocument); }


    virtual bool acceptMousePressedInfo() { return true; }
    virtual void mousePressed(const QPoint& pos);
    virtual void mouseReleased(const QPoint& pos);

    void getMessages(WizDatabase& db, const QString& userGUID, CWizMessageDataArray& arrayMsg);
    void setUnreadCount(int nCount);
    QString unreadString() const;
    bool hitTestUnread();

    virtual QString getSectionName();
    virtual int getSortOrder() const { return 10; }

    virtual QRect getExtraButtonRect(const QRect &itemBorder, bool ignoreIconExist = false) const;

//    void showCoachingTips();
private:
    int m_nFilter;
    int m_nUnread;
    QSize m_szUnreadSize;   
};

class WizCategoryViewShortcutItem;
class WizCategoryViewShortcutRootItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewShortcutRootItem(WizExplorerApp& app, const QString& strName);

    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
    { Q_UNUSED(pCtrl); Q_UNUSED(pos); }

    virtual void getDocuments(WizDatabase& db,
                              CWizDocumentDataArray& arrayDocument);

    virtual bool accept(WizDatabase& db, const WIZDOCUMENTDATA& data);
    virtual void drop(const CWizDocumentDataArray& arrayDocument, bool forceCopy = false);
    virtual void drop(const WizCategoryViewItemBase* pItem);
    virtual bool acceptDrop(const WIZDOCUMENTDATA& /*data*/) const {return true;}
    virtual bool acceptDrop(const WizCategoryViewItemBase* pItem) const;

    WizCategoryViewShortcutItem* addItemToShortcuts(const WizCategoryViewItemBase* pItem);
    WizCategoryViewShortcutItem* addDocumentToShortcuts(const WIZDOCUMENTDATA& document);

    virtual QString getSectionName();
    virtual int getSortOrder() const { return 11; }

    void addPlaceHoldItem();
    bool isContainsPlaceHoldItem();
    void removePlaceHoldItem();
};

class WizCategoryViewShortcutPlaceHoldItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewShortcutPlaceHoldItem(WizExplorerApp& app, const QString& strName);

    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos) {}
    virtual void getDocuments(WizDatabase& db,
                              CWizDocumentDataArray& arrayDocument)
    { Q_UNUSED(arrayDocument); }

    virtual int getItemHeight(int hintHeight) const;

    virtual void drawItemBody(QPainter* p, const QStyleOptionViewItem* vopt) const;
};

class WizCategoryViewShortcutItem : public WizCategoryViewItemBase
{
public:
    enum ShortcutType
    {
        Document,
        PersonalFolder,
        PersonalTag,
        GroupTag
    };
    //
    WizCategoryViewShortcutItem(WizExplorerApp& app, const QString& strName, ShortcutType type,
                                 const QString& strKbGuid, const QString& strGuid, const QString& location, bool bEncrypted = false);

    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos);
    virtual void getDocuments(WizDatabase& db,
                              CWizDocumentDataArray& arrayDocument)
    { Q_UNUSED(arrayDocument); }

    virtual bool accept(WizDatabase& db, const WIZDOCUMENTDATA& data);

    QString guid() const {return m_strGuid;}
    QString location() const { return m_location; }
    ShortcutType shortcutType() const { return m_type; }

private:
    QString m_strGuid;
    QString m_location;
    ShortcutType m_type;
};

class WizCategoryViewSearchRootItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewSearchRootItem(WizExplorerApp& app, const QString& strName);

    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos);

    virtual void getDocuments(WizDatabase& db,
                              CWizDocumentDataArray& arrayDocument)
    { Q_UNUSED(arrayDocument); }

    virtual QString getSectionName();
    virtual int getSortOrder() const { return 12; }
};

class WizCategoryViewSearchItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewSearchItem(WizExplorerApp& app, const QString& strName,
                               int type = Category_QuickSearchItem);

    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos);

    virtual void getDocuments(WizDatabase& db,
                              CWizDocumentDataArray& arrayDocument)
    { Q_UNUSED(arrayDocument); }

    virtual QString getSQLWhere() { return ""; }
    virtual QString getSelectParam() { return ""; }
};

class WizCategoryViewTimeSearchItem : public WizCategoryViewSearchItem
{
public:
    WizCategoryViewTimeSearchItem(WizExplorerApp& app, const QString& strName,
                               const QString strSelectParam, DateInterval interval);        

    virtual bool operator<(const QTreeWidgetItem &other) const;

    virtual QString getSQLWhere();

protected:
    QString m_strSelectParam;
    DateInterval m_dateInterval;
};

class WizCategoryViewCustomSearchItem : public WizCategoryViewSearchItem
{
public:
    WizCategoryViewCustomSearchItem(WizExplorerApp& app, const QString& strName,
                               const QString strSelectParam, const QString strSqlWhere,
                                     const QString& strGuid, const QString& keyword, int searchScope);

    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos);

    virtual QString getSQLWhere();
    virtual void setSQLWhere(const QString& strSql);
    virtual QString getSelectParam();
    virtual void setSelectParam(const QString& strParam);
    void setKeyword(const QString& strKeyword);
    QString getKeyword();
    int searchScope() const;
    void setSearchScope(int searchScope);

protected:
    QString m_strSelectParam;
    QString m_strSQLWhere;
    QString m_strKeywrod;
    int m_nSearchScope;
};

class WizCategoryViewAllFoldersItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewAllFoldersItem(WizExplorerApp& app, const QString& strName, const QString& strKbGUID);
    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos);
    virtual void getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(WizDatabase& db, const WIZDOCUMENTDATA& data);
    virtual bool acceptDrop(const WizCategoryViewItemBase* pItem) const;
    virtual bool acceptDrop(const QString& urls) const { Q_UNUSED(urls); return true; }
    virtual QString getSectionName();
    virtual int getSortOrder() const { return 20; }
};

class WizCategoryViewFolderItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewFolderItem(WizExplorerApp& app, const QString& strLocation, const QString& strKbGUID);
    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos);
    virtual void getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(WizDatabase& db, const WIZDOCUMENTDATA& data);
    virtual bool acceptDrop(const WIZDOCUMENTDATA& data) const;
    virtual bool acceptDrop(const WizCategoryViewItemBase* pItem) const;
    virtual bool acceptDrop(const QString& urls) const { Q_UNUSED(urls); return true; }
    virtual bool dragAble() const { return true; }
    virtual void drop(const CWizDocumentDataArray& arrayDocument, bool forceCopy = false);

    virtual bool operator < (const QTreeWidgetItem &other) const;

    virtual QTreeWidgetItem* clone() const;

    virtual QString id() const;

    QString location() const { return m_strName; }
    void setLocation(const QString& strLocation);
    QString name() const;

private:
    QRect m_rcUnread;
};

class WizCategoryViewAllTagsItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewAllTagsItem(WizExplorerApp& app, const QString& strName, const QString& strKbGUID);
    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos);
    virtual void getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(WizDatabase& db, const WIZDOCUMENTDATA& data);
    virtual bool acceptDrop(const WizCategoryViewItemBase* pItem) const;
    virtual QString getSectionName();
    virtual int getSortOrder() const { return 21; }
};

class WizCategoryViewTagItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewTagItem(WizExplorerApp& app, const WIZTAGDATA& tag, const QString& strKbGUID);
    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos);
    virtual void getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(WizDatabase& db, const WIZDOCUMENTDATA& data);
    virtual bool acceptDrop(const WIZDOCUMENTDATA& data) const;
    virtual bool acceptDrop(const WizCategoryViewItemBase* pItem) const;
    virtual bool dragAble() const { return true; }
    virtual void drop(const CWizDocumentDataArray& arrayDocument, bool forceCopy = false);
    virtual void drop(const WizCategoryViewItemBase* pItem);

    virtual QTreeWidgetItem *clone() const;

    void reload(WizDatabase& db);
    void setTagPosition(int nPos);
    const WIZTAGDATA& tag() const { return m_tag; }

private:
    WIZTAGDATA m_tag;
};

class WizCategoryViewStyleRootItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewStyleRootItem(WizExplorerApp& app, const QString& strName);

    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
    { Q_UNUSED(pCtrl); Q_UNUSED(pos); }

    virtual void getDocuments(WizDatabase& db,
                              CWizDocumentDataArray& arrayDocument)
    { Q_UNUSED(arrayDocument); }
    virtual QString getSectionName();
    virtual int getSortOrder() const { return 22; }
};

class WizCategoryViewGroupsRootItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewGroupsRootItem(WizExplorerApp& app, const QString& strName);

    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos);

    virtual void getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(WizDatabase& db, const WIZDOCUMENTDATA& data);
//    virtual bool acceptDrop(const CWizCategoryViewItemBase* pItem) const;
    virtual QString getSectionName();

};

class WizCategoryViewBizGroupRootItem : public WizCategoryViewGroupsRootItem
{
public:
    WizCategoryViewBizGroupRootItem(WizExplorerApp& app,
                                     const WIZBIZDATA& biz);

    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos);
    //
    const WIZBIZDATA biz() const { return m_biz; }
    virtual int getSortOrder() const { return 30; }    
    //
    virtual void getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual void drawExtraBadge(QPainter* p, const QStyleOptionViewItem* vopt) const;

    //
    virtual bool acceptMousePressedInfo() { return true; }
    virtual void mousePressed(const QPoint& pos);
    virtual void mouseReleased(const QPoint& pos);

    //
    bool isExtraButtonUseable() const;
    bool isUnreadButtonUseable() const;
    void updateUnreadCount();
    QString unreadString() const;
    bool hitTestUnread();
    virtual QString getExtraButtonToolTip() const;
    virtual QRect getExtraButtonRect(const QRect &itemBorder, bool ignoreIconExist = false) const;
    //
    bool isOwner();
    bool isAdmin();
    bool isHr();


private:
    WIZBIZDATA m_biz;
    int m_unReadCount;
    QSize m_szUnreadSize;
};
class WizCategoryViewOwnGroupRootItem : public WizCategoryViewGroupsRootItem
{
public:
    WizCategoryViewOwnGroupRootItem(WizExplorerApp& app);

    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos);

    virtual int getSortOrder() const { return 31; }
};

class WizCategoryViewJionedGroupRootItem : public WizCategoryViewGroupsRootItem
{
public:
    WizCategoryViewJionedGroupRootItem(WizExplorerApp& app);
    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
    { Q_UNUSED(pCtrl); Q_UNUSED(pos); }

    virtual int getSortOrder() const { return 32; }
};

class WizCategoryViewLinkItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewLinkItem(WizExplorerApp& app, const QString& strName, int commandId)
        : WizCategoryViewItemBase(app, strName)
        , m_commandId(commandId) { setFlags(Qt::NoItemFlags); setText(0, strName); }

    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
    { Q_UNUSED(pCtrl); Q_UNUSED(pos); }
    virtual void getDocuments(WizDatabase& db,
                              CWizDocumentDataArray& arrayDocument)
    { Q_UNUSED(arrayDocument); }
    int commandId() const { return m_commandId; }

    virtual void drawItemBody(QPainter* p, const QStyleOptionViewItem* vopt) const;

protected:
    int m_commandId;
};

class WizCategoryViewCreateGroupLinkItem : public WizCategoryViewLinkItem
{
public:
    WizCategoryViewCreateGroupLinkItem(WizExplorerApp& app, const QString& strName, int commandId)
        : WizCategoryViewLinkItem(app, strName, commandId) {}
    //
    virtual QString getSectionName();
    virtual int getSortOrder() const { return 29; }


};


class WizCategoryViewGroupRootItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewGroupRootItem(WizExplorerApp& app,
                                  const WIZGROUPDATA& group);

    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos);
    virtual void getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(WizDatabase& db, const WIZDOCUMENTDATA& data);
    virtual bool acceptDrop(const WIZDOCUMENTDATA& data) const;
    virtual bool acceptDrop(const WizCategoryViewItemBase* pItem) const;
    virtual bool acceptDrop(const QString& urls) const;
    virtual void drop(const CWizDocumentDataArray& arrayDocument, bool forceCopy = false);
    virtual void drawExtraBadge(QPainter* p, const QStyleOptionViewItem* vopt) const;
    void reload(WizDatabase& db);
    //
    virtual bool acceptMousePressedInfo() { return true; }
    virtual void mousePressed(const QPoint& pos);
    virtual void mouseReleased(const QPoint& pos);    
    //
    virtual bool operator<(const QTreeWidgetItem &other) const;
    //
    bool isAdmin(WizDatabase& db);
    bool isOwner(WizDatabase& db);

    bool isBizGroup() const;
    QString bizGUID() const;
    //
    void setUnreadCount(int nCount);
    int getUnreadCount();
    QString unreadString() const;
    bool hitTestUnread();
    virtual QString getExtraButtonToolTip() const;
    virtual QRect getExtraButtonRect(const QRect &itemBorder, bool ignoreIconExist = false) const;

private:
    WIZGROUPDATA m_group;
    int m_nUnread;
    QSize m_szUnreadSize;
};

class WizCategoryViewGroupNoTagItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewGroupNoTagItem(WizExplorerApp& app, const QString& strKbGUID);
    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos) {Q_UNUSED(pCtrl); Q_UNUSED(pos);}
    virtual void getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(WizDatabase& db, const WIZDOCUMENTDATA& data);
    virtual int getSortOrder() const { return 10; }
};

class WizCategoryViewGroupItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewGroupItem(WizExplorerApp& app, const WIZTAGDATA& tag, const QString& strKbGUID);
    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos);
    virtual void getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(WizDatabase& db, const WIZDOCUMENTDATA& data);
    virtual bool acceptDrop(const WizCategoryViewItemBase* pItem) const;
    virtual bool acceptDrop(const WIZDOCUMENTDATA& data) const;
    virtual bool acceptDrop(const QString& urls) const;
    virtual bool dragAble() const { return true; }
    virtual void drop(const CWizDocumentDataArray& arrayDocument, bool forceCopy = false);

    virtual QString id() const;

    virtual bool operator<(const QTreeWidgetItem &other) const;

    void reload(WizDatabase& db);
    void setTagPosition(int nPos);
    const WIZTAGDATA& tag() const { return m_tag; }

    virtual int getSortOrder() const { return 11; }

private:
    WIZTAGDATA m_tag;
};

class WizCategoryViewTrashItem : public WizCategoryViewFolderItem
{
public:
    WizCategoryViewTrashItem(WizExplorerApp& app, const QString& strKbGUID);
    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos);
    virtual void getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(WizDatabase& db, const WIZDOCUMENTDATA& data);
    virtual bool acceptDrop(const WIZDOCUMENTDATA& data) const;
    virtual bool acceptDrop(const WizCategoryViewItemBase* pItem) const;
    virtual bool dragAble() const { return false; }
    virtual int getSortOrder() const { return 12; }
    virtual bool isWebView() const { return true; }
    //
    virtual void drop(const CWizDocumentDataArray& arrayDocument, bool forceCopy = false);
};


class WizCategoryViewMySharesItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewMySharesItem(WizExplorerApp& app, const QString& strName);
    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
    { Q_UNUSED(pCtrl); Q_UNUSED(pos); }
    virtual void getDocuments(WizDatabase& db,
                              CWizDocumentDataArray& arrayDocument)
    { Q_UNUSED(arrayDocument); }
    virtual QString getSectionName();
    virtual int getSortOrder() const { return 13; }
    virtual bool isWebView() const { return true; }
};

#endif // WIZCATEGORYVIEWITEM_H
