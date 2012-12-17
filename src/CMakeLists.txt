# use FindQt4.cmake module
find_package(Qt4 REQUIRED QtCore QtGui QtXml QtNetwork QtWebKit)
include(${QT_USE_FILE})

set(wiznote_SOURCES
    zip/zip.c
    zip/unzip.c
    zip/quazipnewinfo.cpp
    zip/quazipfile.cpp
    zip/quazip.cpp
    zip/quacrc32.cpp
    zip/quaadler32.cpp
    zip/qioapi.cpp
    zip/zutil.c
    zip/uncompr.c
    zip/trees.c
    zip/inftrees.c
    zip/inflate.c
    zip/inffast.c
    zip/infback.c
    zip/gzio.c
    zip/deflate.c
    zip/crc32.c
    zip/compress.c
    zip/adler32.c
    zip/wizzip.cpp
    share/wizqthelper.cpp
    share/wizmisc.cpp
    share/sqlite3.c
    share/cppsqlite3.cpp
    share/wizobject.cpp
    share/wizkmcore.cpp
    share/wizxmlrpc.cpp
    share/wizapi.cpp
    share/wizsync.cpp
    share/wizdatabase.cpp
    share/wizverifyaccount.cpp
    share/wizsettings.cpp
    share/wizxml.cpp
    share/wizsyncthread.cpp
    share/wizui.cpp
    share/wizmd5.cpp
    share/wizhtml2zip.cpp
    share/wizwin32helper.cpp
    share/wizdrawtexthelper.cpp
    share/wizuihelper.cpp
    share/wizdownloadobjectdata.cpp
    share/wizcreateaccount.cpp
    share/wizcommonui.cpp
    share/wizanimateaction.cpp
    share/wizpopupwidget.cpp
    share/wizmultilinelistwidget.cpp
    share/wizfileiconprovider.cpp
    share/wizwindowshelper.cpp
    share/wizimagepushbutton.cpp
    share/wizindex.cpp
    share/wizthumbindex.cpp
    share/wizenc.cpp
    share/wizziwreader.cpp
    html/wizhtmlreader.cpp
    html/wizhtmlcollector.cpp
    main.cpp
    wizdocumentlistview.cpp
    wizdocumentview.cpp
    wizdocumentwebview.cpp
    wizactions.cpp
    wiznotestyle.cpp
    wizcategoryview.cpp
    newfolderdialog.cpp
    newtagdialog.cpp
    wizdownloadobjectdatadialog.cpp
    wizdocumenthistory.cpp
    wizattachmentlistwidget.cpp
    wiztaglistwidget.cpp
    wizconsoledialog.cpp
    wizpreferencedialog.cpp
    wizaboutdialog.cpp
    wizwelcomedialog.cpp
    wizproxydialog.cpp
    wizcreateaccountdialog.cpp
    wizmainwindow.cpp
    wizupdater.cpp
    wizupdaterprogressdialog.cpp
    wizstatusbar.cpp
    wizupgradenotifydialog.cpp
    wizcertmanager.cpp
    wizusercipherform.cpp
    wiznoteinfoform.cpp
)

