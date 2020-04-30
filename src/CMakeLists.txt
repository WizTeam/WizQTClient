project(WizNote)

add_subdirectory(test)

set(wiznote_SOURCES_MAC
    mac/WizSearchWidget.mm
    mac/WizMacHelper.mm
    mac/WizMacToolBar.mm
    mac/WizMacToolBarDelegate.mm
    mac/WizMacActionHelper.cpp
    mac/WizUserInfoWidgetBaseMac.mm
    mac/WizUserInfoWidgetBaseMac_mm.cpp
    utils/WizNotify.mm
    mac/WizIAPHelper.mm
    mac/rmstore/RMAppReceipt.mm
    mac/rmstore/RMStoreAppReceiptVerificator.mm
    mac/WizNotificationCenter.mm
    widgets/WizIAPDialog.cpp
    mac/DTWebArchive.m
)

set(wiznote_HEADERS_MAC
    mac/WizMacHelper.h
    mac/WizMacHelper_mm.h
    mac/WizSearchWidget_mm.h
    mac/WizMacToolBar.h
    mac/WizMacToolBarDelegate.h
    mac/WizMacActionHelper.h
    mac/WizUserInfoWidgetBaseMac_mm.h
    mac/WizIAPHelper.h
    mac/rmstore/RMAppReceipt.h
    mac/rmstore/RMStoreAppReceiptVerificator.h
    mac/WizNotificationCenter.h
    widgets/WizIAPDialog.h
    mac/DTWebArchive.h
)


set(wiznote_FORMS_MAC
    ui/WizIAPDialog.ui
)

