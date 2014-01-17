#include "wizUserInfoWidgetBaseMac_mm.h"


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
