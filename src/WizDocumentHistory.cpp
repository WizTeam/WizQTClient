#include "WizDocumentHistory.h"



////////////////////////////////////////////////////////////////////////////////

WizDocumentViewHistory::WizDocumentViewHistory()
    : m_nIndex(-1)
{
}
void WizDocumentViewHistory::addHistory(const WIZDOCUMENTDATA& data)
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

bool WizDocumentViewHistory::canBack() const
{
    return m_nIndex > 0;
}
bool WizDocumentViewHistory::canForward() const
{
    return m_nIndex < int(m_arrayHistory.size()) - 1;
}
WIZDOCUMENTDATA WizDocumentViewHistory::back()
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

WIZDOCUMENTDATA WizDocumentViewHistory::forward()
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
