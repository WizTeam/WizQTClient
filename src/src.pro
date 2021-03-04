QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets webengine webenginewidgets svg xml websockets

macx {
    QMAKE_MAC_SDK = macosx10.14
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.14

    QT += macextras
    LIBS += -framework Carbon
    LIBS += -framework Cocoa
    LIBS += -framework StoreKit
    LIBS += -framework IOKit

    QMAKE_INFO_PLIST = ../build/osx/info.plist
    ICON = ../build/common/logo/wiznote.icns

    QMAKE_CXXFLAGS += -Wno-unused-value -Wno-unused-variable -Wno-unused-parameter -Wno-inconsistent-missing-override


}

TARGET = WizNote

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

macx {
DEFINES += USECOCOATOOLBAR
}

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

RESOURCES  += ../resources/wiznote.qrc

SOURCES += \
    mac/WizSearchWidget_mac.mm \
    utils/WizLogger.cpp \
    utils/WizMisc_utils.cpp \
    utils/WizNotify.cpp \
    utils/WizNotify_mac.mm \
    utils/WizPinyin.cpp \
    share/WizZip.cpp \
    share/WizQtHelper.cpp \
    share/WizMisc.cpp \
    share/sqlite3.c \
    share/cppsqlite3.cpp \
    share/WizObject.cpp \
    share/WizKMCore.cpp \
    share/WizGlobal.cpp \
    share/WizDatabase.cpp \
    share/WizDatabaseManager.cpp \
    share/WizSettings.cpp \
    share/WizUI.cpp \
    share/WizMd5.cpp \
    share/WizHtml2Zip.cpp \
    share/WizWin32Helper.cpp \
    share/WizDrawTextHelper.cpp \
    share/WizUIHelper.cpp \
    share/WizUIBase.cpp \
    share/WizCommonUI.cpp \
    share/WizAnimateAction.cpp \
    share/WizPopupWidget.cpp \
    share/WizMultiLineListWidget.cpp \
    share/WizFileIconProvider.cpp \
    share/WizWindowsHelper.cpp \
    share/WizIndexBase.cpp \
    share/WizIndex.cpp \
    share/WizThumbIndex.cpp \
    share/WizEnc.cpp \
    share/WizZiwReader.cpp \
    html/WizHtmlReader.cpp \
    html/WizHtmlCollector.cpp \
    share/WizSearch.cpp \
    widgets/WizScrollBar.cpp \
    sync/WizToken.cpp \
    sync/WizApiEntry.cpp \
    share/WizObjectDataDownloader.cpp \
    sync/WizAvatarHost.cpp \
    sync/WizKMServer.cpp \
    sync/WizKMSync.cpp \
    sync/WizAsyncApi.cpp \
    sync/WizAvatarUploader.cpp \
    sync/WizJSONServerBase.cpp \
    sync/WizSync.cpp \
    widgets/WizUserInfoWidget.cpp \
    widgets/WizUserInfoWidgetBase.cpp \
    widgets/WizSegmentedButton.cpp \
    utils/WizStyleHelper.cpp \
    utils/WizPathResolve.cpp \
    share/WizWindowTitleBar.cpp \
    share/WizShadowEffect.cpp \
    widgets/WizTagBar.cpp \
    widgets/WizTrayIcon.cpp \
    share/WizEventLoop.cpp \
    share/WizObjectOperator.cpp \
    widgets/WizAboutDialog.cpp \
    widgets/WizLocalProgressWebView.cpp \
    widgets/WizCrashReportDialog.cpp \
    widgets/WizCustomToolBar.cpp \
    widgets/WizTipsWidget.cpp \
    widgets/WizSingleDocumentView.cpp \
    widgets/WizShareLinkDialog.cpp \
    share/WizAnalyzer.cpp \
    share/WizTranslater.cpp \
    widgets/WizAdvancedSearchDialog.cpp \
    share/WizUdpClient.cpp \
    share/WizMessageBox.cpp \
    share/WizFileMonitor.cpp \
    share/WizLockedFile.cpp \
    share/WizLocalPeer.cpp \
    share/WizSingleApplication.cpp \
    widgets/WizImageButton.cpp \
    widgets/WizCodeEditorDialog.cpp \
    widgets/WizFramelessWebDialog.cpp \
    widgets/WizScreenShotWidget.cpp \
    widgets/WizVerificationCodeDialog.cpp \
    widgets/WizEmailShareDialog.cpp \
    widgets/WizTemplatePurchaseDialog.cpp \
    widgets/WizExecutingActionDialog.cpp \
    share/WebSocketClientWrapper.cpp \
    share/WebSocketTransport.cpp \
    share/WizWebEngineView.cpp \
    widgets/WizTableSelector.cpp \
    widgets/WizUserServiceExprDialog.cpp \
    share/jsoncpp/jsoncpp.cpp \
    share/WizRequest.cpp \
    WizCategoryView.cpp \
    WizCategoryViewItem.cpp \
    WizDocumentListView.cpp \
    WizDocumentListViewItem.cpp \
    WizDocumentView.cpp \
    WizDocumentWebView.cpp \
    WizActions.cpp \
    WizNoteStyle.cpp \
    WizLineInputDialog.cpp \
    WizDocumentHistory.cpp \
    WizAttachmentListWidget.cpp \
    WizTagListWidget.cpp \
    WizConsoleDialog.cpp \
    WizPreferenceDialog.cpp \
    WizProxyDialog.cpp \
    WizCreateAccountDialog.cpp \
    WizMainWindow.cpp \
    WizCellButton.cpp \
    WizTitleBar.cpp \
    WizInfoBar.cpp \
    WizNotifyBar.cpp \
    WizTitleEdit.cpp \
    WizStatusBar.cpp \
    WizUpgradeNotifyDialog.cpp \
    WizUserCipherForm.cpp \
    WizNoteInfoForm.cpp \
    WizSearchWidget.cpp \
    WizEditorInsertLinkForm.cpp \
    WizUpgrade.cpp \
    WizButton.cpp \
    WizEditorToolBar.cpp \
    WizEditorInsertTableForm.cpp \
    WizFileImporter.cpp \
    WizFolderSelector.cpp \
    WizProgressDialog.cpp \
    WizFolderView.cpp \
    WizDocumentSelectionView.cpp \
    WizDocumentTransitionView.cpp \
    WizPopupButton.cpp \
    WizWebSettingsDialog.cpp \
    WizUserVerifyDialog.cpp \
    WizMessageListView.cpp \
    WizThumbCache.cpp \
    WizMessageCompleter.cpp \
    WizDocumentEditStatus.cpp \
    WizLoginDialog.cpp \
    WizSearchReplaceWidget.cpp \
    WizDocTemplateDialog.cpp \
    WizMobileFileReceiver.cpp \
    WizWebEngineInjectObject.cpp \
    WizCombineNotesDialog.cpp \
    WizSvgEditorDialog.cpp \
    WizOEMSettings.cpp  \
    mac/WizSearchWidget_mm.cpp \
    core/WizAccountManager.cpp \
    core/WizCommentManager.cpp \
    core/WizNoteManager.cpp \
    share/WizThreads.cpp \
    WizPositionDelegate.cpp \
    main.cpp \
    WizInitBizCertDialog.cpp \
    share/WizDocumentStyle.cpp \
    WizPlugins.cpp

