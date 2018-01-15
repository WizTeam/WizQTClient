#ifndef PINYIN_H
#define PINYIN_H

class QString;

enum {
    WIZ_C2P_NORMAL = 0x0,
    WIZ_C2P_FIRST_LETTER_ONLY = 0x1,
    WIZ_C2P_POLYPHONE = 0x2
};

int WizToolsChinese2PinYin(QString text, unsigned int flags, QString& strTextResult);
bool WizToolsIsChinese2(QString str);

int WizToolsSmartCompare(QString text1, QString text2);


#endif // PINYIN_H
