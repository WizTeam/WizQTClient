#ifndef WIZDOCUMENTLISTVIEWITEM_H
#define WIZDOCUMENTLISTVIEWITEM_H

#include <QListWidget>

#include "share/wizobject.h"

class CWizDatabase;
class CWizThumbIndexCache;
class CWizUserAvatarDownloaderHost;

class CWizDocumentListViewItem : public QListWidgetItem
{
public:
    enum type {
        PrivateDocument,
        GroupDocument,
        MessageDocument
    };

    explicit CWizDocumentListViewItem(const WIZDOCUMENTDATA& data);
    explicit CWizDocumentListViewItem(const WIZMESSAGEDATA& msg);

    int type() { return nType; }

    const WIZDOCUMENTDATA& document() const { return m_data; }
    const QString& tags(CWizDatabase& db);
    const WIZABSTRACT& abstract(CWizThumbIndexCache& thumbCache);

    void resetAbstract(const WIZABSTRACT& abs);
    void resetAvatar(const QString& strFileName);
    void reload(CWizDatabase& db);

    // message
    const WIZMESSAGEDATA& message() const { return m_message; }
    const QImage& avatar(const CWizDatabase& db,
                         CWizUserAvatarDownloaderHost& downloader);

    // used for sorting
    virtual bool operator<(const QListWidgetItem &other) const;

private:
    WIZDOCUMENTDATA m_data;
    WIZABSTRACT m_abstract;
    QString m_tags;

    WIZMESSAGEDATA m_message;
    QImage m_imgAvatar;

private:
    bool isAvatarNeedUpdate(const QString& strFileName);

    // public only for quickly drawing access
public:
    int nType;
};

#endif // WIZDOCUMENTLISTVIEWITEM_H