HEADERS += \
    share/WizZip.h \
    share/WizQtHelper.h \
    share/WizMisc.h \
    share/sqlite3ext.h \
    share/sqlite3.h \
    share/cppsqlite3.h \
    share/WizObject.h \
    share/WizKMCore.h \
    share/WizDatabase.h \
    share/WizDatabaseManager.h \
    share/WizSettings.h \
    share/WizUI.h \
    share/WizHtml2Zip.h \
    share/WizMd5.h \
    share/WizWin32Helper.h \
    share/WizDrawTextHelper.h \
    share/WizUIHelper.h \
    share/WizUIBase.h \
    share/WizCommonUI.h \
    share/WizAnimateAction.h \
    share/WizPopupWidget.h \
    share/WizMultiLineListWidget.h \
    share/WizFileIconProvider.h \
    share/WizWindowsHelper.h \
    share/WizIndexBase.h \
    share/WizIndex.h \
    share/WizThumbIndex.h \
    share/WizEnc.h \
    share/WizZiwReader.h \
    share/WizGlobal.h \
    html/WizHtmlReader.h \
    html/WizHtmlCollector.h \
    sync/WizKMServer.h \
    sync/WizKMSync.h \
    sync/WizAsyncApi.h \
    sync/WizAvatarUploader.h \
    share/WizSyncableDatabase.h \
    share/WizSearch.h \
    utils/WizMisc_utils.h \
    utils/WizNotify.h \
    widgets/WizAboutDialog.h \
    widgets/WizSegmentedButton.h \
    widgets/WizUserInfoWidget.h \
    widgets/WizUserInfoWidgetBase.h \
    widgets/WizScrollBar.h \
    sync/WizToken.h \
    sync/WizToken_p.h \
    sync/WizApiEntry.h \
    share/WizObjectDataDownloader.h \
    sync/WizAvatarHost.h \
    sync/WizAvatarHost_p.h \
    utils/WizStyleHelper.h \
    utils/WizPathResolve.h \
    share/WizWindowTitleBar.h \
    share/WizShadowWindow.h \
    share/WizShadowEffect.h \
    widgets/WizTableSelector.h \
    share/jsoncpp/json/json.h \
    share/jsoncpp/json/json-forwards.h \
    share/WizRequest.h \
    WizDef.h \
    WizActions.h \
    WizNoteStyle.h \
    WizCategoryView.h \
    WizCategoryViewItem.h \
    WizDocumentListView.h \
    WizDocumentListViewItem.h \
    WizDocumentView.h \
    WizDocumentWebView.h \
    WizDocumentHistory.h \
    WizAttachmentListWidget.h \
    WizTagListWidget.h \
    WizConsoleDialog.h \
    WizPreferenceDialog.h \
    WizProxyDialog.h \
    WizCreateAccountDialog.h \
    WizMainWindow.h \
    WizCellButton.h \
    WizTitleBar.h \
    WizInfoBar.h \
    WizNotifyBar.h \
    WizTitleEdit.h \
    sync/WizSync.h \
    sync/WizKMSync_p.h \
    WizStatusBar.h \
    WizUpgradeNotifyDialog.h \
    WizUserCipherForm.h \
    WizNoteInfoForm.h \
    WizLineInputDialog.h \
    WizSearchWidget.h \
    WizEditorInsertLinkForm.h \
    WizUpgrade.h \
    WizEditorToolBar.h \
    WizWebSettingsDialog.h \
    WizPopupButton.h \
    WizDocumentTransitionView.h \
    WizButton.h \
    WizEditorInsertTableForm.h \
    WizFileImporter.h \
    WizFolderSelector.h \
    WizProgressDialog.h \
    WizFolderView.h \
    WizDocumentSelectionView.h \
    WizUserVerifyDialog.h \
    utils/WizLogger.h \
    utils/WizPinyin.h \
    WizMessageListView.h \
    WizThumbCache.h \
    WizThumbCache_p.h \
    WizMessageCompleter.h \
    widgets/WizImageButton.h \
    WizDocumentEditStatus.h \
    WizLoginDialog.h \
    share/WizFileMonitor.h \
    WizSearchReplaceWidget.h \
    widgets/WizCodeEditorDialog.h \
    WizDocTemplateDialog.h \
    WizMobileFileReceiver.h \
    widgets/WizFramelessWebDialog.h \
    share/WizLockedFile.h \
    share/WizLocalPeer.h \
    share/WizSingleApplication.h \
    widgets/WizScreenShotWidget.h \
    widgets/WizVerificationCodeDialog.h \
    widgets/WizEmailShareDialog.h \
    share/WebSocketClientWrapper.h \
    share/WebSocketTransport.h \
    share/WizWebEngineView.h \
    WizWebEngineInjectObject.h \
    widgets/WizShareLinkDialog.h \
    share/WizAnalyzer.h \
    share/WizTranslater.h \
    widgets/WizAdvancedSearchDialog.h \
    share/WizUdpClient.h \
    share/WizMessageBox.h \
    WizOEMSettings.h \
    widgets/WizTagBar.h \
    widgets/WizTrayIcon.h \
    share/WizEventLoop.h \
    sync/WizJSONServerBase.h \
    share/WizObjectOperator_p.h \
    share/WizObjectOperator.h \
    widgets/WizLocalProgressWebView.h \
    widgets/WizCrashReportDialog.h \
    widgets/WizCustomToolBar.h \
    widgets/WizSingleDocumentView.h \
    widgets/WizTemplatePurchaseDialog.h \
    widgets/WizExecutingActionDialog.h \
    widgets/WizUserServiceExprDialog.h \
    WizPositionDelegate.h \
    core/WizAccountManager.h \
    core/WizCommentManager.h \
    core/WizNoteManager.h \
    share/WizThreads.h \
    share/WizThreads_p.h \
    WizInitBizCertDialog.h \
    share/WizDocumentStyle.h \
    WizCombineNotesDialog.h \
    WizSvgEditorDialog.h \
    widgets/WizTipsWidget.h\
    WizPlugins.h

