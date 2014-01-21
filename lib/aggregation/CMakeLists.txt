project(aggregation)

set(aggr_HEADERS
    aggregate.h
    aggregation_global.h
)

set(aggr_SOURCES
    aggregate.cpp
)

include(QtChooser)
add_library(aggregation SHARED ${aggr_SOURCES} ${aggr_HEADERS})
qt_use_modules(aggregation Core)
qt_suppress_warnings(aggregation)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

install(TARGETS aggregation DESTINATION lib/wiznote/plugins)
