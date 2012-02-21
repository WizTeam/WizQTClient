#ifndef WIZUI_H
#define WIZUI_H

#include "wizqthelper.h"
#include <QImage>


class QPainter;

class CWizSkin9GridImage
{
protected:
    QImage m_img;
    QRect m_arrayImageGrid[9];
    //
    BOOL Clear();
public:
    static BOOL SplitRect(const QRect& rcSrc, QPoint ptTopLeft, QRect* parrayRect, int nArrayCount);
    BOOL SetImage(const CString& strImageFileName, QPoint ptTopLeft);
    //
    void Draw(QPainter* p, QRect rc, int nAlpha) const;
    BOOL Valid() const;
};



#endif // WIZUI_H
