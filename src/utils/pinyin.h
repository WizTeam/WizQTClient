#ifndef PINYIN_H
#define PINYIN_H

class QString;

enum {
    WIZ_C2P_FIRST_LETTER_ONLY = 0x0,
    WIZ_C2P_POLYPHONE = 0x1
};

int WizToolsChinese2PinYin(const wchar_t* lpszText, unsigned int flags, QString& pbstrTextResult);


#endif // PINYIN_H
