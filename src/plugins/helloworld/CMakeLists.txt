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
    helloworld.pluginspec
)

include(QtChooser)
add_library(helloworld SHARED ${helloworld_SOURCES} ${helloworld_HEADERS} ${helloworld_SPEC})
qt_use_modules(helloworld)

# set all include directories for in and out of source builds
include_directories(
    ${libs_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(helloworld extensionsystem)

set_target_properties(helloworld PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${WIZNOTE_PLUGIN_DIR})
if(APPLE)
    set_source_files_properties(${helloworld_SPEC} PROPERTIES MACOSX_PACKAGE_LOCATION .)
else(APPLE)
    install(FILES ${helloworld_SPEC} DESTINATION .)
endif(APPLE)
