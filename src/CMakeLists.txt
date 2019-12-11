project(PhotonRenderer)

cmake_minimum_required(VERSION 3.0)

find_package(assimp REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glew REQUIRED)
find_package(opengl REQUIRED)
find_package(rapidjson REQUIRED)

set(BUILD_TARGET "PhotonRenderer")
set(SRC_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB_RECURSE SRC_LIST LIST_DIRECTORIES false "${SRC_ROOT_DIR}/*.cpp" "${SRC_ROOT_DIR}/*.h")
file(GLOB SHADER_LIST "${SRC_ROOT_DIR}/Shaders/*.glsl" "${SRC_ROOT_DIR}/Shaders/*.vert" "${SRC_ROOT_DIR}/Shaders/*.frag" "${SRC_ROOT_DIR}/Shaders/*.geom")

foreach(SRC IN ITEMS ${SRC_LIST})
	get_filename_component(SRC_PATH "${SRC}" PATH)
	file(RELATIVE_PATH SRC_PATH_REL "${SRC_ROOT_DIR}" "${SRC_PATH}")
	string(REPLACE "/" "\\" GROUP_PATH "${SRC_PATH_REL}")
	source_group("${GROUP_PATH}" FILES ${SRC})
endforeach()
source_group("Shaders" FILES ${SHADER_LIST})

add_executable(PhotonRenderer ${SRC_LIST} ${SHADER_LIST})
target_link_libraries(PhotonRenderer PRIVATE glfw GLEW::GLEW assimp::assimp ${OPENGL_gl_LIBRARY})

if(WIN32)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT PhotonRenderer)
endif()
