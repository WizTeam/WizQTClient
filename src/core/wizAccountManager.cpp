#include "wizAccountManager.h"
#include "share/wizDatabase.h"

const QString g_strAccountSection = "Account";

CWizAccountManager::CWizAccountManager(CWizDatabaseManager& dbMgr)
    : m_dbMgr(dbMgr)
{

}

bool CWizAccountManager::isVip()
{
    CWizDatabase& personDb = m_dbMgr.db();

    CString strUserType = personDb.GetMetaDef(g_strAccountSection, "USERTYPE");
    if (strUserType.IsEmpty() || strUserType == "free")
        return false;

    return true;
}

bool CWizAccountManager::isPaidUser()
{
    if (isVip())
        return true;

    CWizDatabase& personDb = m_dbMgr.db();

    CWizBizDataArray arrayBiz;
    personDb.GetUserBizInfo(true, arrayBiz);

    for (WIZBIZDATA biz : arrayBiz)
    {
        if (biz.bizLevel > 0)
            return true;
    }

    return false;
}

