#ifndef WIZDOCUMENTLISTVIEWITEM_H
#define WIZDOCUMENTLISTVIEWITEM_H

#include <QListWidgetItem>
#include <QObject>

#include "share/wizobject.h"

class QPixmap;

class CWizExplorerApp;
class CWizDatabase;
class CWizUserAvatarDownloaderHost;

struct WizDocumentListViewItemData
{
    int nType;
    WIZDOCUMENTDATA doc;
    WIZABSTRACT thumb;

    QString strInfo; // for second line info drawing (auto change when sorting type change)

    // only used for group or message document
    qint64 nMessageId;
    int nReadStatus;    // 0: not read 1: read
    QString strAuthorId; // for request author avatar
    QImage imgAuthorAvatar;
};

class CWizDocumentListViewItem : public QObject, public QListWidgetItem
{
    Q_OBJECT

public:
    enum ItemType {
        TypePrivateDocument,
        TypeGroupDocument,
        TypeMessage
    };

    explicit CWizDocumentListViewItem(CWizExplorerApp& app,
                                      const WizDocumentListViewItemData& data);

    void setSortingType(int type);

    const WizDocumentListViewItemData& data() { return m_data; }
    const WIZDOCUMENTDATA& document() const { return m_data.doc; }
    int itemType() const { return m_data.nType; }
    void reload(CWizDatabase& db);

    const QImage& avatar(const CWizDatabase& db);

    // called by CWizDocumentListView when thumbCache pool is ready for reading
    void resetAbstract(const WIZABSTRACT& abs);
    void resetAvatar(const QString& strFileName);

    // used for sorting
    virtual bool operator<(const QListWidgetItem &other) const;

    // drawing
    void draw(QPainter* p, const QStyleOptionViewItemV4* vopt, int nViewType) const;
    void setNeedUpdate() const;
    QString cacheKey() const;

private:
    QPixmap draw_impl(const QStyleOptionViewItemV4* vopt, int nItemType, int nViewType) const;
    QPixmap drawPrivateSummaryView_impl(const QStyleOptionViewItemV4* vopt) const;
    QPixmap drawGroupSummaryView_impl(const QStyleOptionViewItemV4* vopt) const;
    QPixmap drawPrivateTwoLineView_impl(const QStyleOptionViewItemV4* vopt) const;
    QPixmap drawGroupTwoLineView_impl(const QStyleOptionViewItemV4* vopt) const;
    QPixmap drawOneLineView_impl(const  QStyleOptionViewItemV4* vopt) const;

private:
    CWizExplorerApp& m_app;
    WizDocumentListViewItemData m_data;
    int m_nSortingType;

    int m_nSize;
    QString m_strTags;
    const QString& tags();
    const QString& tagTree();

    bool isAvatarNeedUpdate(const QString& strFileName);

private Q_SLOTS:
    void on_thumbnailReloaded();

Q_SIGNALS:
    void thumbnailReloaded();
};

#endif // WIZDOCUMENTLISTVIEWITEM_H
