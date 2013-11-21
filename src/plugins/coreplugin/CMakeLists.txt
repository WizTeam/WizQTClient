set(coreplugin_HEADERS
    core_global.h
    coreplugin.h
    coreconstants.h
    wizdef.h
    wizactions.h
    wiznotestyle.h
    wizCategoryView.h
    wizCategoryViewItem.h
    wizDocumentListView.h
    wizDocumentListViewItem.h
    wizDocumentView.h
    wizDocumentWebView.h
    #wizdocumenthistory.h
    wizattachmentlistwidget.h
    wiztaglistwidget.h
    wizconsoledialog.h
    wizpreferencedialog.h
    wizAboutDialog.h
    wizLoginDialog.h
    #wizproxydialog.h
    wizcreateaccountdialog.h
    wizmainwindow.h
    #wizupdaterprogressdialog.h
    wizstatusbar.h
    wizupgradenotifydialog.h
    wizcertmanager.h
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
)

set(coreplugin_SOURCES
    coreplugin.cpp
    wizCategoryView.cpp
    wizCategoryViewItem.cpp
    wizDocumentListView.cpp
    wizDocumentListViewItem.cpp
    wizDocumentView.cpp
    wizDocumentWebView.cpp
    wizactions.cpp
    wiznotestyle.cpp
    wizLineInputDialog.cpp
    #wizdocumenthistory.cpp
    wizattachmentlistwidget.cpp
    wiztaglistwidget.cpp
    wizconsoledialog.cpp
    wizpreferencedialog.cpp
    wizAboutDialog.cpp
    wizLoginDialog.cpp
    #wizproxydialog.cpp
    wizcreateaccountdialog.cpp
    wizmainwindow.cpp
    #wizupdaterprogressdialog.cpp
    wizstatusbar.cpp
    wizupgradenotifydialog.cpp
    wizcertmanager.cpp
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
)

set(coreplugin_FORMS
    ui/wizLineInputDialog.ui
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

#include(QtChooser)
#qt_add_ui(coreplugin_FORM_HEADERS ${coreplugin_FORMS})
#add_library(coreplugin SHARED ${coreplugin_SOURCES} ${coreplugin_HEADERS} ${coreplugin_FORM_HEADERS})
#qt_use_modules(coreplugin)
#
#add_definitions(-DCORE_LIBRARY)

#include_directories(
#    #${CMAKE_CURRENT_SOURCE_DIR}
#    #${CMAKE_CURRENT_BINARY_DIR}
#    ${extensionsystem_SOURCE_DIR}/..
#)
#
#target_link_libraries(coreplugin ${extensionsystem})
#
#
#set_target_properties(coreplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/share/wiznote/plugins)
#
#install(FILES coreplugin.pluginspec DESTINATION share/wiznote/plugins)
#if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#    add_custom_command(TARGET helloworld POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/coreplugin.pluginspec ${CMAKE_BINARY_DIR}/share/wiznote/plugins)
#endif()
