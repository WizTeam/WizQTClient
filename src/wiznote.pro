#-------------------------------------------------
#
# Project created by QtCreator 2012-01-05T10:12:30
#
#-------------------------------------------------

QT += core gui
QT += xml
QT += network
QT += webkit


TARGET = WizNote
TEMPLATE = app

# Date: 18/02/2012 10:59 
# Maintainer: Zhang PengCheng
# Purpose: hide .o file in src directory.
OBJECTS_DIR = .obj


OBJECTIVE_SOURCES += share/wizuihelper_mac.mm \
    mac/wizmactoolbardelegate.mm \
    mac/wizmactoolbar.mm \
    mac/wizmachelper.mm \
    mac/wizmacicon.mm


SOURCES += main.cpp\
        mainwindow.cpp \
    share/wizqthelper.cpp \
    share/wizmisc.cpp \
    share/sqlite3.c \
    share/index.cpp \
    share/cppsqlite3.cpp \
    share/wizobject.cpp \
    share/wizkmcore.cpp \
    share/wizxmlrpc.cpp \
    share/wizapi.cpp \
    share/wizsync.cpp \
    share/wizdatabase.cpp \
    welcomedialog.cpp \
    share/wizverifyaccount.cpp \
    share/wizsettings.cpp \
    wizdocumentlistview.cpp \
    zip/zip.c \
    zip/unzip.c \
    zip/quazipnewinfo.cpp \
    zip/quazipfile.cpp \
    zip/quazip.cpp \
    zip/quacrc32.cpp \
    zip/quaadler32.cpp \
    zip/qioapi.cpp \
    zip/zutil.c \
    zip/uncompr.c \
    zip/trees.c \
    zip/inftrees.c \
    zip/inflate.c \
    zip/inffast.c \
    zip/infback.c \
    zip/gzio.c \
    zip/deflate.c \
    zip/crc32.c \
    zip/compress.c \
    zip/adler32.c \
    share/wizui.cpp \
    wizdocumentview.cpp \
    wizdocumentwebview.cpp \
    share/wizhtml2zip.cpp \
    html/wizhtmlreader.cpp \
    html/wizhtmlcollector.cpp \
    share/wizmd5.cpp \
    zip/wizzip.cpp \
    wizactions.cpp \
    share/wizwin32helper.cpp \
    share/thumbindex.cpp \
    share/wizdrawtexthelper.cpp \
    wiznotestyle.cpp \
    wizcategoryview.cpp \
    newfolderdialog.cpp \
    newtagdialog.cpp \
    share/wizuihelper.cpp \
    share/wizdownloadobjectdata.cpp \
    share/wizdownloadobjectdatadialog.cpp \
    wizdocumenthistory.cpp \
    aboutdialog.cpp \
    createaccountdialog.cpp \
    share/wizcreateaccount.cpp \
    mac/wizmacactionhelper.cpp \
    share/wizcommonui.cpp \
    share/wizanimateaction.cpp \
    wiznotesettings.cpp \
    wizattachmentlistwidget.cpp \
    share/wizpopupwidget.cpp \
    share/wizmultilinelistwidget.cpp \
    share/wizfileiconprovider.cpp \
    share/wizwindowshelper.cpp \
    share/wizimagepushbutton.cpp \
    wiztaglistwidget.cpp

HEADERS  += mainwindow.h \
    share/wizqthelper.h \
    share/wizmisc.h \
    share/sqlite3ext.h \
    share/sqlite3.h \
    share/index.h \
    share/cppsqlite3.h \
    share/wizobject.h \
    share/wizkmcore.h \
    share/wizapi.h \
    share/wizxmlrpc.h \
    share/wizsync.h \
    share/wizdatabase.h \
    welcomedialog.h \
    share/wizverifyaccount.h \
    share/wizsettings.h \
    wizdocumentlistview.h \
    zip/zip.h \
    zip/unzip.h \
    zip/quazipnewinfo.h \
    zip/quazipfileinfo.h \
    zip/quazipfile.h \
    zip/quazip_global.h \
    zip/quazip.h \
    zip/quacrc32.h \
    zip/quachecksum32.h \
    zip/quaadler32.h \
    zip/ioapi.h \
    zip/crypt.h \
    zip/zutil.h \
    zip/zlib.h \
    zip/zconf.in.h \
    zip/zconf.h \
    zip/trees.h \
    zip/inftrees.h \
    zip/inflate.h \
    zip/inffixed.h \
    zip/inffast.h \
    zip/deflate.h \
    zip/crc32.h \
    share/wizui.h \
    wizdocumentview.h \
    wizdocumentwebview.h \
    wizdef.h \
    share/wizhtml2zip.h \
    html/wizhtmlreader.h \
    html/wizhtmlcollector.h \
    share/wizmd5.h \
    zip/wizzip.h \
    wizactions.h \
    share/wizwin32helper.h \
    share/thumbindex.h \
    share/wizdrawtexthelper.h \
    wiznotestyle.h \
    wizcategoryview.h \
    newfolderdialog.h \
    newtagdialog.h \
    share/wizuihelper.h \
    share/wizdownloadobjectdata.h \
    share/wizdownloadobjectdatadialog.h \
    wizdocumenthistory.h \
    aboutdialog.h \
    createaccountdialog.h \
    share/wizcreateaccount.h \
    mac/wizmacactionhelper.h \
    share/wizcommonui.h \
    share/wizanimateaction.h \
    wiznotesettings.h \
    wizattachmentlistwidget.h \
    share/wizpopupwidget.h \
    share/wizmultilinelistwidget.h \
    share/wizfileiconprovider.h \
    share/wizwindowshelper.h \
    share/wizimagepushbutton.h \
    mac/wizmacicon.h \
    wiztaglistwidget.h


mac:HEADERS +=     \
    mac/wizmactoolbardelegate.h \
    mac/wizmactoolbar.h \
    mac/wizmachelper.h


FORMS    += \
    welcomedialog.ui \
    newfolderdialog.ui \
    newtagdialog.ui \
    share/wizdownloadobjectdatadialog.ui \
    aboutdialog.ui \
    createaccountdialog.ui





mac:LIBS += -framework Cocoa -framework Carbon


RC_FILE = wiznote.rc
ICON=images/wiznote.icns


TRANSLATIONS = wiznote_zh_CN.ts \
    wiznote_zh_TW.ts
