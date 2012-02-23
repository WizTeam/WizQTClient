#include "wizimagepushbutton.h"

CWizImagePushButton::CWizImagePushButton(const QIcon& icon, const QString &text, QWidget* parent)
    :QPushButton(icon, text, parent)
{
}

QSize CWizImagePushButton::sizeHint() const
{
    QSize sz(32, 26);
    QString label(text());
    if (!label.isEmpty())
    {
        sz.setWidth(sz.width() + 8 + label.length() * 8);
    }
    return sz;
}
