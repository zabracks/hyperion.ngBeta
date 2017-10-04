find_package(PythonLibs 2.7 REQUIRED)

# Define the current source locations
set(CURRENT_HEADER_DIR ${CMAKE_SOURCE_DIR}/include/plugin)
set(CURRENT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/libsrc/plugin)

FILE ( GLOB Plugin_SOURCES "${CURRENT_HEADER_DIR}/*.h"  "${CURRENT_SOURCE_DIR}/*.h"  "${CURRENT_SOURCE_DIR}/*.cpp" )
include_directories(${PYTHON_INCLUDE_DIRS})

add_library(plugin
	${Plugin_SOURCES}
)

target_link_libraries(plugin
	hyperion
	hyperion-utils
	Qt5::Core
	${PYTHON_LIBRARIES}
)
