#include "wizimagepushbutton.h"

#include <QBitmap>

CWizImagePushButton::CWizImagePushButton(const QIcon& icon, const QString &text, QWidget* parent)
    :QPushButton(icon, text, parent)
    , m_redFlag(false)
{
    //QPixmap mask = icon.pixmap(32, 26);
    //setIcon(QIcon(mask));
    //setMask(mask.mask());
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
