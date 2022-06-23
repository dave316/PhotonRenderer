project(GLTFViewer)

cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 17)
set(BUILD_TARGET "GLTFViewer")
set(SRC_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB_RECURSE SRC_LIST LIST_DIRECTORIES false "${SRC_ROOT_DIR}/*.cpp" "${SRC_ROOT_DIR}/*.h")
foreach(SRC IN ITEMS ${SRC_LIST})
	get_filename_component(SRC_PATH "${SRC}" PATH)
	file(RELATIVE_PATH SRC_PATH_REL "${SRC_ROOT_DIR}" "${SRC_PATH}")
	string(REPLACE "/" "\\" GROUP_PATH "${SRC_PATH_REL}")
	source_group("${GROUP_PATH}" FILES ${SRC})
endforeach()

include_directories("../../src")
include_directories("../../src/3rdParty")

add_executable(GLTFViewer ${SRC_LIST})
target_link_libraries(GLTFViewer PRIVATE libphoton)

if(WIN32)
	set_target_properties(GLTFViewer PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
endif()