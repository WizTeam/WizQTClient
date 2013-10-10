project(wiznote)

set(wiznote_SOURCES_MAC
    mac/wizSearchWidget.mm
    mac/wizmachelper.mm
    mac/qmaccocoaviewcontainer.mm
)

set(wiznote_HEADERS_MAC
    mac/wizmachelper.h
    mac/wizmachelper_mm.h
    mac/wizSearchWidget_mm.h
    mac/qmaccocoaviewcontainer.h
)

set(wiznote_SOURCES
    share/wizzip.cpp
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
    share/wizDatabase.cpp
    share/wizDatabaseManager.cpp
    share/wizverifyaccount.cpp
    share/wizsettings.cpp

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
    share/wizIndexBase.cpp
    share/wizIndex.cpp
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
    wizAboutDialog.cpp
    wizLoginDialog.cpp
    wizproxydialog.cpp
    wizcreateaccountdialog.cpp
    wizmainwindow.cpp
    wizupdaterprogressdialog.cpp
    wizstatusbar.cpp
    wizupgradenotifydialog.cpp
    wizcertmanager.cpp
    wizusercipherform.cpp
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
    wizProgressDialog.cpp
    wizFolderView.cpp
    widgets/wizScrollBar.cpp
    wizDocumentSelectionView.cpp
    share/wizApiEntry.cpp
    share/wizObjectDataDownloader.cpp
    share/wizUserAvatar.cpp
    wizDocumentTransitionView.cpp
    sync/wizkmxmlrpc.cpp
    sync/wizkmsync.cpp
    wizPopupButton.cpp
    widgets/wizUserInfoWidget.cpp
    widgets/wizSegmentedButton.cpp
    wizWebSettingsDialog.cpp
    sync/wizAvatarUploader.cpp
)

