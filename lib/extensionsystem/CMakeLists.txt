project(extensionsystem)

set(extsys_HEADERS
    pluginerrorview.h
    plugindetailsview.h
    invoker.h
    iplugin.h
    iplugin_p.h
    extensionsystem_global.h
    pluginmanager.h
    pluginmanager_p.h
    pluginspec.h
    pluginspec_p.h
    pluginview.h
    optionsparser.h
    plugincollection.h
    pluginerroroverview.h
)

set(extsys_SOURCES
    pluginerrorview.cpp
    plugindetailsview.cpp
    invoker.cpp
    iplugin.cpp
    pluginmanager.cpp
    pluginspec.cpp
    pluginview.cpp
    optionsparser.cpp
    plugincollection.cpp
    pluginerroroverview.cpp
)

set(extsys_FORMS
    pluginview.ui
    pluginerrorview.ui
    plugindetailsview.ui
    pluginerroroverview.ui
)

set(extsys_RESOURCES
    pluginview.qrc
)

include(QtChooser)
qt_add_ui(extsys_FORM_HEADERS ${extsys_FORMS})
qt_add_resources(extsys_RC ${extsys_RESOURCES})
message(STATUS ${extsys_RC})
add_library(extensionsystem SHARED ${extsys_SOURCES} ${extsys_HEADERS} ${extsys_FORM_HEADERS} ${extsys_RC})
qt_use_modules(extensionsystem)
qt_suppress_warnings(extensionsystem)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${libs_SOURCE_DIR}
)

target_link_libraries(extensionsystem aggregation)

if(UNIX AND NOT APPLE)
    target_link_libraries(extensionsystem ${CMAKE_DL_LIBS})
endif(UNIX AND NOT APPLE)

install(TARGETS extensionsystem DESTINATION lib/wiznote/plugins)
