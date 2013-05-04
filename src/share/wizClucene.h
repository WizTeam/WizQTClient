#ifndef WIZCLUCENE_H
#define WIZCLUCENE_H

// interface
class IWizCluceneSearch
{
protected:
    bool beginUpdateDocument(const wchar_t* lpszIndexPath, void** ppHandle);
    bool endUpdateDocument(void* pHandle);
    bool updateDocument(void* pHandle,
                        const wchar_t* lpszKbGUID,
                        const wchar_t* lpszDocumentID,
                        const wchar_t* lpszTitle,
                        const wchar_t* lpszText);

    bool deleteDocument(const wchar_t* lpszIndexPath, const wchar_t* lpszDocumentID);
    bool searchDocument(const wchar_t* lpszIndexPath, const wchar_t* lpszKeywords);

    virtual bool onSearchProcess(const wchar_t* lpszKbGUID,
                                 const wchar_t* lpszDocumentID,
                                 const wchar_t* lpszURL) = 0;

    virtual bool onSearchEnd() = 0;
};

// debug related
//#define WIZ_FTS_ENABLE_TEST
//#define WIZ_FTS_ENABLE_CJK2

#ifdef WIZ_FTS_ENABLE_TEST
int WizTest();
#endif

#endif // WIZCLUCENE_H
