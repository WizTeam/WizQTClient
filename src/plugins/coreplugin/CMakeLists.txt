project(coreplugin_plugin)

set(coreplugin_HEADERS
    core_global.h
    coreplugin.h
    coreconstants.h
)

set(coreplugin_SOURCES
    coreplugin.cpp
)

set(coreplugin_FORMS
)

set(coreplugin_SPEC
    coreplugin.pluginspec
)

include(QtChooser)
qt_add_ui(coreplugin_FORM_HEADERS ${coreplugin_FORMS})
add_library(coreplugin SHARED ${coreplugin_SOURCES} ${coreplugin_HEADERS} ${coreplugin_FORM_HEADERS} ${coreplugin_SPEC})
qt_use_modules(coreplugin)
qt_suppress_warnings(coreplugin)

add_definitions(-DCORE_LIBRARY)

include_directories(
    ${libs_SOURCE_DIR}
)
target_link_libraries(coreplugin extensionsystem)

add_custom_command(TARGET coreplugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/coreplugin.pluginspec ${WIZNOTE_PLUGIN_DIR})

install(FILES ${coreplugin_SPEC} DESTINATION share/wiznote/plugins)
install(TARGETS coreplugin DESTINATION share/wiznote/plugins)
