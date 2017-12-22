#ifndef WIZDOCUMENTLISTVIEWITEM_H
#define WIZDOCUMENTLISTVIEWITEM_H

#include <QListWidgetItem>
#include <QObject>

#include "share/WizObject.h"

class QPixmap;

class WizExplorerApp;
class WizDatabase;
class CWizUserAvatarDownloaderHost;

enum WizDocumentListItemType
{
    WizDocumentListType_Document = QListWidgetItem::UserType + 1,
    WizDocumentListType_Section
};

struct WizDocumentListViewItemData
{
    int nType;
    WIZDOCUMENTDATAEX doc;
    WIZABSTRACT thumb;

    QString location;    // use to sort by location
    QStringList infoList; // for second line info drawing (auto change when sorting type change)

    // only used for group or message document
    qint64 nMessageId;
    int nReadStatus;    // 0: not read 1: read
    QString strAuthorId; // for request author avatar
};

struct WizDocumentListViewSectionData
{
    QDate date;
    QPair<int, int> sizePair;
    QString strInfo;
};


class WizDocumentListViewBaseItem : public QObject, public QListWidgetItem
{
    Q_OBJECT
public:
    explicit WizDocumentListViewBaseItem(QObject* parent, WizDocumentListItemType type);

    virtual void setSortingType(int type);
    virtual void setLeadInfoState(int state);

    // drawing
    virtual void draw(QPainter* p, const QStyleOptionViewItem* vopt, int nViewType) const {}

protected:
    int m_nSortingType;      // upercase : -  decrease : +
    int m_nLeadInfoState;
};

class WizDocumentListViewDocumentItem;
class WizDocumentListViewSectionItem : public WizDocumentListViewBaseItem
{
    Q_OBJECT
public:
    explicit WizDocumentListViewSectionItem(const WizDocumentListViewSectionData& data, const QString& text, int docCount);
    const WizDocumentListViewSectionData& sectionData() const { return m_data; }

    // used for sorting
    virtual bool operator<(const QListWidgetItem &other) const;

    virtual void draw(QPainter* p, const QStyleOptionViewItem* vopt, int nViewType) const;

private:
    bool compareWithDocumentItem(const WizDocumentListViewDocumentItem* docItem) const;

private:
    WizDocumentListViewSectionData m_data;
    QString m_text;
    int m_documentCount;
};

class WizDocumentListViewDocumentItem : public WizDocumentListViewBaseItem
{
    Q_OBJECT

public:
    enum ItemType {
        TypePrivateDocument,
        TypeGroupDocument,
        TypeMessage
    };

    explicit WizDocumentListViewDocumentItem(WizExplorerApp& app,
                                      const WizDocumentListViewItemData& data);

    virtual void setSortingType(int type);
    virtual void setLeadInfoState(int state);

    const WizDocumentListViewItemData& itemData() { return m_data; }
    const WIZDOCUMENTDATA& document() const { return m_data.doc; }
    WIZDOCUMENTDATA& document() { return m_data.doc; }
    int itemType() const { return m_data.nType; }
    int documentSize() const;
    QString documentLocation() const;
    void reload(WizDatabase& db);

    const QImage& avatar(const WizDatabase& db);

    // called by CWizDocumentListView when thumbCache pool is ready for reading
    void resetAbstract(const WIZABSTRACT& abs);
    void resetAvatar(const QString& strFileName);

    // used for sorting
    virtual bool operator<(const QListWidgetItem &other) const;

    // drawing
    void draw(QPainter* p, const QStyleOptionViewItem* vopt, int nViewType) const;
    void setNeedUpdate() const;
    QString cacheKey() const;

    bool isSpecialFocus() const;
    void setSpecialFocused(bool isSpecialFocus);

    void updateDocumentUnreadCount();

Q_SIGNALS:
    void thumbnailReloaded();

private Q_SLOTS:
    void on_thumbnailReloaded();

private:
    void draw_impl(QPainter* p, const QStyleOptionViewItem* vopt, int nItemType, int nViewType) const;
    void drawPrivateSummaryView_impl(QPainter* p, const QStyleOptionViewItem* vopt) const;
    void drawGroupSummaryView_impl(QPainter* p, const QStyleOptionViewItem* vopt) const;
    void drawPrivateTwoLineView_impl(QPainter* p, const QStyleOptionViewItem* vopt) const;
    void drawGroupTwoLineView_impl(QPainter* p, const QStyleOptionViewItem* vopt) const;
    void drawOneLineView_impl(QPainter* p, const  QStyleOptionViewItem* vopt) const;
    void drawSyncStatus(QPainter* p, const QStyleOptionViewItem* vopt, int nViewType) const;

    QRect drawItemBackground(QPainter* p, const QRect& rect, bool selected, bool focused) const;

    bool isAvatarNeedUpdate(const QString& strFileName);
    bool isContainsAttachment() const;

    int badgeType(bool isSummaryView = false) const;

    //bool adjust(const QListWidgetItem &other) const;

    bool compareWithSectionItem(const WizDocumentListViewSectionItem* secItem) const;

    void updateDocumentLocationData();

    bool needDrawDocumentLocation() const;

    void updateInfoList();

private:
    WizExplorerApp& m_app;
    WizDocumentListViewItemData m_data;   

    int m_nSize;
    QString m_strTags;
    QString m_strAuthor;
    const QString& tags();
    const QString& tagTree();


    bool m_specialFocused;
    bool m_documentUnread;

};


#endif // WIZDOCUMENTLISTVIEWITEM_H
