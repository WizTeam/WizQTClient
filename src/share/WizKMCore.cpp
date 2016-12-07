#include "WizKMCore.h"



//void WizKMStartAllDocumentsIndexing(const CString& str, bool bPtompt)
//{
//    Q_UNUSED(str);
//    Q_UNUSED(bPtompt);
//}

HRESULT FTSDeleteDocument(const CString& strIndexPath, const CString& strDocumentGUID)
{
    Q_UNUSED(strIndexPath);
    Q_UNUSED(strDocumentGUID);
    //
    return E_FAIL;
}
HRESULT FTSSearchDocument(const CString& strIndexPath, const CString& strKeywords, IWizToolsSearchDocumentsEvents* pEvents)
{
    Q_UNUSED(strIndexPath);
    Q_UNUSED(strKeywords);
    Q_UNUSED(pEvents);
    //
    return E_FAIL;
}

//void WizKMRegularTagsText(CString& strTagsText)
//{
//    Q_UNUSED(strTagsText);
//}


