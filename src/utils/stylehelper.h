#ifndef UTILS_STYLEHELPER_H
#define UTILS_STYLEHELPER_H

class QSize;

namespace Utils {
class StyleHelper
{
public:
    static QSize avatarSize();
    static int avatarHeight();
    static int lineSpacing();
    static int margin();
    static int thumbnailHeight();
};
} // namespace Utils

#endif // UTILS_STYLEHELPER_H