FORMS += \
    ui/WizLineInputDialog.ui \
    ui/WizConsoleDialog.ui \
    ui/WizPreferenceDialog.ui \
    ui/WizProxyDialog.ui \
    ui/WizCreateAccountDialog.ui \
    ui/WizUpgradeNotifyDialog.ui \
    ui/WizUserCipherForm.ui \
    ui/WizNoteInfoForm.ui \
    ui/WizEditorInsertLinkForm.ui \
    ui/WizEditorInsertTableForm.ui \
    ui/WizProgressDialog.ui \
    ui/WizLoginDialog.ui \
    ui/WizSearchReplaceWidget.ui \
    ui/WizDocTemplateDialog.ui \
    ui/WizVerificationCodeDialog.ui \
    ui/WizEmailShareDialog.ui \
    ui/WizAdvancedSearchDialog.ui \
    ui/WizCrashReportDialog.ui \
    ui/WizTemplatePurchaseDialog.ui \
    ui/WizExecutingActionDialog.ui \
    ui/WizInitBizCertDialog.ui \
    ui/WizUserServiceExprDialog.ui \
    ui/WizCombineNotesDialog.ui


macx {

SOURCES += \
    mac/WizMacHelper.mm \
    mac/WizMacToolBar.mm \
    mac/WizMacToolBarDelegate.mm \
    mac/WizMacActionHelper.cpp \
    mac/WizUserInfoWidgetBaseMac.mm \
    mac/WizUserInfoWidgetBaseMac_mm.cpp \
    mac/WizIAPHelper.mm \
    mac/rmstore/RMAppReceipt.mm \
    mac/rmstore/RMStoreAppReceiptVerificator.mm \
    mac/WizNotificationCenter.mm \
    widgets/WizIAPDialog.cpp \
    mac/DTWebArchive.m

HEADERS += \
    mac/WizMacHelper.h \
    mac/WizMacHelper_mm.h \
    mac/WizSearchWidget_mm.h \
    mac/WizMacToolBar.h \
    mac/WizMacToolBarDelegate.h \
    mac/WizMacActionHelper.h \
    mac/WizUserInfoWidgetBaseMac_mm.h \
    mac/WizIAPHelper.h \
    mac/rmstore/RMAppReceipt.h \
    mac/rmstore/RMStoreAppReceiptVerificator.h \
    mac/WizNotificationCenter.h \
    widgets/WizIAPDialog.h \
    mac/DTWebArchive.h

FORMS += \
    ui/WizIAPDialog.ui
}


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../lib/quazip/release/ -lquazip
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../lib/quazip/debug/ -lquazip
else:unix: LIBS += -L$$OUT_PWD/../lib/quazip/ -lquazip

