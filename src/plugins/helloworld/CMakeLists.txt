project(plugin_helloworld)

set(plugin_helloworld_HEADERS
    helloworldplugin.h
    helloworldwindow.h
)

set(plugin_helloworld_SOURCES
    helloworldplugin.cpp
    helloworldwindow.cpp
)

if(WIZNOTE_USE_QT5)
    find_package(Qt5 REQUIRED Gui)
else(WIZNOTE_USE_QT5)
    find_package(Qt4 REQUIRED)
    INCLUDE(${QT_USE_FILE})
endif(WIZNOTE_USE_QT5)

set(CMAKE_AUTOMOC ON)

add_library(helloworld SHARED ${plugin_helloworld_SOURCES} ${plugin_helloworld_HEADERS})

install(FILES helloworld.pluginspec DESTINATION share/wiznote/plugins)

set_target_properties(helloworld PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/share/wiznote/plugins)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            add_custom_command(TARGET helloworld POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/helloworld.pluginspec ${CMAKE_BINARY_DIR}/share/wiznote/plugins)
endif()

if(WIZNOTE_USE_QT5)
    qt5_use_modules(helloworld Core Gui Widgets)
else(WIZNOTE_USE_QT5)
    target_link_libraries(helloworld ${QT_LIBRARIES})
endif(WIZNOTE_USE_QT5)

# set all include directories for in and out of source builds
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${extensionsystem_SOURCE_DIR}/..
)

target_link_libraries(helloworld ${extensionsystem})
