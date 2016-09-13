#include "WizDrawTextHelper.h"

#include <QPainter>
#include <QTextLayout>
#include <QFontMetrics>

int WizDrawTextSingleLine(QPainter* p, const QRect& rc, QString& str, int flags, QColor color, bool elidedText)
{
    if (rc.width() <= 0)
        return 0;

    if (str.isEmpty())
        return 0;

    p->setPen(color);

    if (elidedText) {
        CString strRet = p->fontMetrics().elidedText(str, Qt::ElideRight, rc.width());
        p->drawText(rc, flags, strRet);
        return p->fontMetrics().width(strRet);
    } else {
        QTextLayout textLayout(str, p->font());
        textLayout.beginLayout();

        int width = 0;
        QTextLine line = textLayout.createLine();
        if (line.isValid())
        {
            line.setLineWidth(rc.width());

            CString lineText = str.left(line.textLength());
            str.remove(0, line.textLength());
            p->drawText(rc, flags, lineText);
            width = line.width();
        }

        textLayout.endLayout();
        return width;
    }
}
