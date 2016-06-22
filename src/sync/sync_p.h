#ifndef WIZSERVICE_SYNC_P_H
#define WIZSERVICE_SYNC_P_H

#include "wizKMServer.h"

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
    bool DownloadDocumentList(__int64 nServerVersion);
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
    IWizSyncableDatabase* m_pDatabase;
    WIZUSERINFOBASE m_info;
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
        //
        __int64 nNextVersion = nVersion + 1;
        int nCountPerPage = 200;
        //
        while (1)
        {
            std::deque<TData> arrayPageData;
            //
            //QString strProgress = WizFormatString1(::WizTranslationsTranslateString(_T("Start Version: %1")), WizInt64ToStr(nNextVersion));
            //m_pProgress->OnText(wizhttpstatustypeNormal, strProgress);
            //
            if (!m_server.getList<TData>(nCountPerPage, nNextVersion, arrayPageData))
            {
                TOLOG2(_T("Failed to get object list: CountPerPage=%1, Version=%2"), WizIntToStr(nCountPerPage), WizInt64ToStr(nVersion));
                return FALSE;
            }
            //
            if (arrayPageData.empty())
                break;
            //
            for (TData& data : arrayPageData)
            {
                data.strKbGUID = m_info.strKbGUID;
            }
            //
            if (!OnDownloadList<TData>(arrayPageData))
                return FALSE;
            //
            nNextVersion = GetObjectsVersion<TData>(nNextVersion, arrayPageData);
            //
            if (!m_pDatabase->SetObjectVersion(strObjectType, nNextVersion))
                return FALSE;

            //
            if (int(arrayPageData.size()) < nCountPerPage)
                break;
            //
            nNextVersion++;
        }
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
        return m_pDatabase->OnDownloadDocumentList(arrayData);
    }
    template <class TData>
    bool OnDownloadList(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayData)
    {
        return m_pDatabase->OnDownloadAttachmentList(arrayData);
    }
};

#endif // WIZSERVICE_SYNC_P_H
