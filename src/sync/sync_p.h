#ifndef WIZSERVICE_SYNC_P_H
#define WIZSERVICE_SYNC_P_H

#include "wizKMServer.h"

struct WIZDOCUMENTDATAEX_XMLRPC_SIMPLE;

class CWizKMSync
{
public:
    CWizKMSync(IWizSyncableDatabase* pDatabase, const WIZUSERINFOBASE& info,
               IWizKMSyncEvents* pEvents, bool bGroup, bool bUploadOnly, QObject* parent);
public:
    bool Sync();
    bool DownloadObjectData();

protected:
    bool SyncCore();
    bool UploadValue(const QString& strKey);
    bool DownloadValue(const QString& strKey);

    bool DownloadDeletedList(__int64 nServerVersion);
    bool DownloadTagList(__int64 nServerVersion);
    bool DownloadStyleList(__int64 nServerVersion);
    bool DownloadSimpleDocumentList(__int64 nServerVersion);
    bool DownloadFullDocumentList();
    bool DownloadAttachmentList(__int64 nServerVersion);

    bool UploadDeletedList();
    bool UploadTagList();
    bool UploadStyleList();
    bool UploadDocumentList();
    bool UploadAttachmentList();

    bool UploadKeys();
    bool DownloadKeys();
    bool ProcessOldKeyValues();

private:
    std::deque<WIZDOCUMENTDATAEX_XMLRPC_SIMPLE> m_arrayDocumentNeedToBeDownloaded;
    int CalDocumentPartForDownloadToLocal(const WIZDOCUMENTDATAEX_XMLRPC_SIMPLE& data);

private:
    IWizSyncableDatabase* m_pDatabase;
    WIZUSERINFOBASE m_info;
    WIZKBINFO m_kbInfo;
    IWizKMSyncEvents* m_pEvents;
    bool m_bGroup;
    bool m_bUploadOnly;

    CWizKMDatabaseServer m_server;

    std::map<QString, WIZKEYVALUEDATA> m_mapOldKeyValues;

public:
    template <class TData>
    static __int64 GetObjectsVersion(__int64 nOldVersion, const std::deque<TData>& arrayData)
    {
        if (arrayData.empty())
            return nOldVersion;
        //
        __int64 nVersion = nOldVersion;
        for (typename std::deque<TData>::const_iterator it = arrayData.begin();
            it != arrayData.end();
            it++)
        {
            nVersion = std::max<__int64>(nOldVersion, it->nVersion);
        }
        return nVersion;
    }

private:
    template <class TData>
    bool DownloadList(__int64 nServerVersion, const QString& strObjectType, WizKMSyncProgress progress)
    {
        m_pEvents->OnSyncProgress(::GetSyncStartProgress(progress));
        //
        __int64 nVersion = m_pDatabase->GetObjectVersion(strObjectType);
        if (nServerVersion == nVersion)
        {
            m_pEvents->OnStatus(QObject::tr("No change, skip"));
            return TRUE;
        }
        //
        std::deque<TData> arrayData;
        if (!m_server.getAllList<TData>(200, nVersion, arrayData))
            return FALSE;
        //
        if (!OnDownloadList<TData>(arrayData))
            return FALSE;
        //
        for (typename std::deque<TData>::iterator it = arrayData.begin();
             it != arrayData.end();
             it++)
        {
            it->strKbGUID = m_info.strKbGUID;
        }
        //
        nVersion = GetObjectsVersion<TData>(nVersion, arrayData);
        //
        nVersion = std::max<__int64>(nVersion, nServerVersion);
        //
        return m_pDatabase->SetObjectVersion(strObjectType, nVersion);
    }

    template <class TData>
    bool OnDownloadList(const std::deque<TData>& arrayData)
    {
        ATLASSERT(FALSE);
        return FALSE;
    }
    template <class TData>
    bool OnDownloadList(const std::deque<WIZDELETEDGUIDDATA>& arrayData)
    {
        return m_pDatabase->OnDownloadDeletedList(arrayData);
    }
    template <class TData>
    bool OnDownloadList(const std::deque<WIZTAGDATA>& arrayData)
    {
        return m_pDatabase->OnDownloadTagList(arrayData);
    }
    template <class TData>
    bool OnDownloadList(const std::deque<WIZSTYLEDATA>& arrayData)
    {
        return m_pDatabase->OnDownloadStyleList(arrayData);
    }
    template <class TData>
    bool OnDownloadList(const std::deque<WIZDOCUMENTDATAEX>& arrayData)
    {
        m_arrayDocumentNeedToBeDownloaded.clear();
        m_arrayDocumentNeedToBeDownloaded.assign(arrayData.begin(), arrayData.end());
        //
        return DownloadFullDocumentList();
    }
    template <class TData>
    bool OnDownloadList(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayData)
    {
        return m_pDatabase->OnDownloadAttachmentList(arrayData);
    }
};

#endif // WIZSERVICE_SYNC_P_H
