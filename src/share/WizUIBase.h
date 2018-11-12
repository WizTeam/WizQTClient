#ifndef WIZUIBASE_H
#define WIZUIBASE_H

#include <QtGui>

bool isDarkMode();
QPixmap qpixmapWithTintColor(const QPixmap& pixmap, QColor tintColor);
QImage qimageWithTintColor(const QImage& image, QColor tintColor);

#define WizColorLineEditorBackground QColor(100, 98, 102)
#define WizColorButtonIcon QColor(0xcc, 0xcc, 0xcc)

#define WIZ_TINT_COLOR_STRING   "#448adf"
#define WIZ_TINT_COLOR          QColor(WIZ_TINT_COLOR_STRING)

void WizApplyDarkModeStyles(QWidget* widget);

#endif // WIZUIBASE_H