INCLUDEPATH += $$PWD/../lib
DEPENDPATH += $$PWD/../lib/quazip

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lib/quazip/release/libquazip.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lib/quazip/debug/libquazip.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lib/quazip/release/quazip.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lib/quazip/debug/quazip.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../lib/quazip/libquazip.a

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../lib/zlib/release/ -lzlib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../lib/zlib/debug/ -lzlib
else:unix: LIBS += -L$$OUT_PWD/../lib/zlib/ -lzlib

INCLUDEPATH += $$PWD/../lib/zlib
DEPENDPATH += $$PWD/../lib/zlib

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lib/zlib/release/libzlib.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lib/zlib/debug/libzlib.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lib/zlib/release/zlib.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lib/zlib/debug/zlib.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../lib/zlib/libzlib.a

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../lib/cryptopp/release/ -lcryptopp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../lib/cryptopp/debug/ -lcryptopp
else:unix: LIBS += -L$$OUT_PWD/../lib/cryptopp/ -lcryptopp

INCLUDEPATH += $$PWD/../lib/cryptopp
DEPENDPATH += $$PWD/../lib/cryptopp

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lib/cryptopp/release/libcryptopp.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lib/cryptopp/debug/libcryptopp.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lib/cryptopp/release/cryptopp.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../lib/cryptopp/debug/cryptopp.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../lib/cryptopp/libcryptopp.a



# copy resources
# https://dragly.org/2013/11/05/copying-data-files-to-the-build-directory-when-working-with-qmake/
WIZNOTE_RESOURCESPATH = $$OUT_PWD/share
macx {
    WIZNOTE_RESOURCESPATH = $$OUT_PWD/$${TARGET}.app/Contents/Resources
}

message($$WIZNOTE_RESOURCESPATH)


copyResource.commands = $(COPY_DIR) $$PWD/../share/ $$WIZNOTE_RESOURCESPATH
first.depends = $(first) copyResource
export(first.depends)
export(copyResource.commands)

# copy mac translations
macx {
    copyServiceMenu.commands = $(COPY_DIR) $$PWD/../build/osx/localize/ $$WIZNOTE_RESOURCESPATH
    first.depends = $(first) copyResource copyServiceMenu
    export(first.depends)
    export(copyServiceMenu.commands)

    QMAKE_EXTRA_TARGETS += first copyServiceMenu copyResource
} else {
    QMAKE_EXTRA_TARGETS += first copyResource
}