set(wiznote_SOURCES
    utils/WizLogger.cpp
    utils/WizMisc.cpp
    utils/WizPinyin.cpp
    utils/WizNotify.cpp
    share/WizZip.cpp
    share/WizQtHelper.cpp
    share/WizMisc.cpp
    share/sqlite3.c
    share/cppsqlite3.cpp
    share/WizObject.cpp
    share/WizKMCore.cpp
    share/WizGlobal.cpp
    share/WizDatabase.cpp
    share/WizDatabaseManager.cpp
    share/WizSettings.cpp
    share/WizUI.cpp
    share/WizMd5.cpp
    share/WizHtml2Zip.cpp
    share/WizWin32Helper.cpp
    share/WizDrawTextHelper.cpp
    share/WizUIHelper.cpp
    share/WizUIBase.cpp
    share/WizCommonUI.cpp
    share/WizAnimateAction.cpp
    share/WizPopupWidget.cpp
    share/WizMultiLineListWidget.cpp
    share/WizFileIconProvider.cpp
    share/WizWindowsHelper.cpp
    share/WizIndexBase.cpp
    share/WizIndex.cpp
    share/WizThumbIndex.cpp
    share/WizEnc.cpp
    share/WizZiwReader.cpp
    html/WizHtmlReader.cpp
    html/WizHtmlCollector.cpp    
    share/WizSearch.cpp
    widgets/WizScrollBar.cpp
    sync/WizToken.cpp
    sync/WizApiEntry.cpp
    share/WizObjectDataDownloader.cpp
    sync/WizAvatarHost.cpp
    sync/WizKMServer.cpp
    sync/WizKMSync.cpp
    sync/WizAsyncApi.cpp
    sync/WizAvatarUploader.cpp
    sync/WizJSONServerBase.cpp
    sync/WizSync.cpp
    widgets/WizUserInfoWidget.cpp
    widgets/WizUserInfoWidgetBase.cpp
    widgets/WizSegmentedButton.cpp
    utils/WizStyleHelper.cpp
    utils/WizPathResolve.cpp
    share/WizWindowTitleBar.cpp
    share/WizShadowEffect.cpp
    widgets/WizTagBar.cpp
    widgets/WizTrayIcon.cpp
    share/WizEventLoop.cpp
    share/WizObjectOperator.cpp
    widgets/WizAboutDialog.cpp
    widgets/WizLocalProgressWebView.cpp
    widgets/WizCrashReportDialog.cpp
    widgets/WizCustomToolBar.cpp
    widgets/WizTipsWidget.cpp
    widgets/WizSingleDocumentView.cpp
    widgets/WizShareLinkDialog.cpp
    share/WizAnalyzer.cpp
    share/WizTranslater.cpp
    widgets/WizAdvancedSearchDialog.cpp
    share/WizUdpClient.cpp
    share/WizMessageBox.cpp
    share/WizFileMonitor.cpp
    share/WizLockedFile.cpp
    share/WizLocalPeer.cpp
    share/WizSingleApplication.cpp
    widgets/WizImageButton.cpp
    widgets/WizCodeEditorDialog.cpp
    widgets/WizFramelessWebDialog.cpp
    widgets/WizScreenShotWidget.cpp
    widgets/WizVerificationCodeDialog.cpp
    widgets/WizEmailShareDialog.cpp
    widgets/WizTemplatePurchaseDialog.cpp
    widgets/WizExecutingActionDialog.cpp
    share/WebSocketClientWrapper.cpp
    share/WebSocketTransport.cpp
    share/WizWebEngineView.cpp
    widgets/WizTableSelector.cpp
    widgets/WizUserServiceExprDialog.cpp

    share/jsoncpp/jsoncpp.cpp
    share/WizRequest.cpp

    WizCategoryView.cpp
    WizCategoryViewItem.cpp
    WizDocumentListView.cpp
    WizDocumentListViewItem.cpp
    WizDocumentView.cpp
    WizDocumentWebView.cpp
    WizActions.cpp
    WizNoteStyle.cpp
    WizLineInputDialog.cpp
    WizDocumentHistory.cpp
    WizAttachmentListWidget.cpp
    WizTagListWidget.cpp
    WizConsoleDialog.cpp
    WizPreferenceDialog.cpp
    WizProxyDialog.cpp
    WizCreateAccountDialog.cpp
    WizMainWindow.cpp
    WizCellButton.cpp
    WizTitleBar.cpp
    WizInfoBar.cpp
    WizNotifyBar.cpp
    WizTitleEdit.cpp
    WizStatusBar.cpp
    WizUpgradeNotifyDialog.cpp
    WizUserCipherForm.cpp
    WizNoteInfoForm.cpp
    WizSearchWidget.cpp
    WizEditorInsertLinkForm.cpp
    WizUpgrade.cpp
    WizButton.cpp
    WizEditorToolBar.cpp
    WizEditorInsertTableForm.cpp
    WizFileImporter.cpp
    WizFolderSelector.cpp
    WizProgressDialog.cpp
    WizFolderView.cpp
    WizDocumentSelectionView.cpp
    WizDocumentTransitionView.cpp
    WizPopupButton.cpp
    WizWebSettingsDialog.cpp
    WizUserVerifyDialog.cpp
    WizMessageListView.cpp
    WizThumbCache.cpp
    WizMessageCompleter.cpp
    WizDocumentEditStatus.cpp
    WizLoginDialog.cpp
    WizSearchReplaceWidget.cpp
    WizDocTemplateDialog.cpp
    WizMobileFileReceiver.cpp
    WizWebEngineInjectObject.cpp
    WizCombineNotesDialog.cpp
    WizSvgEditorDialog.cpp

    WizOEMSettings.cpp    
    mac/WizSearchWidget_mm.cpp
    core/WizAccountManager.cpp
    core/WizCommentManager.cpp
    core/WizNoteManager.cpp
    share/WizThreads.cpp
    WizPositionDelegate.cpp
    main.cpp
    WizInitBizCertDialog.cpp
    share/WizDocumentStyle.cpp

    WizPlugins.cpp
)

