#ifndef WIZNOTESTYLE_H
#define WIZNOTESTYLE_H

#include <QString>
#include <QStyle>

QStyle* WizGetStyle(const QString& skinName);

QStyle* WizGetImageButtonStyle(const QString& normalBackgroundFileName, const QString& hotBackgroundFileName,
                               const QString& downBackgroundFileName, const QString& disabledBackgroundFileName,
                               const QColor& normalTextColor, const QColor& activeTextColor, const QColor& disableTextColor);


#endif // WIZNOTESTYLE_H
