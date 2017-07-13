#ifndef WIZSERVICE_SYNC_P_H
#define WIZSERVICE_SYNC_P_H

#include "WizKMServer.h"
#include "utils/WizLogger.h"
#include "share/WizMisc.h"

class WizKMSync
{
public:
    WizKMSync(IWizSyncableDatabase* pDatabase, const WIZUSERINFOBASE& userInfo,
              const WIZKBINFO& kbInfo, const WIZKBVALUEVERSIONS& versions,
               IWizKMSyncEvents* pEvents, bool bGroup, bool bUploadOnly, QObject* parent);
public:
    bool sync();
    bool downloadObjectData();    

protected:
    bool syncCore();
    bool uploadValue(const QString& strKey);
    bool downloadValue(const QString& strKey);

    bool downloadDeletedList(__int64 nServerVersion);
    bool downloadTagList(__int64 nServerVersion);
    bool downloadStyleList(__int64 nServerVersion);
    bool downloadDocumentList(__int64 nServerVersion);
    bool downloadAttachmentList(__int64 nServerVersion);
    bool downloadParamList(__int64 nServerVersion);

    bool uploadDeletedList();
    bool uploadTagList();
    bool uploadStyleList();
    bool uploadDocumentList();
    bool uploadAttachmentList();
    bool uploadParamList();

    bool uploadKeys();
    bool downloadKeys();
    bool processOldKeyValues();

private:
    IWizSyncableDatabase* m_pDatabase;
    WIZUSERINFOBASE m_userInfo;
    IWizKMSyncEvents* m_pEvents;
    bool m_bGroup;
    bool m_bUploadOnly;

    WizKMDatabaseServer m_server;

    std::map<QString, WIZKEYVALUEDATA> m_mapOldKeyValues;

public:
    template <class TData>
    static __int64 getObjectsVersion(__int64 nOldVersion, const std::deque<TData>& arrayData)
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
    bool downloadList(__int64 nServerVersion, const QString& strObjectType, WizKMSyncProgress progress)
    {
        m_pEvents->onSyncProgress(::GetSyncStartProgress(progress));
        //
        __int64 nVersion = m_pDatabase->getObjectVersion(strObjectType);
        if (nServerVersion == nVersion)
        {
            m_pEvents->onStatus(QObject::tr("No change, skip"));
            return TRUE;
        }
        //
        //
        __int64 nNextVersion = nVersion + 1;
        int nCountPerPage = 200;
        //
        while (1)
        {
            if (m_pEvents->isStop())
                return FALSE;
            //
            std::deque<TData> arrayPageData;
            //
            //QString strProgress = WizFormatString1(::WizTranslationsTranslateString("Start Version: %1"), WizInt64ToStr(nNextVersion));
            //m_pProgress->OnText(wizhttpstatustypeNormal, strProgress);
            //
            if (!m_server.getList<TData>(nCountPerPage, nNextVersion, arrayPageData))
            {
                TOLOG2("Failed to get object list: CountPerPage=%1, Version=%2", WizIntToStr(nCountPerPage), WizInt64ToStr(nVersion));
                return FALSE;
            }
            //
            if (arrayPageData.empty())
                break;
            //
            for (TData& data : arrayPageData)
            {
                data.strKbGUID = m_userInfo.strKbGUID;
            }
            //
            if (!onDownloadList<TData>(arrayPageData))
                return FALSE;
            //
            nNextVersion = getObjectsVersion<TData>(nNextVersion, arrayPageData);
            //
            if (!m_pDatabase->setObjectVersion(strObjectType, nNextVersion))
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
        return m_pDatabase->setObjectVersion(strObjectType, nVersion);
    }

    template <class TData>
    bool onDownloadList(const std::deque<TData>& arrayData)
    {
        ATLASSERT(FALSE);
        return FALSE;
    }
    template <class TData>
    bool onDownloadList(const std::deque<WIZDELETEDGUIDDATA>& arrayData)
    {
        return m_pDatabase->onDownloadDeletedList(arrayData);
    }
    template <class TData>
    bool onDownloadList(const std::deque<WIZTAGDATA>& arrayData)
    {
        return m_pDatabase->onDownloadTagList(arrayData);
    }
    template <class TData>
    bool onDownloadList(const std::deque<WIZSTYLEDATA>& arrayData)
    {
        return m_pDatabase->onDownloadStyleList(arrayData);
    }
    template <class TData>
    bool onDownloadList(const std::deque<WIZDOCUMENTDATAEX>& arrayData)
    {
        return m_pDatabase->onDownloadDocumentList(arrayData);
    }
    template <class TData>
    bool onDownloadList(const std::deque<WIZDOCUMENTATTACHMENTDATAEX>& arrayData)
    {
        return m_pDatabase->onDownloadAttachmentList(arrayData);
    }
    template <class TData>
    bool onDownloadList(const std::deque<WIZDOCUMENTPARAMDATA>& arrayData)
    {
        return m_pDatabase->onDownloadParamList(arrayData);
    }
};

#endif // WIZSERVICE_SYNC_P_H
