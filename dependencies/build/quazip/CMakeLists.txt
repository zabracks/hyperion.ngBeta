# set all include directories for in and out of source builds
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${ZLIB_INCLUDE_DIRS}
)

file(GLOB SRCS "*.c" "*.cpp")
file(GLOB PUBLIC_HEADERS "*.h")

# Must be added to enable export macro
ADD_DEFINITIONS(-DQUAZIP_BUILD)

set(SRCS ${SRCS} ${PUBLIC_HEADERS})

#add_library(${QUAZIP_LIB_TARGET_NAME} SHARED ${SRCS})
add_library(quazip_static STATIC ${SRCS})

# Windows uses .lib extension for both static and shared library
# *nix systems use different extensions for SHARED and STATIC library and by convention both libraries have the same name
if (NOT WIN32)
	set_target_properties(quazip_static PROPERTIES OUTPUT_NAME quazip${QUAZIP_LIB_VERSION_SUFFIX})
endif ()

#set_target_properties(${QUAZIP_LIB_TARGET_NAME} quazip_static PROPERTIES VERSION 1.0.0 SOVERSION 1 DEBUG_POSTFIX d)
# Link against ZLIB_LIBRARIES if needed (on Windows this variable is empty) add -lz to prevent link errors to zlib.so
target_link_libraries(quazip_static Qt5::Core ${ZLIB_LIBRARIES} -lz)

#install(FILES ${PUBLIC_HEADERS} DESTINATION include/quazip${QUAZIP_LIB_VERSION_SUFFIX})
#install(TARGETS ${QUAZIP_LIB_TARGET_NAME} quazip_static LIBRARY DESTINATION ${LIB_DESTINATION} ARCHIVE DESTINATION ${LIB_DESTINATION} RUNTIME DESTINATION ${LIB_DESTINATION})
