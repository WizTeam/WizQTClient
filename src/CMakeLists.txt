project(wiznote)

set(wiznote_SOURCES_MAC
    widgets/qkineticscroller_mac.mm
    mac/wizSearchWidget.mm
    mac/wizmachelper.mm
)

set(wiznote_HEADERS_MAC
    mac/wizmachelper.h
)

set(wiznote_SOURCES
    widgets/qkineticscroller.cpp
    widgets/qscrollareakineticscroller.cpp
    widgets/qwebviewkineticscroller.cpp
    widgets/qsegmentcontrol.cpp
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
    share/wizxml.cpp
    share/wizxmlrpc.cpp
    share/wizXmlRpcServer.cpp
    share/wizapi.cpp
    share/wizKbSync.cpp
    share/wizSync.cpp
    share/wizDatabase.cpp
    share/wizDatabaseManager.cpp
    share/wizverifyaccount.cpp
    share/wizsettings.cpp
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
    share/wizThumbIndexCache.cpp
    share/wizenc.cpp
    share/wizziwreader.cpp
    html/wizhtmlreader.cpp
    html/wizhtmlcollector.cpp
    main.cpp
    wizCategoryView.cpp
    wizCategoryViewItem.cpp
    wizDocumentListView.cpp
    wizDocumentListViewItem.cpp
    wizDocumentView.cpp
    wizDocumentWebView.cpp
    wizactions.cpp
    wiznotestyle.cpp
    wizLineInputDialog.cpp
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
    wizGroupAttributeForm.cpp
    wiznoteinfoform.cpp
    share/wizClucene.cpp
    share/wizSearchIndexer.cpp
    wizSearchWidget.cpp
    wizEditorInsertLinkForm.cpp
    wizUpgrade.cpp
    wizButton.cpp
    wizEditorToolBar.cpp
    wizEditorInsertTableForm.cpp
    wizFolderSelector.cpp
    #share/wizIOHostThread.cpp
    wizProgressDialog.cpp
    wizFolderView.cpp
    widgets/wizScrollBar.cpp
    wizDocumentSelectionView.cpp
    share/wizGroupMessage.cpp
    share/wizApiEntry.cpp
    share/wizObjectDataDownloader.cpp
    share/wizUserAvatar.cpp
    wizDocumentTransitionView.cpp
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
    share/wizxml.h
    share/wizxmlrpc.h
    share/wizXmlRpcServer.h
    share/wizKbSync.h
    share/wizSync.h
    share/wizDatabase.h
    share/wizDatabaseManager.h
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
    share/wizThumbIndexCache.h
    share/wizsyncthread.h
    share/wizenc.h
    share/wizziwreader.h
    html/wizhtmlreader.h
    html/wizhtmlcollector.h
    wizdef.h
    wizactions.h
    wiznotestyle.h
    wizCategoryView.h
    wizCategoryViewItem.h
    wizDocumentListView.h
    wizDocumentListViewItem.h
    wizDocumentView.h
    wizDocumentWebView.h
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
    wizGroupAttributeForm.h
    wiznoteinfoform.h
    wizLineInputDialog.h
    share/wizClucene.h
    share/wizSearchIndexer.h
    wizSearchWidget.h
    wizEditorInsertLinkForm.h
    wizUpgrade.h
    wizEditorToolBar.h

    # rapidjson
    rapidjson/allocators.h
    rapidjson/document.h
    rapidjson/encodedstream.h
    rapidjson/encodings.h
    rapidjson/filereadstream.h
    rapidjson/filestream.h
    rapidjson/filewritestream.h
    rapidjson/prettywriter.h
    rapidjson/rapidjson.h
    rapidjson/reader.h
    rapidjson/stringbuffer.h
    rapidjson/reader.h
    rapidjson/internal/pow10.h
    rapidjson/internal/stack.h
    rapidjson/internal/strfunc.h
)

set(wiznote_WRAP_HEADERS
    widgets/qkineticscroller_p.h
    widgets/qkineticscroller.h
    widgets/qscrollareakineticscroller.h
    widgets/qwebviewkineticscroller.h
    widgets/qsegmentcontrol.h
    wizaboutdialog.h
    zip/quazipfile.h
    share/wizanimateaction.h
    share/wizapi.h
    wizattachmentlistwidget.h
    wizcertmanager.h
    share/wizcommonui.h
    wizconsoledialog.h
    share/wizcreateaccount.h
    wizcreateaccountdialog.h
    share/wizDatabase.h
    share/wizDatabaseManager.h
    wizCategoryView.h
    wizDocumentListView.h
    wizDocumentView.h
    wizDocumentWebView.h
    share/wizdownloadobjectdata.h
    wizdownloadobjectdatadialog.h
    wizEditorInsertLinkForm.h
    wizGroupAttributeForm.h
    share/wizindex.h
    share/wizKbSync.h
    wizmainwindow.h
    share/wizmisc.h
    wizLineInputDialog.h
    wiznoteinfoform.h
    share/wizpopupwidget.h
    wizpreferencedialog.h
    wizproxydialog.h
    wizSearchWidget.h
    share/wizSearchIndexer.h
    wizstatusbar.h
    share/wizSync.h
    share/wizsyncthread.h
    wiztaglistwidget.h
    share/wizThumbIndexCache.h
    wizupdater.h
    wizupdaterprogressdialog.h
    wizUpgrade.h
    wizupgradenotifydialog.h
    wizusercipherform.h
    share/wizverifyaccount.h
    wizwelcomedialog.h
    share/wizXmlRpcServer.h
    share/wizziwreader.h
    #share/wizuihelper.h
    wizButton.h
    wizEditorToolBar.h
    wizEditorInsertTableForm.h
    wizFolderSelector.h
    #share/wizIOHostThread.h
    wizProgressDialog.h
    wizFolderView.h
    widgets/wizScrollBar.h
    wizDocumentSelectionView.h
    share/wizGroupMessage.h
    share/wizApiEntry.h
    share/wizObjectDataDownloader.h
    share/wizUserAvatar.h
    wizDocumentTransitionView.h
)

