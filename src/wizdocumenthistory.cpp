#include "wizdocumenthistory.h"



////////////////////////////////////////////////////////////////////////////////

CWizDocumentViewHistory::CWizDocumentViewHistory()
    : m_nIndex(-1)
{
}
void CWizDocumentViewHistory::addHistory(const WIZDOCUMENTDATA& data)
{
    if (m_nIndex < 0)
    {
        m_arrayHistory.clear();
        m_arrayHistory.push_back(data);
        m_nIndex = 0;
    }
    else if (m_nIndex >= int(m_arrayHistory.size()) - 1)
    {
        m_arrayHistory.push_back(data);
        m_nIndex = int(m_arrayHistory.size()) - 1;
    }
    else
    {
        m_arrayHistory.resize(m_nIndex + 1);
        m_arrayHistory.push_back(data);
        m_nIndex = int(m_arrayHistory.size()) - 1;
    }
}

bool CWizDocumentViewHistory::canBack() const
{
    return m_nIndex > 0;
}
bool CWizDocumentViewHistory::canForward() const
{
    return m_nIndex < int(m_arrayHistory.size()) - 1;
}
WIZDOCUMENTDATA CWizDocumentViewHistory::back()
{
    WIZDOCUMENTDATA data;
    if (!canBack())
        return data;
    //
    m_nIndex--;
    //
    ATLASSERT(m_nIndex >= 0 && m_nIndex < int(m_arrayHistory.size()) - 1);
    if (m_nIndex >= 0 && m_nIndex < int(m_arrayHistory.size()) - 1)
    {
        return m_arrayHistory[m_nIndex];
    }
    //
    return data;
}

WIZDOCUMENTDATA CWizDocumentViewHistory::forward()
{
    WIZDOCUMENTDATA data;
    if (!canForward())
        return data;
    //
    m_nIndex++;
    //
    ATLASSERT(m_nIndex >= 0 && m_nIndex <= int(m_arrayHistory.size()) - 1);
    if (m_nIndex >= 0 && m_nIndex <= int(m_arrayHistory.size()) - 1)
    {
        return m_arrayHistory[m_nIndex];
    }
    //
    return data;
}
