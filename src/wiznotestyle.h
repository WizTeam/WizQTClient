#ifndef WIZNOTESTYLE_H
#define WIZNOTESTYLE_H

#include <QString>
#include <QStyle>

QStyle* WizGetStyle(const QString& skinName);

class QColor;
QColor WizGetCategoryBackroundColor(const QString& skinName);
QColor WizGetDocumentsBackroundColor(const QString& skinName);
QColor WizGetClientBackgroundColor(const QString& skinName);

#endif // WIZNOTESTYLE_H
