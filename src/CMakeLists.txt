project(wiznote)

add_subdirectory(plugins)

set(wiznote_SOURCES_MAC
    mac/wizSearchWidget.mm
    mac/wizmachelper.mm
    mac/wizmactoolbar.mm
    mac/wizmactoolbardelegate.mm
    mac/wizmacactionhelper.cpp
    mac/wizUserInfoWidgetBaseMac.mm
    mac/wizUserInfoWidgetBaseMac_mm.cpp
    utils/notify.mm
)

set(wiznote_HEADERS_MAC
    mac/wizmachelper.h
    mac/wizmachelper_mm.h
    mac/wizSearchWidget_mm.h
    mac/wizmactoolbar.h
    mac/wizmactoolbardelegate.h
    mac/wizmacactionhelper.h
    mac/wizUserInfoWidgetBaseMac_mm.h
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
    sync/wizXmlRpcServer.cpp
    #share/wizapi.cpp
    share/wizDatabase.cpp
    share/wizDatabaseManager.cpp
    #share/wizverifyaccount.cpp
    share/wizsettings.cpp
    share/wizui.cpp
    share/wizmd5.cpp
    share/wizhtml2zip.cpp
    share/wizwin32helper.cpp
    share/wizdrawtexthelper.cpp
    share/wizuihelper.cpp
    #share/wizcreateaccount.cpp
    share/wizcommonui.cpp
    share/wizanimateaction.cpp
    share/wizpopupwidget.cpp
    share/wizmultilinelistwidget.cpp
    share/wizfileiconprovider.cpp
    share/wizwindowshelper.cpp
    share/wizIndexBase.cpp
    share/wizIndex.cpp
    share/wizthumbindex.cpp
    share/wizenc.cpp
    share/wizziwreader.cpp
    html/wizhtmlreader.cpp
    html/wizhtmlcollector.cpp
    main.cpp
    share/wizClucene.cpp
    share/wizSearchIndexer.cpp
    widgets/wizScrollBar.cpp
    sync/token.cpp
    sync/apientry.cpp
    share/wizObjectDataDownloader.cpp
    #share/wizUserAvatar.cpp
    sync/avatar.cpp
    sync/wizkmxmlrpc.cpp
    sync/wizkmsync.cpp
    sync/asyncapi.cpp
    #sync/wizCloudPool.cpp
    sync/avataruploader.cpp
    widgets/wizUserInfoWidget.cpp
    widgets/wizUserInfoWidgetBase.cpp
    widgets/wizSegmentedButton.cpp
    utils/stylehelper.cpp
    utils/pathresolve.cpp

    #coreplugin.cpp
    wizCategoryView.cpp
    wizCategoryViewItem.cpp
    wizDocumentListView.cpp
    wizDocumentListViewItem.cpp
    wizDocumentView.cpp
    wizDocumentWebView.cpp
    wizactions.cpp
    wiznotestyle.cpp
    wizLineInputDialog.cpp
    wizdocumenthistory.cpp
    wizattachmentlistwidget.cpp
    wiztaglistwidget.cpp
    wizconsoledialog.cpp
    wizpreferencedialog.cpp
    wizLoginDialog.cpp
    wizproxydialog.cpp
    wizcreateaccountdialog.cpp
    wizmainwindow.cpp
    #icore.cpp
    notecomments.cpp
    cellbutton.cpp
    titlebar.cpp
    infobar.cpp
    notifybar.cpp
    titleedit.cpp
    sync/sync.cpp
    #wizupdaterprogressdialog.cpp
    wizstatusbar.cpp
    wizupgradenotifydialog.cpp
    #wizcertmanager.cpp
    wizusercipherform.cpp
    wiznoteinfoform.cpp
    wizSearchWidget.cpp
    wizEditorInsertLinkForm.cpp
    wizUpgrade.cpp
    wizButton.cpp
    wizEditorToolBar.cpp
    wizEditorInsertTableForm.cpp
    wizFolderSelector.cpp
    wizProgressDialog.cpp
    wizFolderView.cpp
    wizDocumentSelectionView.cpp
    wizDocumentTransitionView.cpp
    wizPopupButton.cpp
    wizWebSettingsDialog.cpp
    wizUserVerifyDialog.cpp
    plugindialog.cpp
    utils/logger.cpp
    utils/misc.cpp
    utils/pinyin.cpp
    utils/notify.cpp
    messagelistview.cpp
    thumbcache.cpp
    messagecompleter.cpp
    widgets/wizImageButton.cpp
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
    #share/wizapi.h
    share/wizxml.h
    share/wizxmlrpc.h
    sync/wizXmlRpcServer.h
    share/wizDatabase.h
    share/wizDatabaseManager.h
    #share/wizverifyaccount.h
    share/wizsettings.h
    share/wizui.h
    share/wizhtml2zip.h
    share/wizmd5.h
    share/wizwin32helper.h
    share/wizdrawtexthelper.h
    share/wizuihelper.h
    #share/wizcreateaccount.h
    share/wizcommonui.h
    share/wizanimateaction.h
    share/wizpopupwidget.h
    share/wizmultilinelistwidget.h
    share/wizfileiconprovider.h
    share/wizwindowshelper.h
    share/wizIndexBase.h
    share/wizIndex.h
    share/wizthumbindex.h
    share/wizenc.h
    share/wizziwreader.h
    html/wizhtmlreader.h
    html/wizhtmlcollector.h
    sync/wizkmxmlrpc.h
    sync/wizkmsync.h
    sync/asyncapi.h
    #sync/wizCloudPool.h
    sync/avataruploader.h
    share/wizSyncableDatabase.h
    share/wizClucene.h
    share/wizSearchIndexer.h
    widgets/wizSegmentedButton.h
    widgets/wizUserInfoWidget.h
    widgets/wizUserInfoWidgetBase.h
    widgets/wizScrollBar.h
    sync/token.h
    sync/token_p.h
    sync/apientry.h
    sync/apientry_p.h
    share/wizObjectDataDownloader.h
    #share/wizUserAvatar.h
    sync/avatar.h
    sync/avatar_p.h
    utils/stylehelper.h
    utils/pathresolve.h

    wizdef.h
    wizactions.h
    wiznotestyle.h
    wizCategoryView.h
    wizCategoryViewItem.h
    wizDocumentListView.h
    wizDocumentListViewItem.h
    wizDocumentView.h
    wizDocumentWebView.h
    wizdocumenthistory.h
    wizattachmentlistwidget.h
    wiztaglistwidget.h
    wizconsoledialog.h
    wizpreferencedialog.h
    wizLoginDialog.h
    wizproxydialog.h
    wizcreateaccountdialog.h
    wizmainwindow.h
    #icore.h
    notecomments.h
    notecomments_p.h
    cellbutton.h
    titlebar.h
    infobar.h
    notifybar.h
    titleedit.h
    sync/sync.h
    sync/sync_p.h
    #wizupdaterprogressdialog.h
    wizstatusbar.h
    wizupgradenotifydialog.h
    #wizcertmanager.h
    wizusercipherform.h
    wiznoteinfoform.h
    wizLineInputDialog.h
    wizSearchWidget.h
    wizEditorInsertLinkForm.h
    wizUpgrade.h
    wizEditorToolBar.h
    wizWebSettingsDialog.h
    wizPopupButton.h
    wizDocumentTransitionView.h
    wizButton.h
    wizEditorInsertTableForm.h
    wizFolderSelector.h
    wizProgressDialog.h
    wizFolderView.h
    wizDocumentSelectionView.h
    wizUserVerifyDialog.h
    plugindialog.h
    utils/logger.h
    utils/misc.h
    utils/pinyin.h
    utils/notify.h
    messagelistview.h
    thumbcache.h
    thumbcache_p.h
    messagecompleter.h
    widgets/wizImageButton.h
)

