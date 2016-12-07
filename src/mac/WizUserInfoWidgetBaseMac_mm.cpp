#include "WizUserInfoWidgetBaseMac_mm.h"
#ifdef USECOCOATOOLBAR

#include <QPainter>
#include "sync/WizAvatarUploader.h"
#include "sync/WizAvatarHost.h"
#include "utils/WizStyleHelper.h"


QString WizUserInfoWidgetBaseMac::text() const
{
    return m_text;
}
void WizUserInfoWidgetBaseMac::setText(QString val)
{
    m_text = val;
    m_textWidth = 0;
    calTextSize();
}

#endif
