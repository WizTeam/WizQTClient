#ifndef WIZDRAWTEXTHELPER_H
#define WIZDRAWTEXTHELPER_H

#include "wizqthelper.h"

class QPainter;
class QRect;
class QColor;

int WizDrawTextSingleLine(QPainter* p, const QRect& rc, QString& str, int flags, QColor color, BOOL elidedText);

#endif // WIZDRAWTEXTHELPER_H
