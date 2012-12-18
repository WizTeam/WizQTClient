#ifndef WIZCLUCENE_H
#define WIZCLUCENE_H

#define WIZ_FTS_ENABLE_TEST
//#define WIZ_FTS_ENABLE_CJK2

// interface
class IWizSearchDocumentsEvents
{
public:
    virtual bool OnDocuments(const wchar_t* lpszKbGUID, const wchar_t* lpszDocumentID, const wchar_t* lpszURL) = 0;
};

bool WizFTSBeginUpdateDocument(const wchar_t* lpszIndexPath, void** ppHandle);
bool WizFTSEndUpdateDocument(void* pHandle);

bool WizFTSUpdateDocument(void* pHandle, \
                          const wchar_t* lpszWizDocumentID, \
                          const wchar_t* lpszKbGUID, \
                          const wchar_t* lpszDocumentID, \
                          const wchar_t* lpszURL, \
                          const wchar_t* lpszText);

bool WizFTSDeleteDocument(const wchar_t* lpszIndexPath, const wchar_t* lpszWizDocumentID);
bool WizFTSSearchDocument(const wchar_t* lpszIndexPath, const wchar_t* lpszKeywords, IWizSearchDocumentsEvents* pEvents);

// debug related functions
#ifdef WIZ_FTS_ENABLE_TEST

int WizTest();

#endif

#endif // WIZCLUCENE_H