set(wiznote_HEADERS
    zip/zip.h
    zip/unzip.h
    zip/quazipnewinfo.h
    zip/quazipfileinfo.h
    zip/quazipfile.h
    zip/quazip_global.h
    zip/quazip.h
    zip/quacrc32.h
    zip/quachecksum32.h
    zip/quaadler32.h
    zip/ioapi.h
    zip/crypt.h
    zip/zutil.h
    zip/zlib.h
    zip/zconf.in.h
    zip/zconf.h
    zip/trees.h
    zip/inftrees.h
    zip/inflate.h
    zip/inffixed.h
    zip/inffast.h
    zip/deflate.h
    zip/crc32.h
    zip/wizzip.h
    share/wizqthelper.h
    share/wizmisc.h
    share/sqlite3ext.h
    share/sqlite3.h
    share/cppsqlite3.h
    share/wizobject.h
    share/wizkmcore.h
    share/wizapi.h
    share/wizxmlrpc.h
    share/wizsync.h
    share/wizdatabase.h
    share/wizverifyaccount.h
    share/wizsettings.h
    share/wizui.h
    share/wizhtml2zip.h
    share/wizmd5.h
    share/wizwin32helper.h
    share/wizdrawtexthelper.h
    share/wizuihelper.h
    share/wizdownloadobjectdata.h
    share/wizcreateaccount.h
    share/wizcommonui.h
    share/wizanimateaction.h
    share/wizpopupwidget.h
    share/wizmultilinelistwidget.h
    share/wizfileiconprovider.h
    share/wizwindowshelper.h
    share/wizimagepushbutton.h
    share/wizindex.h
    share/wizthumbindex.h
    share/wizxml.h
    share/wizsyncthread.h
    share/wizenc.h
    share/wizziwreader.h
    html/wizhtmlreader.h
    html/wizhtmlcollector.h
    wizdocumentlistview.h
    wizdocumentview.h
    wizdocumentwebview.h
    wizdef.h
    wizactions.h
    wiznotestyle.h
    wizcategoryview.h
    wizdownloadobjectdatadialog.h
    wizdocumenthistory.h
    wizattachmentlistwidget.h
    wiztaglistwidget.h
    wizconsoledialog.h
    wizpreferencedialog.h
    wizaboutdialog.h
    wizwelcomedialog.h
    wizproxydialog.h
    wizcreateaccountdialog.h
    wizmainwindow.h
    wizupdater.h
    wizupdaterprogressdialog.h
    wizstatusbar.h
    wizupgradenotifydialog.h
    wizcertmanager.h
    wizusercipherform.h
    wiznoteinfoform.h
    newfolderdialog.h
    newtagdialog.h
)

set(wiznote_FORMS
    ui/newfolderdialog.ui 
    ui/newtagdialog.ui
    ui/wizdownloadobjectdatadialog.ui
    ui/wizconsoledialog.ui
    ui/wizpreferencedialog.ui
    ui/wizaboutdialog.ui
    ui/wizwelcomedialog.ui
    ui/wizproxydialog.ui
    ui/wizcreateaccountdialog.ui
    ui/wizupdaterprogressdialog.ui
    ui/wizupgradenotifydialog.ui
    ui/wizusercipherform.ui
    ui/wiznoteinfoform.ui
)

set(wiznote_TRANSLATIONS
    wiznote_zh_CN.ts
    wiznote_zh_TW.ts
)

# collect all resources files
#file(GLOB_RECURSE wiznote_RESOURCES RELATIVE ${CMAKE_SOURCE_DIR}/share/wiznote ${CMAKE_SOURCE_DIR}/share/wiznote/*)

QT4_ADD_TRANSLATION(wiznote_QM ${wiznote_TRANSLATIONS})

# include uic generated ui_*.h headers
include_directories(${CMAKE_CURRENT_BINARY_DIR})
qt4_wrap_ui(wiznote_FORM_HEADERS ${wiznote_FORMS})

# cmake will autogen zconf.h at BINARY_DIR
#include_directories(${testcmake_SOURCE_DIR}/zlib ${testcmake_BINARY_DIR}/zlib)
#link_directories(${testcmake_BINARY_DIR}/zlib)


include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/)

link_directories(${wiznote_BINARY_DIR}/lib/)

if(UNIX)
    if(APPLE)
        add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${wiznote_SOURCES} ${wiznote_FORM_HEADERS} ${wiznote_QM})

        # plist info
        set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_ICON_FILE wiznote.icns)

        # copy resources to bundle
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Resources/)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/share/wiznote/ ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Resources/)

        # resovle library dependency
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND macdeployqt ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app)

    elseif(NOT APPLE)
        add_executable(${PROJECT_NAME} ${wiznote_SOURCES} ${wiznote_FORM_HEADERS} ${wiznote_QM})
    endif()
endif()

target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES} cryptlib)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(UNIX)
    if(NOT APPLE)
        add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD COMMAND cp -R ${CMAKE_HOME_DIRECTORY}/share/wiznote/* ${CMAKE_BINARY_DIR})
    endif()
endif()

# moc related
set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC true)

# platform specific argument should be passed in
if(UNIX)
    if(APPLE)
        set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC_MOC_OPTIONS "-DQ_OS_MAC")
    elseif(NOT APPLE)
        set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC_MOC_OPTIONS "-DQ_OS_LINUX")
    endif()
elseif(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC_MOC_OPTIONS "-DQ_OS_WIN32")
endif()

