project(coreplugin_plugin)

set(coreplugin_HEADERS
    core_global.h
    coreplugin.h
    coreconstants.h
    icore.h
    inoteview.h
    itreeview.h
    aboutdialog.h
)

set(coreplugin_SOURCES
    coreplugin.cpp
    icore.cpp
    inoteview.cpp
    itreeview.cpp
    aboutdialog.cpp
)

set(coreplugin_FORMS
)

set(coreplugin_SPEC
    Core.pluginspec
)

include(QtChooser)
qt_add_ui(coreplugin_FORM_HEADERS ${coreplugin_FORMS})
add_library(coreplugin SHARED ${coreplugin_SOURCES} ${coreplugin_HEADERS} ${coreplugin_FORM_HEADERS} ${coreplugin_SPEC})
qt_use_modules(coreplugin)
qt_suppress_warnings(coreplugin)

set_target_properties(coreplugin PROPERTIES LIBRARY_OUTPUT_NAME "Core")
add_definitions(-DCORE_LIBRARY)

include_directories(
    ${libs_SOURCE_DIR}
    ${wiznote_SOURCE_DIR} #FIXME
)
target_link_libraries(coreplugin extensionsystem)

add_custom_command(TARGET coreplugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${WIZNOTE_PLUGINSPEC_DIR})
add_custom_command(TARGET coreplugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/Core.pluginspec ${WIZNOTE_PLUGINSPEC_DIR})

install(FILES ${coreplugin_SPEC} DESTINATION lib/wiznote/plugins)
install(TARGETS coreplugin DESTINATION lib/wiznote/plugins)