set(wiznote_HEADERS
    share/WizZip.h
    share/WizQtHelper.h
    share/WizMisc.h
    share/sqlite3ext.h
    share/sqlite3.h
    share/cppsqlite3.h
    share/WizObject.h
    share/WizKMCore.h
    share/WizDatabase.h
    share/WizDatabaseManager.h
    share/WizSettings.h
    share/WizUI.h
    share/WizHtml2Zip.h
    share/WizMd5.h
    share/WizWin32Helper.h
    share/WizDrawTextHelper.h
    share/WizUIHelper.h
    share/WizUIBase.h
    share/WizCommonUI.h
    share/WizAnimateAction.h
    share/WizPopupWidget.h
    share/WizMultiLineListWidget.h
    share/WizFileIconProvider.h
    share/WizWindowsHelper.h
    share/WizIndexBase.h
    share/WizIndex.h
    share/WizThumbIndex.h
    share/WizEnc.h
    share/WizZiwReader.h
    share/WizGlobal.h
    html/WizHtmlReader.h
    html/WizHtmlCollector.h
    sync/WizKMServer.h
    sync/WizKMSync.h
    sync/WizAsyncApi.h
    sync/WizAvatarUploader.h
    share/WizSyncableDatabase.h
    share/WizSearch.h
    widgets/WizAboutDialog.h
    widgets/WizSegmentedButton.h
    widgets/WizUserInfoWidget.h
    widgets/WizUserInfoWidgetBase.h
    widgets/WizScrollBar.h
    sync/WizToken.h
    sync/WizToken_p.h
    sync/WizApiEntry.h
    share/WizObjectDataDownloader.h
    sync/WizAvatarHost.h
    sync/WizAvatarHost_p.h
    utils/WizStyleHelper.h
    utils/WizPathResolve.h
    share/WizWindowTitleBar.h
    share/WizShadowWindow.h
    share/WizShadowEffect.h
    widgets/WizTableSelector.h

    share/jsoncpp/json/json.h
    share/jsoncpp/json/json-forwards.h
    share/WizRequest.h

    WizDef.h
    WizActions.h
    WizNoteStyle.h
    WizCategoryView.h
    WizCategoryViewItem.h
    WizDocumentListView.h
    WizDocumentListViewItem.h
    WizDocumentView.h
    WizDocumentWebView.h
    WizDocumentHistory.h
    WizAttachmentListWidget.h
    WizTagListWidget.h
    WizConsoleDialog.h
    WizPreferenceDialog.h
    WizProxyDialog.h
    WizCreateAccountDialog.h
    WizMainWindow.h
    WizCellButton.h
    WizTitleBar.h
    WizInfoBar.h
    WizNotifyBar.h
    WizTitleEdit.h
    sync/WizSync.h
    sync/WizKMSync_p.h
    WizStatusBar.h
    WizUpgradeNotifyDialog.h
    WizUserCipherForm.h
    WizNoteInfoForm.h
    WizLineInputDialog.h
    WizSearchWidget.h
    WizEditorInsertLinkForm.h
    WizUpgrade.h
    WizEditorToolBar.h
    WizWebSettingsDialog.h
    WizPopupButton.h
    WizDocumentTransitionView.h
    WizButton.h
    WizEditorInsertTableForm.h
    WizFileImporter.h
    WizFolderSelector.h
    WizProgressDialog.h
    WizFolderView.h
    WizDocumentSelectionView.h
    WizUserVerifyDialog.h
    utils/WizLogger.h
    utils/WizMisc.h
    utils/WizPinyin.h
    utils/WizNotify.h
    WizMessageListView.h
    WizThumbCache.h
    WizThumbCache_p.h
    WizMessageCompleter.h
    widgets/WizImageButton.h
    WizDocumentEditStatus.h
    WizLoginDialog.h
    share/WizFileMonitor.h
    WizSearchReplaceWidget.h
    widgets/WizCodeEditorDialog.h
    WizDocTemplateDialog.h
    WizMobileFileReceiver.h
    widgets/WizFramelessWebDialog.h
    share/WizLockedFile.h
    share/WizLocalPeer.h
    share/WizSingleApplication.h
    widgets/WizScreenShotWidget.h
    widgets/WizVerificationCodeDialog.h
    widgets/WizEmailShareDialog.h
    share/WebSocketClientWrapper.h
    share/WebSocketTransport.h
    share/WizWebEngineView.h
    WizWebEngineInjectObject.h
    widgets/WizShareLinkDialog.h
    share/WizAnalyzer.h
    share/WizTranslater.h
    widgets/WizAdvancedSearchDialog.h
    share/WizUdpClient.h
    share/WizMessageBox.h
    WizOEMSettings.h
    widgets/WizTagBar.h
    widgets/WizTrayIcon.h
    share/WizEventLoop.h
    sync/WizJSONServerBase.h
    share/WizObjectOperator_p.h
    share/WizObjectOperator.h
    widgets/WizLocalProgressWebView.h
    widgets/WizCrashReportDialog.h
    widgets/WizCustomToolBar.h
    widgets/WizTipsWidget.cpp
    widgets/WizSingleDocumentView.h
    widgets/WizTemplatePurchaseDialog.h
    widgets/WizExecutingActionDialog.h
    widgets/WizUserServiceExprDialog.h
    WizPositionDelegate.h
    core/WizAccountManager.h
    core/WizCommentManager.h
    core/WizNoteManager.h
    share/WizThreads.h
    share/WizThreads_p.h
    WizInitBizCertDialog.h
    share/WizDocumentStyle.h
    WizCombineNotesDialog.h
    WizSvgEditorDialog.h

    WizPlugins.h
)

