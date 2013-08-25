#ifndef WIZDOCUMENTLISTVIEWITEM_H
#define WIZDOCUMENTLISTVIEWITEM_H

#include <QListWidget>

#include "share/wizobject.h"

class CWizDatabase;
class CWizThumbIndexCache;
class CWizUserAvatarDownloaderHost;

struct WizDocumentListViewItemData
{
    int nType;
    WIZDOCUMENTDATA doc;
    WIZABSTRACT thumb;

    // only used for private document
    QString strTags;

    // only used for group or message document
    qint64 nMessageId;
    int nReadStatus;    // 0: not read 1: read
    QString strAuthorGUID; // for request author avatar
    QImage imgAuthorAvatar;
};

class CWizDocumentListViewItem : public QListWidgetItem
{
public:
    enum ItemType {
        TypePrivateDocument,
        TypeGroupDocument,
        TypeMessage
    };

    explicit CWizDocumentListViewItem(const WizDocumentListViewItemData& data);

    const WizDocumentListViewItemData& data() { return m_data; }
    const WIZDOCUMENTDATA& document() const { return m_data.doc; }
    int itemType() const { return m_data.nType; }
    void reload(CWizDatabase& db);

    const WIZABSTRACT& abstract(CWizThumbIndexCache& thumbCache);

    const QImage& avatar(const CWizDatabase& db,
                         CWizUserAvatarDownloaderHost& downloader);

    const QString& tags(CWizDatabase& db);

    // called by CWizDocumentListView when thumbCache pool is ready for reading
    void resetAbstract(const WIZABSTRACT& abs);
    void resetAvatar(const QString& strFileName);

    // used for sorting
    virtual bool operator<(const QListWidgetItem &other) const;

private:
    WizDocumentListViewItemData m_data;

    bool isAvatarNeedUpdate(const QString& strFileName);
};

#endif // WIZDOCUMENTLISTVIEWITEM_H
