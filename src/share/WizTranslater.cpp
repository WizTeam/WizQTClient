#include "WizTranslater.h"
#include <QObject>


WizTranslater::WizTranslater()
{
}

WizTranslater::~WizTranslater()
{

}

WizTranslateItem*WizTranslater::itemsData()
{
    static WizTranslateItem itemGroup[] =
    {
        {"{error_text1}", QObject::tr("Load Error")},
        {"{error_text2}", QObject::tr("Network anomalies, check the network, then retry!")},


        {"",""}
    };

    return itemGroup;
}


QString WizTranlateString(const QString& strString)
{
    WizTranslater translater;
    WizTranslateItem* arrayData = translater.itemsData();
    int index = 0;
    while (1)
    {
        WizTranslateItem& item = arrayData[index];
        if (item.key.isEmpty())
            break;

        if (strString == item.key)
            return item.value;

        index++;
    }

    return strString;
}
