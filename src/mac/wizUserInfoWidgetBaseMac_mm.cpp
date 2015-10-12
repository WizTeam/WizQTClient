#include "wizUserInfoWidgetBaseMac_mm.h"
#ifdef USECOCOATOOLBAR

#include <QPainter>
#include "sync/avataruploader.h"
#include "sync/avatar.h"
#include "utils/stylehelper.h"


using namespace WizService;
using namespace WizService::Internal;

QString CWizUserInfoWidgetBaseMac::text() const
{
    return m_text;
}
void CWizUserInfoWidgetBaseMac::setText(QString val)
{
    m_text = val;
    m_textWidth = 0;
    calTextSize();
}

#endif
