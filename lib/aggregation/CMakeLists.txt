project(aggregation)

set(aggr_HEADERS
    aggregate.h
    aggregation_global.h
)

set(aggr_SOURCES
    aggregate.cpp
)

if(WIZNOTE_USE_QT5)
    find_package(Qt5 REQUIRED Core)
else(WIZNOTE_USE_QT5)
    find_package(Qt4 REQUIRED QtCore)
    INCLUDE(${QT_USE_FILE})
endif(WIZNOTE_USE_QT5)

set(CMAKE_AUTOMOC ON)

add_library(aggregation SHARED ${aggr_SOURCES} ${aggr_HEADERS})
set_target_properties(aggregation PROPERTIES VERSION 1.0.0 SOVERSION 0)

if(WIZNOTE_USE_QT5)
    qt5_use_modules(aggregation Core)
else(WIZNOTE_USE_QT5)
    target_link_libraries(aggregation ${QT_LIBRARIES})
endif(WIZNOTE_USE_QT5)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

#set_property(TARGET aggregation PROPERTY PUBLIC_HEADER ${aggr_HEADERS})
#set_property(TARGET aggregation PROPERTY LIBRARY_OUTPUT_DIRECTORY aggregation

#install(TARGETS aggregation DESTINATION lib)
#install(FILES ${aggr_HEADERS} DESTINATION aggregation COMPONENT Headers)
