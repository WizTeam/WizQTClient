#include "WizAccountManager.h"
#include "share/WizDatabase.h"
#include "share/WizMessageBox.h"

const QString g_strAccountSection = "Account";

WizAccountManager::WizAccountManager(WizDatabaseManager& dbMgr)
    : m_dbMgr(dbMgr)
{

}

bool WizAccountManager::isVip()
{
    WizDatabase& personDb = m_dbMgr.db();

    CString strUserType = personDb.getMetaDef(g_strAccountSection, "USERTYPE");
    if (strUserType.isEmpty() || strUserType == "free")
        return false;

    return true;
}

bool WizAccountManager::isPaidUser()
{
    if (isVip())
        return true;

    WizDatabase& personDb = m_dbMgr.db();

    CWizBizDataArray arrayBiz;
    personDb.getAllBizInfo(arrayBiz);

    for (WIZBIZDATA biz : arrayBiz)
    {
        if (biz.bizLevel > 0)
            return true;
    }

    return false;
}

bool WizAccountManager::isPaidGroup(const QString& kbGUID)
{
    WIZGROUPDATA group;
    if (m_dbMgr.db().getGroupData(kbGUID, group))
    {
        if (group.bizGUID.isEmpty())
            return false;

        WIZBIZDATA biz;
        if (m_dbMgr.db().getBizData(group.bizGUID, biz))
        {
            return biz.bizLevel > 0;
        }
    }

    return false;
}

bool WizAccountManager::isBizGroup(const QString& kbGUID)
{
    WIZGROUPDATA group;
    m_dbMgr.db().getGroupData(kbGUID, group);
    return group.isBiz();
}