set(wiznote_FORMS
    ui/WizLineInputDialog.ui
    ui/WizConsoleDialog.ui
    ui/WizPreferenceDialog.ui
    ui/WizProxyDialog.ui
    ui/WizCreateAccountDialog.ui
    ui/WizUpgradeNotifyDialog.ui
    ui/WizUserCipherForm.ui
    ui/WizNoteInfoForm.ui
    ui/WizEditorInsertLinkForm.ui
    ui/WizEditorInsertTableForm.ui
    ui/WizProgressDialog.ui
    ui/WizLoginDialog.ui
    ui/WizSearchReplaceWidget.ui
    ui/WizDocTemplateDialog.ui
    ui/WizVerificationCodeDialog.ui
    ui/WizEmailShareDialog.ui
    ui/WizAdvancedSearchDialog.ui
    ui/WizCrashReportDialog.ui
    ui/WizTemplatePurchaseDialog.ui
    ui/WizExecutingActionDialog.ui
    ui/WizInitBizCertDialog.ui
    ui/WizUserServiceExprDialog.ui
    ui/WizCombineNotesDialog.ui
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

message("WizNote whill compiling as ${CMAKE_BUILD_TYPE}, use Qt: ${QT_VERSION}")

# build
if(UNIX)
    if(APPLE)
        qt_add_ui(wiznote_FORM_HEADERS ${wiznote_FORMS_MAC})

        set(wiznote_LOGO ${CMAKE_SOURCE_DIR}/build/common/logo/wiznote.icns)

        # we should copy qt internal qm files, otherwise qt can't translate some internal resources
        # and also macdeployqt will not do this work for us
        # refer to: https://bugreports.qt-project.org/browse/QTBUG-18662
        file(GLOB _qt5_QM ${qt_translations_dir}/qt_*.qm)

        add_executable(WizNote MACOSX_BUNDLE  ${wiznote_HEADERS} ${wiznote_HEADERS_MAC} ${wiznote_FORM_HEADERS} ${wiznote_SOURCES_MAC} ${wiznote_SOURCES} ${wiznote_QM} ${_qt5_QM} ${wiznote_RC} ${wiznote_LOGO})
        set_target_properties(WizNote PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
        set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC_MOC_OPTIONS "-DQ_OS_MAC")

        # this should be issue of cmake, refer: http://public.kitware.com/Bug/bug_revision_view_page.php?rev_id=958
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-F /System/Library/Frameworks")

        # qmake will do this automatically
        set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-mmacosx-version-min=10.7")

        # suppress cmake warnning when link library is not a target, refer to cmp0016 for detailed info.
        cmake_policy(SET CMP0016 OLD)
        find_library(_CARBON_LIBRARY Carbon)
        find_library(_COCOA_LIBRARY Cocoa)
        find_library(_WEBENGINE_LIBRARY WebEngine)
        find_library(_WEBENGINEWIDGETS_LIBRARY WebEngineWidgets)
        find_library(_WEBCHANNEL_LIBRARY WebChannel)
        find_library(_STORE_LIBRARY StoreKit)
        find_library(_IO_LIBRARY IOKit)
        find_package(Qt5Svg)
        if (PLCrashReporter)
            find_library(_CRASHREPORTER CrashReporter)
        endif(PLCrashReporter)

        # please set env: MACOSX_DEPLOYMENT_TARGET = 10.8 to ~/.bash_profile if you need NSUserNotification support.
        message("use cocoa library: ${_COCOA_LIBRARY}")
        target_link_libraries(${PROJECT_NAME} ${_CARBON_LIBRARY} ${_COCOA_LIBRARY}
                  ${_STORE_LIBRARY} ${_IO_LIBRARY} ${_CRASHREPORTER})

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
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/build/osx/localize/ ${WIZNOTE_RESOURCE_DIR})

    elseif(NOT APPLE)
        add_executable(WizNote ${wiznote_SOURCES} ${wiznote_HEADERS} ${wiznote_FORM_HEADERS} ${wiznote_QM} ${wiznote_RC})
        set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC_MOC_OPTIONS "-DQ_OS_LINUX")
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-static-libstdc++ -static-libgcc")
        find_package(Qt5Svg)

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
else()
    add_definitions(-DUNICODE=TRUE)
    add_definitions(-D_UNICODE=TRUE)
    add_executable(WizNote WIN32 ${wiznote_SOURCES} ${wiznote_HEADERS} ${wiznote_FORM_HEADERS} ${wiznote_QM} ${wiznote_RC})
    set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC_MOC_OPTIONS "-DQ_OS_WIN32")

    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/share/ DESTINATION share/wiznote PATTERN ".DS_Store" EXCLUDE)
    install(FILES ${wiznote_QM} DESTINATION share/wiznote/locales)
    install(FILES ${CMAKE_SOURCE_DIR}/LICENSE DESTINATION share/licenses/wiznote)

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/share/ ${CMAKE_BINARY_DIR}/share/wiznote)

    foreach(_file ${wiznote_QM})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/share/wiznote/locales/
            COMMAND ${CMAKE_COMMAND} -E copy ${_file} ${CMAKE_BINARY_DIR}/share/wiznote/locales/)
    endforeach()