set(wiznote_HEADERS
    share/wizzip.h
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
    share/wizIndexBase.h
    share/wizIndex.h
    share/wizthumbindex.h
    share/wizThumbIndexCache.h
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
    wizAboutDialog.h
    wizLoginDialog.h
    wizproxydialog.h
    wizcreateaccountdialog.h
    wizmainwindow.h
    wizupdaterprogressdialog.h
    wizstatusbar.h
    wizupgradenotifydialog.h
    wizcertmanager.h
    wizusercipherform.h
    wiznoteinfoform.h
    wizLineInputDialog.h
    share/wizClucene.h
    share/wizSearchIndexer.h
    wizSearchWidget.h
    wizEditorInsertLinkForm.h
    wizUpgrade.h
    wizEditorToolBar.h
    sync/wizkmxmlrpc.h
    sync/wizkmsync.h
    share/wizSyncableDatabase.h
    sync/wizAvatarUploader.h
    wizWebSettingsDialog.h
    widgets/wizSegmentedButton.h
    widgets/wizUserInfoWidget.h
    wizPopupButton.h
    wizDocumentTransitionView.h

    wizButton.h
    wizEditorInsertTableForm.h
    wizFolderSelector.h
    wizProgressDialog.h
    wizFolderView.h
    widgets/wizScrollBar.h
    wizDocumentSelectionView.h
    share/wizApiEntry.h
    share/wizObjectDataDownloader.h
    share/wizUserAvatar.h

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

set(wiznote_FORMS
    ui/wizLineInputDialog.ui
    ui/wizdownloadobjectdatadialog.ui
    ui/wizconsoledialog.ui
    ui/wizpreferencedialog.ui
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

set(wiznote_RCS
    ${CMAKE_SOURCE_DIR}/resources/wiznote.qrc
)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif()

message("Wiznote whill compiling as ${CMAKE_BUILD_TYPE}")

find_package(Qt5 REQUIRED Core Widgets LinguistTools Xml Network WebKit WebKitWidgets)
set(CMAKE_AUTOMOC ON)
qt5_add_translation(wiznote_QM ${wiznote_TRANSLATIONS})
qt5_wrap_ui(wiznote_FORM_HEADERS ${wiznote_FORMS})
qt5_add_resources(wiznote_RC ${wiznote_RCS})

message("Compile with Qt: ${Qt5_VERSION}.")


# build
if(UNIX)
    if(APPLE)
        set(wiznote_LOGO ${CMAKE_SOURCE_DIR}/build/common/logo/wiznote.icns)

        # we should copy qt internal qm files, otherwise qt can't translate some internal resources
        # and also macdeployqt will not do this work for us
        # refer to: https://bugreports.qt-project.org/browse/QTBUG-18662
        get_filename_component(_qt5_TRANSLATIONS ${Qt5_DIR}/../../../translations/ ABSOLUTE)
        file(GLOB _qt5_QM ${_qt5_TRANSLATIONS}/qt_*.qm)

        add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${wiznote_SOURCES} ${wiznote_HEADERS} ${wiznote_SOURCES_MAC} ${wiznote_HEADERS_MAC} ${wiznote_FORM_HEADERS} ${wiznote_QM} ${_qt5_QM} ${wiznote_RC} ${wiznote_LOGO})
        set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bundle)
        set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC_MOC_OPTIONS "-DQ_OS_MAC")

        # this should be issue of cmake, refer: http://public.kitware.com/Bug/bug_revision_view_page.php?rev_id=958
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-F /Library/Frameworks")

        # qmake will do this automatically
        set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-mmacosx-version-min=10.6")

        # suppress cmake warnning when link library is not a target, refer to cmp0016 for detailed info.
        cmake_policy(SET CMP0016 OLD)
        find_library(_CARBON_LIBRARY Carbon)
        find_library(_COCOA_LIBRARY Cocoa)
        target_link_libraries(${PROJECT_NAME} ${_CARBON_LIBRARY} ${_COCOA_LIBRARY})

        # setup plist info and logo
        set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/build/osx/Info.plist")
        set_source_files_properties(${wiznote_LOGO} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
        set_source_files_properties(${wiznote_QM} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/locales)
        set_source_files_properties(${_qt5_QM} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/locales)

        # copy resources to bundle
        set(wiznote_BUNDLE ${CMAKE_BINARY_DIR}/bundle/${PROJECT_NAME}.app)
        set(wiznote_BUNDLE_CONTENTS ${CMAKE_BINARY_DIR}/bundle/${PROJECT_NAME}.app/Contents)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/share/ ${wiznote_BUNDLE_CONTENTS}/Resources/)

        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            get_filename_component(_qt5_BINARY_DIR ${Qt5_DIR}/../../../bin/ ABSOLUTE)
            # resovle library dependency if build type is release
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${_qt5_BINARY_DIR}/macdeployqt ${wiznote_BUNDLE}
                COMMAND ln -s /Applications ${CMAKE_BINARY_DIR}/bundle/Applications
            )
        endif()

    elseif(NOT APPLE)
        add_executable(${PROJECT_NAME} ${wiznote_SOURCES} ${wiznote_HEADERS} ${wiznote_FORM_HEADERS} ${wiznote_QM} ${wiznote_RC})
        set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC_MOC_OPTIONS "-DQ_OS_LINUX")

        install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
        install(DIRECTORY ${CMAKE_SOURCE_DIR}/share/ DESTINATION share/wiznote PATTERN ".DS_Store" EXCLUDE)
        install(FILES ${wiznote_QM} DESTINATION share/wiznote/locales)

        # install icons and desktop entry
        install(FILES ${CMAKE_SOURCE_DIR}/build/common/wiznote.desktop DESTINATION share/applications)
        install(FILES ${CMAKE_SOURCE_DIR}/build/common/logo/wiznote16.png DESTINATION share/icons/hicolor/16x16/apps RENAME wiznote.png)
        install(FILES ${CMAKE_SOURCE_DIR}/build/common/logo/wiznote24.png DESTINATION share/icons/hicolor/24x24/apps RENAME wiznote.png)
        install(FILES ${CMAKE_SOURCE_DIR}/build/common/logo/wiznote32.png DESTINATION share/icons/hicolor/32x32/apps RENAME wiznote.png)
        install(FILES ${CMAKE_SOURCE_DIR}/build/common/logo/wiznote64.png DESTINATION share/icons/hicolor/64x64/apps RENAME wiznote.png)
        install(FILES ${CMAKE_SOURCE_DIR}/build/common/logo/wiznote128.png DESTINATION share/icons/hicolor/128x128/apps RENAME wiznote.png)
        install(FILES ${CMAKE_SOURCE_DIR}/build/common/logo/wiznote256.png DESTINATION share/icons/hicolor/256x256/apps RENAME wiznote.png)

        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/share/ ${CMAKE_BINARY_DIR}/share/wiznote)

            foreach(_file ${wiznote_QM})
                add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/share/wiznote/locales/
                    COMMAND ${CMAKE_COMMAND} -E copy ${_file} ${CMAKE_BINARY_DIR}/share/wiznote/locales/)
            endforeach()
        endif()
    endif()
endif()

qt5_use_modules(${PROJECT_NAME} Core Widgets Xml Network WebKit WebKitWidgets)

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
    quazip
    cryptlib
    clucene-core-static
    clucene-shared-static
    ${CMAKE_DL_LIBS}
)
