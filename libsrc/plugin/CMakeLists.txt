find_package(PythonLibs 3.4 REQUIRED)

# Define the current source locations
set(QUAZIP_HEADER_DIR ${CMAKE_SOURCE_DIR}/dependencies/build/quazip)
set(CURRENT_HEADER_DIR ${CMAKE_SOURCE_DIR}/include/plugin)
set(CURRENT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/libsrc/plugin)

FILE ( GLOB Plugin_SOURCES "${CURRENT_HEADER_DIR}/*.h"  "${CURRENT_SOURCE_DIR}/*.h"  "${CURRENT_SOURCE_DIR}/*.cpp")

include_directories(${PYTHON_INCLUDE_DIRS})
include_directories(${QUAZIP_HEADER_DIR})

add_library(plugins
	${Plugin_SOURCES}
)

target_link_libraries(plugins
	hyperion
	hyperion-utils
	database
	python
	Qt5::Core
	quazip_static
	${PYTHON_LIBRARIES}
)
