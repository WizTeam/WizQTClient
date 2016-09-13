#ifndef WIZDOCUMENTHISTORY_H
#define WIZDOCUMENTHISTORY_H


#include "share/WizDatabase.h"

class WizDocumentViewHistory
{
public:
    WizDocumentViewHistory();
private:
    int m_nIndex;
    std::deque<WIZDOCUMENTDATA> m_arrayHistory;
public:
    void addHistory(const WIZDOCUMENTDATA& data);
    bool canBack() const;
    bool canForward() const;
    WIZDOCUMENTDATA back();
    WIZDOCUMENTDATA forward();
};


#endif // WIZDOCUMENTHISTORY_H