endif()

qt_use_modules(WizNote)
qt_suppress_warnings(WizNote)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/openssl/include
    )

if (APPLE)
    include_directories(~/Library/Frameworks)
elseif(WIN32)
    include_directories(
        ${CMAKE_SOURCE_DIR}/lib/zlib
        ${CMAKE_BINARY_DIR}/lib/zlib
        )
endif()


link_directories(
    ${CMAKE_BINARY_DIR}/bin
)

if(UNIX)
	if(APPLE)
		if(APPSTORE_BUILD)
			target_link_libraries(${PROJECT_NAME}
				quazip
				cryptlib
                Qt5::Svg
				Qt5::WebEngine
                Qt5::WebChannel
				Qt5::WebEngineWidgets
				${CMAKE_SOURCE_DIR}/lib/openssl/lib/libcrypto.a
				${CMAKE_SOURCE_DIR}/lib/openssl/lib/libssl.a
				${CMAKE_DL_LIBS}
			)
		else(APPSTORE_BUILD)
			target_link_libraries(${PROJECT_NAME}
				quazip
				cryptlib
                Qt5::Svg
                Qt5::WebEngine
                Qt5::WebChannel
				Qt5::WebEngineWidgets
				${CMAKE_SOURCE_DIR}/lib/openssl/lib/libcrypto.a
				${CMAKE_SOURCE_DIR}/lib/openssl/lib/libssl.a
				${CMAKE_DL_LIBS}
			)
		endif(APPSTORE_BUILD)
	else(APPLE) #linux
		target_link_libraries(${PROJECT_NAME}
                        -lpthread
                        X11
			quazip
			cryptlib
            Qt5::Svg
            Qt5::WebEngine
            Qt5::WebChannel
			Qt5::WebEngineWidgets
			${CMAKE_DL_LIBS}
		)
	endif(APPLE)
else(NOT UNIX)
	target_link_libraries(${PROJECT_NAME}
		quazip
		cryptlib
        Qt5::Svg
        Qt5::WebEngine
        Qt5::WebChannel
		Qt5::WebEngineWidgets
		${CMAKE_DL_LIBS}
                Qt5::WinMain
	)
endif()