set(wiznote_FORMS
    ui/wizLineInputDialog.ui
    ui/wizconsoledialog.ui
    ui/wizpreferencedialog.ui
    ui/wizproxydialog.ui
    ui/wizcreateaccountdialog.ui
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

include(QtChooser)
qt_add_ui(wiznote_FORM_HEADERS ${wiznote_FORMS})
qt_add_resources(wiznote_RC ${wiznote_RCS})
qt_add_translation(wiznote_QM ${wiznote_TRANSLATIONS})

message("WizNote whll compiling as ${CMAKE_BUILD_TYPE}, use Qt: ${QT_VERSION}")

# build
if(UNIX)
    if(APPLE)
        set(wiznote_LOGO ${CMAKE_SOURCE_DIR}/build/common/logo/wiznote.icns)

        # we should copy qt internal qm files, otherwise qt can't translate some internal resources
        # and also macdeployqt will not do this work for us
        # refer to: https://bugreports.qt-project.org/browse/QTBUG-18662
        file(GLOB _qt5_QM ${qt_translations_dir}/qt_*.qm)

        add_executable(wiznote MACOSX_BUNDLE ${wiznote_SOURCES} ${wiznote_HEADERS} ${wiznote_SOURCES_MAC} ${wiznote_HEADERS_MAC} ${wiznote_FORM_HEADERS} ${wiznote_QM} ${_qt5_QM} ${wiznote_RC} ${wiznote_LOGO})
        set_target_properties(wiznote PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
        set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC_MOC_OPTIONS "-DQ_OS_MAC")

        # this should be issue of cmake, refer: http://public.kitware.com/Bug/bug_revision_view_page.php?rev_id=958
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-F /Library/Frameworks")

        # qmake will do this automatically
        set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-mmacosx-version-min=10.7")

        # suppress cmake warnning when link library is not a target, refer to cmp0016 for detailed info.
        cmake_policy(SET CMP0016 OLD)
        find_library(_CARBON_LIBRARY Carbon)
        find_library(_COCOA_LIBRARY Cocoa)

        # please set env: MACOSX_DEPLOYMENT_TARGET = 10.8 to ~/.bash_profile if you need NSUserNotification support.
        message("use cocoa library: ${_COCOA_LIBRARY}")
        target_link_libraries(${PROJECT_NAME} ${_CARBON_LIBRARY} ${_COCOA_LIBRARY})

        # setup plist info and logo
        set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/build/osx/Info.plist")
        set_source_files_properties(${wiznote_LOGO} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
        set_source_files_properties(${wiznote_QM} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/locales)
        set_source_files_properties(${_qt5_QM} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/locales)

        # copy resources to bundle
        #set(wiznote_BUNDLE ${CMAKE_BINARY_DIR}/bundle/${PROJECT_NAME}.app)
        #set(wiznote_BUNDLE_CONTENTS ${CMAKE_BINARY_DIR}/bundle/${PROJECT_NAME}.app/Contents)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/share/ ${WIZNOTE_RESOURCE_DIR})

        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            # resovle library dependency if build type is release
            add_dependencies(wiznote coreplugin_plugin helloworld_plugin)
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${qt_binary_dir}/macdeployqt ${WIZNOTE_ROOT_DIR}
            )
        endif()

    elseif(NOT APPLE)
        add_executable(wiznote ${wiznote_SOURCES} ${wiznote_HEADERS} ${wiznote_FORM_HEADERS} ${wiznote_QM} ${wiznote_RC})
        set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC_MOC_OPTIONS "-DQ_OS_LINUX")

        install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
        install(DIRECTORY ${CMAKE_SOURCE_DIR}/share/ DESTINATION share/wiznote PATTERN ".DS_Store" EXCLUDE)
        install(FILES ${wiznote_QM} DESTINATION share/wiznote/locales)
        install(FILES ${CMAKE_SOURCE_DIR}/LICENSE DESTINATION share/licenses/wiznote)
        install(FILES ${CMAKE_SOURCE_DIR}/build/common/wiznote.desktop DESTINATION share/applications)

        foreach(ICON_SIZE 8 16 24 32 48 64 128 256 512)
            install(FILES ${CMAKE_SOURCE_DIR}/build/common/logo/wiznote${ICON_SIZE}.png DESTINATION share/icons/hicolor/${ICON_SIZE}x${ICON_SIZE}/apps RENAME wiznote.png)
        endforeach(ICON_SIZE)

        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/share/ ${CMAKE_BINARY_DIR}/share/wiznote)

        foreach(_file ${wiznote_QM})
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/share/wiznote/locales/
                COMMAND ${CMAKE_COMMAND} -E copy ${_file} ${CMAKE_BINARY_DIR}/share/wiznote/locales/)
            endforeach()
    endif()
endif()

qt_use_modules(wiznote)
qt_suppress_warnings(wiznote)

include_directories(
    ${plugins_SOURCE_DIR}
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
    extensionsystem
    aggregation
    coreplugin
    ${CMAKE_DL_LIBS}
)
