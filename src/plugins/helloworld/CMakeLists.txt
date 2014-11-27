project(helloworld_plugin)

set(helloworld_HEADERS
    helloworldplugin.h
    helloworldwindow.h
)

set(helloworld_SOURCES
    helloworldplugin.cpp
    helloworldwindow.cpp
)

set(helloworld_SPEC
    HelloWorld.pluginspec
)

include(QtChooser)
add_library(helloworld SHARED ${helloworld_SOURCES} ${helloworld_HEADERS} ${helloworld_SPEC})
qt_use_modules(helloworld)
qt_suppress_warnings(helloworld)

set_target_properties(helloworld PROPERTIES LIBRARY_OUTPUT_NAME "HelloWorld")

# set all include directories for in and out of source builds
include_directories(
    ${libs_SOURCE_DIR}
    ${plugins_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(helloworld extensionsystem coreplugin)

add_custom_command(TARGET helloworld POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorld.pluginspec ${WIZNOTE_PLUGINSPEC_DIR})

install(FILES ${helloworld_SPEC} DESTINATION lib/wiznote/plugins)
install(TARGETS helloworld DESTINATION lib/wiznote/plugins)