set(wiznote_FORMS
    ui/wizLineInputDialog.ui
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
    ui/wizEditorInsertLinkForm.ui
    ui/wizEditorInsertTableForm.ui
    ui/wizProgressDialog.ui
)

set(wiznote_TRANSLATIONS
    ${CMAKE_SOURCE_DIR}/i18n/wiznote_zh_CN.ts
    ${CMAKE_SOURCE_DIR}/i18n/wiznote_zh_TW.ts
)

# resources
file(GLOB_RECURSE wiznote_RESOURCES ${CMAKE_SOURCE_DIR}/share/wiznote/*)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif()

message("Wiznote whill compiling as ${CMAKE_BUILD_TYPE}")

# use FindQt4.cmake module
find_package(Qt4 REQUIRED QtCore QtGui QtXml QtNetwork QtWebKit)
include(${QT_USE_FILE})

message("Compile with Qt: ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}. Location: ${QT_BINARY_DIR}")

# dynamically create ts files
qt4_add_translation(wiznote_QM ${wiznote_TRANSLATIONS})
# we should copy qt internal qm files, otherwise qt can't translate some internal resources
# https://bugreports.qt-project.org/browse/QTBUG-18662
file(GLOB _QT_INTERNAL_TRANSLATIONS ${QT_TRANSLATIONS_DIR}/qt_*.qm)

# uic
qt4_wrap_ui(wiznote_FORM_HEADERS ${wiznote_FORMS})

# moc, automoc feature is not exist if cmake version <= 2.8.4
if(UNIX)
    if(APPLE)
        qt4_wrap_cpp(wiznote_MOC_HEADERS ${wiznote_WRAP_HEADERS} OPTIONS "-DQ_OS_MAC")
    elseif(NOT APPLE)
        qt4_wrap_cpp(wiznote_MOC_HEADERS ${wiznote_WRAP_HEADERS} OPTIONS "-DQ_OS_LINUX")
    endif()
elseif(WIN32)
    qt4_wrap_cpp(wiznote_MOC_HEADERS ${wiznote_WRAP_HEADERS} OPTIONS "-DQ_OS_WIN32")
endif()

# cmake will autogen zconf.h at BINARY_DIR
#include_directories(${testcmake_SOURCE_DIR}/zlib ${testcmake_BINARY_DIR}/zlib)
#link_directories(${testcmake_BINARY_DIR}/zlib)


# build
if(UNIX)
    if(APPLE)

        add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${wiznote_SOURCES} ${wiznote_HEADERS} ${wiznote_SOURCES_MAC} ${wiznote_MOC_HEADERS} ${wiznote_FORM_HEADERS} ${wiznote_QM})
        set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bundle)

        # this should be issue of cmake, refer: http://public.kitware.com/Bug/bug_revision_view_page.php?rev_id=958
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-F /Library/Frameworks")

        # qmake will do this automatically
        set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-mmacosx-version-min=10.6")

        # suppress cmake warnning when link library is not a target, refer to cmp0016 for detailed info.
        cmake_policy(SET CMP0016 OLD)
        find_library(_CARBON_LIBRARY Carbon)
        find_library(_COCOA_LIBRARY Cocoa)
        target_link_libraries(${PROJECT_NAME} ${_CARBON_LIBRARY} ${_COCOA_LIBRARY})

        # setup plist info
        set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${wiznote_SOURCE_DIR}/../Info.plist")

        # copy resources to bundle
        set(wiznote_BUNDLE ${CMAKE_BINARY_DIR}/bundle/${PROJECT_NAME}.app)
        set(wiznote_BUNDLE_CONTENTS ${CMAKE_BINARY_DIR}/bundle/${PROJECT_NAME}.app/Contents)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/share/wiznote/ ${wiznote_BUNDLE_CONTENTS}/Resources/)

        foreach(_file ${_QT_INTERNAL_TRANSLATIONS})
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy ${_file} ${wiznote_BUNDLE_CONTENTS}/Resources/languages/)
        endforeach()

        # we should copy qm manually to avoid dependency issue
        #set_source_files_properties(${wiznote_QM} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/languages)
        foreach(_file ${wiznote_QM})
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy ${_file} ${wiznote_BUNDLE_CONTENTS}/Resources/languages/)
        endforeach()

        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            # resovle library dependency if build type is release
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND macdeployqt ${wiznote_BUNDLE}
                COMMAND ln -s /Applications ${CMAKE_BINARY_DIR}/bundle/Applications
                COMMAND hdiutil create -srcfolder ${CMAKE_BINARY_DIR}/bundle -volname ${PROJECT_NAME} ${CMAKE_BINARY_DIR}/wiznote.dmg)
        endif()

    elseif(NOT APPLE)
        add_executable(${PROJECT_NAME} ${wiznote_SOURCES} ${wiznote_MOC_HEADERS} ${wiznote_FORM_HEADERS} ${wiznote_QM})
        #set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

        # always build rpath
        set_target_properties(${PROJECT_NAME} PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
        set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "\$ORIGIN/../lib")

        install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
        install(DIRECTORY ${CMAKE_SOURCE_DIR}/share/ DESTINATION share PATTERN ".DS_Store" EXCLUDE)
        install(FILES ${wiznote_QM} ${_QT_INTERNAL_TRANSLATIONS} DESTINATION share/wiznote/languages)

        # install icons and desktop entry (debian)
        install(FILES ${CMAKE_SOURCE_DIR}/share/wiznote/skins/wiznote16.png DESTINATION share/icons/hicolor/16x16/apps RENAME wiznote.png)
        install(FILES ${CMAKE_SOURCE_DIR}/share/wiznote/skins/wiznote24.png DESTINATION share/icons/hicolor/24x24/apps RENAME wiznote.png)
        install(FILES ${CMAKE_SOURCE_DIR}/share/wiznote/skins/wiznote32.png DESTINATION share/icons/hicolor/32x32/apps RENAME wiznote.png)
        install(FILES ${CMAKE_SOURCE_DIR}/share/wiznote/skins/wiznote64.png DESTINATION share/icons/hicolor/64x64/apps RENAME wiznote.png)
        install(FILES ${CMAKE_SOURCE_DIR}/share/wiznote/skins/wiznote128.png DESTINATION share/icons/hicolor/128x128/apps RENAME wiznote.png)
        install(FILES ${CMAKE_SOURCE_DIR}/share/wiznote/skins/wiznote256.png DESTINATION share/icons/hicolor/256x256/apps RENAME wiznote.png)

        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/share/ ${CMAKE_BINARY_DIR}/share)

        foreach(_file ${wiznote_QM})
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy ${_file} ${CMAKE_BINARY_DIR}/share/wiznote/languages/)
        endforeach()

        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/lib/
                COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/plugins/
                COMMAND ${CMAKE_COMMAND} -E copy ${QT_LIBRARY_DIR}/libQtCore.so.4 ${CMAKE_BINARY_DIR}/lib/
                COMMAND ${CMAKE_COMMAND} -E copy ${QT_LIBRARY_DIR}/libQtGui.so.4 ${CMAKE_BINARY_DIR}/lib/
                COMMAND ${CMAKE_COMMAND} -E copy ${QT_LIBRARY_DIR}/libQtNetwork.so.4 ${CMAKE_BINARY_DIR}/lib/
                COMMAND ${CMAKE_COMMAND} -E copy ${QT_LIBRARY_DIR}/libQtWebKit.so.4 ${CMAKE_BINARY_DIR}/lib/
                COMMAND ${CMAKE_COMMAND} -E copy ${QT_LIBRARY_DIR}/libQtXml.so.4 ${CMAKE_BINARY_DIR}/lib/
                COMMAND ${CMAKE_COMMAND} -E copy ${QT_LIBRARY_DIR}/libQtXmlPatterns.so.4 ${CMAKE_BINARY_DIR}/lib/
                COMMAND ${CMAKE_COMMAND} -E copy_directory ${QT_PLUGINS_DIR} ${CMAKE_BINARY_DIR}/plugins/
            )
        endif()

    endif()
endif()

#set(CMAKE_CXX_FLAGS "-std=c++0x")

# bug fix: not use automoc for cmake 2.8.4 backward compatibility.
# moc, platform specific argument should be passed to moc
#if(UNIX)
#    if(APPLE)
#        set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC_MOC_OPTIONS "-DQ_OS_MAC")
#    elseif(NOT APPLE)
#        set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC_MOC_OPTIONS "-DQ_OS_LINUX")
#    endif()
#elseif(WIN32)
#    set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC_MOC_OPTIONS "-DQ_OS_WIN32")
#endif()
#set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC true)

#${clucene_SOURCE_DIR}/src/shared
#${clucene_BINARY_DIR}/src/shared
#${clucene_SOURCE_DIR}/src/core
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/clucene/src/core
    ${CMAKE_SOURCE_DIR}/lib/clucene/src/shared
    ${CMAKE_BINARY_DIR}/lib/clucene/src/shared
)

link_directories(
    ${CMAKE_BINARY_DIR}/bin
)

target_link_libraries(${PROJECT_NAME}
    cryptlib
    clucene-core-static
    clucene-shared-static
    ${QT_LIBRARIES}
    ${CMAKE_DL_LIBS}
)
