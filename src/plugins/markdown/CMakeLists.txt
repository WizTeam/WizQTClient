project(markdown_plugin)

set(markdown_HEADERS
    markdown.h
)

set(markdown_SOURCES
    markdown.cpp
)

set(markdown_FORMS
)

set(markdown_SPEC
    MarkDown.pluginspec
)

include(QtChooser)
qt_add_ui(markdown_FORM_HEADERS ${coreplugin_FORMS})
add_library(markdown SHARED ${markdown_SOURCES} ${markdown_HEADERS} ${markdown_FORM_HEADERS} ${markdown_RC} ${markdown_SPEC})
qt_use_modules(markdown)
qt_suppress_warnings(markdown)

set_target_properties(markdown PROPERTIES LIBRARY_OUTPUT_NAME "MarkDown")
add_definitions(-DMARKDOWN_LIBRARY)

include_directories(
    ${libs_SOURCE_DIR}
    ${plugins_SOURCE_DIR}
)
target_link_libraries(markdown extensionsystem coreplugin)

add_custom_command(TARGET markdown POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/MarkDown.pluginspec ${WIZNOTE_PLUGINSPEC_DIR})

install(FILES ${markdown_SPEC} DESTINATION lib/wiznote/plugins)
install(TARGETS markdown DESTINATION lib/wiznote/plugins)
