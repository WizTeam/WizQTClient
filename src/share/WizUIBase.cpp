#include "WizUIHelper.h"

#include <QPixmap>
#include <QImage>
#include <QColor>
#include "WizSettings.h"
#include "utils/WizPathResolve.h"

#include <QLabel>
#include <QAbstractButton>
#include <QLineEdit>

static int ComputeOverlay(int upperLayerValue, int lowerLayerValue)
{
    int a = lowerLayerValue;
    int b = upperLayerValue;
    //
    if (a < 128)
    {
        int ret = 2 * a * b / 255;
        return ret;
    }
    else
    {
        int ret = 255 - 2 * (255 - a) * (255 - b) / 255;
        return ret;
    }
}

QImage qimageWithTintColor(const QImage& image, QColor tintColor)
{
    if (!image.hasAlphaChannel()) {
        return image;
    }
    //
    if (tintColor == Qt::transparent) {
        return image;
    }
    //
    QImage src = image;
    QImage::Format format = src.format();
    if (format == QImage::Format_ARGB32_Premultiplied || format == QImage::Format_RGBA8888_Premultiplied) {

    } else {
        src = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        format = QImage::Format_ARGB32_Premultiplied;
    }
    //
    int width = src.width();
    int height = src.height();
    //
    int r = tintColor.red();
    int g = tintColor.green();
    int b = tintColor.blue();
    //
    int Db = b; //dest alpha = 0xFF;
    int Dg = g; //dest alpha = 0xFF;
    int Dr = r; //dest alpha = 0xFF;
    int Da = 0xFF;
    //
    bool argb = format == QImage::Format_ARGB32_Premultiplied;
    //
    QImage dest(width, height, QImage::Format_ARGB32_Premultiplied);
    qreal pixelRatio = image.devicePixelRatio();
    dest.setDevicePixelRatio(pixelRatio);
    //
    const uchar* pBytesSrc = src.bits();
    int strideSrc = src.bytesPerLine();
    //
    uchar* pBytesDest = dest.bits();
    int strideDest = dest.bytesPerLine();
    //
    int maxGray = 0;
    //
    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            const uchar* pPixelSrc = pBytesSrc + j * strideSrc + i * 4;
            int Sb = argb ? pPixelSrc[0] : pPixelSrc[1];
            int Sg = argb ? pPixelSrc[1] : pPixelSrc[2];
            int Sr = argb ? pPixelSrc[2] : pPixelSrc[3];
            //
            int Gray = (Sr*299 + Sg*587 + Sb*114 + 500) / 1000;
            if (Gray > maxGray) {
                maxGray = Gray;
                if (maxGray == 255) {
                    break;
                }
            }
        }
        if (maxGray == 255) {
            break;
        }
    }
    //
    if (0 == maxGray) {
        maxGray = 1;
    }
    //
    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            const uchar* pPixelSrc = pBytesSrc + j * strideSrc + i * 4;
            uchar* pPixelDest = pBytesDest + j * strideDest + i * 4;
            //
            int Sb = argb ? pPixelSrc[0] : pPixelSrc[1];
            int Sg = argb ? pPixelSrc[1] : pPixelSrc[2];
            int Sr = argb ? pPixelSrc[2] : pPixelSrc[3];
            int Sa = argb ? pPixelSrc[3] : pPixelSrc[0];
            //
            int Gray = (Sr*299 + Sg*587 + Sb*114 + 500) / 1000;
            Gray = Gray * 255 / maxGray;
            if (Gray > 255) {
                Gray = 255;
                qDebug() << "gray > 255?";
            }
            //
            pPixelDest[3] = Sa;	//keep alpha
            pPixelDest[0] = Db * Gray / 255 * Sa / 0xFF;
            pPixelDest[1] = Dg * Gray / 255 * Sa / 0xFF;
            pPixelDest[2] = Dr * Gray / 255 * Sa / 0xFF;
            //
        }
    }
    return dest;
}


QPixmap qpixmapWithTintColor(const QPixmap& pixmap, QColor tintColor)
{
    QImage image = pixmap.toImage();
    QImage dest = qimageWithTintColor(image, tintColor);
    QPixmap pixmapRet = QPixmap::fromImage(dest);
    return pixmapRet;
}

#ifndef Q_OS_MAC
bool isDarkMode()
{
    static bool first = true;
    static bool darkMode = false;
    if (first) {
        first = false;
        WizSettings wizSettings(Utils::WizPathResolve::globalSettingsFile());
        darkMode = wizSettings.isDarkMode();
    }

    return darkMode;
}
#endif

void WizApplyDarkModeStyles(QWidget* widget)
{
#ifndef Q_OS_MAC
    if (isDarkMode()) {
        for (QObject* child : widget->children()) {
            if (QWidget* widget = dynamic_cast<QLabel*>(child)) {
                widget->setStyleSheet("color:#a6a6a6");
            } else if (QWidget* widget = dynamic_cast<QLineEdit*>(child)) {
                widget->setStyleSheet("color:#a6a6a6");
            } else if (QWidget* widget = dynamic_cast<QAbstractButton*>(child)) {
                widget->setStyleSheet("color:#a6a6a6");
            } else if (QWidget* widget = dynamic_cast<QWidget*>(child)) {
                WizApplyDarkModeStyles(widget);
            }
        }
    }
#endif
}
