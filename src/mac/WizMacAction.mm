#include "wizmacicon.h"

#if 0
#include <QFileInfo>
#include <QPixmapCache>
#include <QPainter>
#include <QStyle>
#include <QDebug>
#include <QDir>



void qt_mac_constructQIconFromIconRef(const IconRef icon, const IconRef overlayIcon, QIcon *retIcon,
                                      QStyle::StandardPixmap standardIcon = QStyle::SP_CustomBase);



QIcon getMacIcon(const QFileInfo &fi)
{
    QIcon retIcon;
    QString fileExtension = fi.suffix().toUpper();
    fileExtension.prepend(QLatin1String("."));

    const QString keyBase = QLatin1String("qt_") + fileExtension;

    QPixmap pixmap;
    if (fi.isFile() && !fi.isExecutable() && !fi.isSymLink()) {
        QPixmapCache::find(keyBase + QLatin1String("16"), pixmap);
    }

    if (!pixmap.isNull()) {
        retIcon.addPixmap(pixmap);
        if (QPixmapCache::find(keyBase + QLatin1String("32"), pixmap)) {
            retIcon.addPixmap(pixmap);
            if (QPixmapCache::find(keyBase + QLatin1String("64"), pixmap)) {
                retIcon.addPixmap(pixmap);
                if (QPixmapCache::find(keyBase + QLatin1String("128"), pixmap)) {
                    retIcon.addPixmap(pixmap);
                    return retIcon;
                }
            }
        }
    }

    QString path = fi.canonicalFilePath();
    //
    FSRef macRef;
    OSStatus status = FSPathMakeRef(reinterpret_cast<const UInt8*>(path.toUtf8().constData()),
                                    &macRef, 0);
    if (status != noErr)
        return retIcon;
    FSCatalogInfo info;
    HFSUniStr255 macName;
    status = FSGetCatalogInfo(&macRef, kIconServicesCatalogInfoMask, &info, &macName, 0, 0);
    if (status != noErr)
        return retIcon;
    IconRef iconRef;
    SInt16 iconLabel;
    status = GetIconRefFromFileInfo(&macRef, macName.length, macName.unicode,
                                    kIconServicesCatalogInfoMask, &info, kIconServicesNormalUsageFlag,
                                    &iconRef, &iconLabel);
    if (status != noErr)
        return retIcon;
    qt_mac_constructQIconFromIconRef(iconRef, 0, &retIcon);
    ReleaseIconRef(iconRef);

    pixmap = retIcon.pixmap(16);
    QPixmapCache::insert(keyBase + QLatin1String("16"), pixmap);
    pixmap = retIcon.pixmap(32);
    QPixmapCache::insert(keyBase + QLatin1String("32"), pixmap);
    pixmap = retIcon.pixmap(64);
    QPixmapCache::insert(keyBase + QLatin1String("64"), pixmap);
    pixmap = retIcon.pixmap(128);
    QPixmapCache::insert(keyBase + QLatin1String("128"), pixmap);

    return retIcon;
}





QPixmap qt_mac_convert_iconref(const IconRef icon, int width, int height)
{
    QPixmap ret(width, height);
    ret.fill(QColor(0, 0, 0, 0));

    CGRect rect = CGRectMake(0, 0, width, height);

    CGContextRef ctx = qt_mac_cg_context(&ret);
    CGAffineTransform old_xform = CGContextGetCTM(ctx);
    CGContextConcatCTM(ctx, CGAffineTransformInvert(old_xform));
    CGContextConcatCTM(ctx, CGAffineTransformIdentity);

    ::RGBColor b;
    b.blue = b.green = b.red = 255*255;
    PlotIconRefInContext(ctx, &rect, kAlignNone, kTransformNone, &b, kPlotIconRefNormalFlags, icon);
    CGContextRelease(ctx);
    return ret;
}

void qt_mac_constructQIconFromIconRef(const IconRef icon, const IconRef overlayIcon, QIcon *retIcon, QStyle::StandardPixmap standardIcon)
{
    int size = 16;
    while (size <= 128) {

        const QString cacheKey = QLatin1String("qt_mac_constructQIconFromIconRef") + QString::number(standardIcon) + QString::number(size);
        QPixmap mainIcon;
        if (standardIcon >= QStyle::SP_CustomBase) {
            mainIcon = qt_mac_convert_iconref(icon, size, size);
        } else if (QPixmapCache::find(cacheKey, mainIcon) == false) {
            mainIcon = qt_mac_convert_iconref(icon, size, size);
            QPixmapCache::insert(cacheKey, mainIcon);
        }

        if (overlayIcon) {
            int littleSize = size / 2;
            QPixmap overlayPix = qt_mac_convert_iconref(overlayIcon, littleSize, littleSize);
            QPainter painter(&mainIcon);
            painter.drawPixmap(size - littleSize, size - littleSize, overlayPix);
        }

        retIcon->addPixmap(mainIcon);
        size += size;  // 16 -> 32 -> 64 -> 128
    }
}

#endif
