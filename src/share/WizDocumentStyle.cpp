#include "WizDocumentStyle.h"
#include "WizDatabase.h"
#include "WizDatabaseManager.h"
#include "../utils/WizLogger.h"

#include <QMutex>
#include <QMutexLocker>

class WizDocumentStylePrivate
{
private:
    QMutex m_mutex;
    std::map<QString, WIZSTYLEDATA> m_styles;
public:
    WizDocumentStylePrivate()
        : m_mutex(QMutex::Recursive)
    {
    }

    WIZSTYLEDATA getStyle(QString styleGuid)
    {
        QMutexLocker locker(&m_mutex);
        Q_UNUSED(locker);
        //
        auto it = m_styles.find(styleGuid);
        if (it == m_styles.end())
        {
            WizDatabase& db = WizDatabaseManager::instance()->db();
            WIZSTYLEDATA style;
            bool ret = db.styleFromGuid(styleGuid, style);
            if (!ret)
            {
                TOLOG1("Failed to find style by guid: %1", styleGuid);
            }
            m_styles[styleGuid] = style;
            return style;
        }
        //
        return it->second;
    }
};


WizDocumentStyle::WizDocumentStyle()
    : m_data(new WizDocumentStylePrivate())
{
}

WizDocumentStyle& WizDocumentStyle::instance()
{
    static WizDocumentStyle style;
    return style;
}

WIZSTYLEDATA WizDocumentStyle::getStyle(QString styleGuid)
{
    if (styleGuid.isEmpty())
        return WIZSTYLEDATA();
    return WIZSTYLEDATA();
    //暂时不开启样式功能
    //return m_data->getStyle(styleGuid);
}

