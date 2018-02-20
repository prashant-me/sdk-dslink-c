include_directories("${CMAKE_CURRENT_LIST_DIR}/include")

set(EXAMPLE_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src")
set(EXAMPLE_SRC
    "${EXAMPLE_SRC_DIR}/main.c"
    "${EXAMPLE_SRC_DIR}/sampleLink.c"
    "${EXAMPLE_SRC_DIR}/sampleQueue.c"
    "${CMAKE_CURRENT_LIST_DIR}/include/sampleLink.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/sampleQueue.h"
)

