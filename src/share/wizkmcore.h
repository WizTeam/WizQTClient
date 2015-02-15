#ifndef WIZKMCORE_H
#define WIZKMCORE_H

#include "wizqthelper.h"


template<class T>
BOOL WizKMObjectArrayIsEqual(const std::deque<T>& arrayData1, const std::deque<T>& arrayData2)
{
    if (arrayData1.size() != arrayData2.size())
        return FALSE;
    //
    std::set<CString> setGUID1;
    foreach (const T& data, arrayData1)
    {
        setGUID1.insert(data.strGUID);
    }
    //
    std::set<CString> setGUID2;
    foreach (const T& data, arrayData2)
    {
        setGUID2.insert(data.strGUID);
    }
    //
    return setGUID1 == setGUID2;
}

template<class T>
BOOL WizKMStringArrayIsEqual(const std::deque<T>& arrayData1, const std::deque<T>& arrayData2)
{
    if (arrayData1.size() != arrayData2.size())
        return FALSE;
    //
    std::set<CString> setGUID1;
    foreach (const T& data, arrayData1)
    {
        setGUID1.insert(data);
    }
    //
    std::set<CString> setGUID2;
    foreach (const T& data, arrayData2)
    {
        setGUID2.insert(data);
    }
    //
    return setGUID1 == setGUID2;
}


#define DOCUMENT_PARAM_NAME_CALENDAR_EVENT_PART		_T("Calendar_")

#define DOCUMENT_PARAM_NAME_CALENDAR_EVENT_START			_T("Calendar_Start")
#define DOCUMENT_PARAM_NAME_CALENDAR_EVENT_END				_T("Calendar_End")
#define DOCUMENT_PARAM_NAME_CALENDAR_EVENT_INFO				_T("Calendar_Info")
#define DOCUMENT_PARAM_NAME_CALENDAR_EVENT_RECURRENCE		_T("Calendar_Recurrence")
#define DOCUMENT_PARAM_NAME_CALENDAR_EVENT_END_RECURRENCE	_T("Calendar_EndRecurrence")

#define CALENDAR_EVENT_PARAM_LINE_COLOR				_T("b")
#define CALENDAR_EVENT_PARAM_LINE_REMINDER			_T("r")
#define CALENDAR_EVENT_PARAM_LINE_COMPLETED			_T("c")

struct IWizToolsSearchDocumentsEvents
{
    virtual BOOL OnDocuments(const CString& strDocumentID, const CString& strURL) = 0;
};


//void WizKMStartAllDocumentsIndexing(const CString& str, bool bPtompt);

HRESULT FTSDeleteDocument(const CString& strIndexPath, const CString& strDocumentGUID);
HRESULT FTSSearchDocument(const CString& strIndexPath, const CString& strKeywords, IWizToolsSearchDocumentsEvents* pEvents);

//void WizKMRegularTagsText(CString& strTagsText);



#endif // WIZKMCORE_H
