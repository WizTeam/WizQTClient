#ifndef WIZDRAWTEXTHELPER_H
#define WIZDRAWTEXTHELPER_H

#include "WizQtHelper.h"

class QPainter;
class QRect;
class QColor;

int WizDrawTextSingleLine(QPainter* p, const QRect& rc, QString& str, int flags, QColor color, bool elidedText);

#endif // WIZDRAWTEXTHELPER_H
