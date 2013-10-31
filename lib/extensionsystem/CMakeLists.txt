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

if(WIZNOTE_USE_QT5)
    find_package(Qt5 REQUIRED Core Widgets LinguistTools)
    qt5_wrap_ui(extsys_FORM_HEADERS ${extsys_FORMS})
    qt5_add_resources(extsys_RC ${extsys_RESOURCES})
else(WIZNOTE_USE_QT5)
    find_package(Qt4 REQUIRED QtCore QtGui)
    qt4_wrap_ui(extsys_FORM_HEADERS ${extsys_FORMS})
    qt4_add_resources(extsys_RC ${extsys_RESOURCES})
    INCLUDE(${QT_USE_FILE})
endif(WIZNOTE_USE_QT5)

set(CMAKE_AUTOMOC ON)

add_library(extensionsystem SHARED ${extsys_SOURCES} ${extsys_HEADERS} ${extsys_FORM_HEADERS} ${extsys_RC})
set_target_properties(extensionsystem PROPERTIES VERSION 1.0.0 SOVERSION 0)

if(WIZNOTE_USE_QT5)
    qt5_use_modules(extensionsystem Core Widgets)
else(WIZNOTE_USE_QT5)
    target_link_libraries(extensionsystem ${QT_LIBRARIES})
endif(WIZNOTE_USE_QT5)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${aggregation_SOURCE_DIR}/..
)

target_link_libraries(extensionsystem aggregation)

if(UNIX AND NOT APPLE)
    target_link_libraries(extensionsystem ${CMAKE_DL_LIBS})
endif(UNIX AND NOT APPLE)
