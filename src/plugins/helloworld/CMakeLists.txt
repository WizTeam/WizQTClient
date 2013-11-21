set(helloworld_HEADERS
    helloworldplugin.h
    helloworldwindow.h
)

set(helloworld_SOURCES
    helloworldplugin.cpp
    helloworldwindow.cpp
)

include(QtChooser)
add_library(helloworld SHARED ${helloworld_SOURCES} ${helloworld_HEADERS})
qt_use_modules(helloworld)

# set all include directories for in and out of source builds
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${extensionsystem_SOURCE_DIR}/..
)

target_link_libraries(helloworld extensionsystem)

install(FILES helloworld.pluginspec DESTINATION share/wiznote/plugins)

set_target_properties(helloworld PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/share/wiznote/plugins)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            add_custom_command(TARGET helloworld POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/helloworld.pluginspec ${CMAKE_BINARY_DIR}/share/wiznote/plugins)
endif()
