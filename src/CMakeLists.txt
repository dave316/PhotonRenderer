project(PhotonRenderer)

cmake_minimum_required(VERSION 3.0)

find_package(assimp REQUIRED)
find_package(freetype CONFIG REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glew REQUIRED)
find_package(glm REQUIRED)
find_package(imgui REQUIRED)
find_package(opengl REQUIRED)
find_package(rapidjson REQUIRED)
find_package(stb REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(BUILD_TARGET "PhotonRenderer")
set(SRC_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories("3rdParty")

file(GLOB_RECURSE SRC_LIST LIST_DIRECTORIES false "${SRC_ROOT_DIR}/*.cpp" "${SRC_ROOT_DIR}/*.h")
file(GLOB SHADER_LIST 
	"${SRC_ROOT_DIR}/Shaders/*.glsl" 
	"${SRC_ROOT_DIR}/Shaders/*.vert" 
	"${SRC_ROOT_DIR}/Shaders/*.frag" 
	"${SRC_ROOT_DIR}/Shaders/*.geom"
	)

foreach(SRC IN ITEMS ${SRC_LIST})
	get_filename_component(SRC_PATH "${SRC}" PATH)
	file(RELATIVE_PATH SRC_PATH_REL "${SRC_ROOT_DIR}" "${SRC_PATH}")
	string(REPLACE "/" "\\" GROUP_PATH "${SRC_PATH_REL}")
	source_group("${GROUP_PATH}" FILES ${SRC})
endforeach()
source_group("Shaders" FILES ${SHADER_LIST})

add_library(libphoton STATIC ${SRC_LIST} ${SHADER_LIST})
if (WIN32)
    set_target_properties(libphoton PROPERTIES OUTPUT_NAME "libphoton")
endif()
target_link_libraries(libphoton PRIVATE glfw freetype GLEW::GLEW assimp::assimp imgui::imgui ${OPENGL_gl_LIBRARY})

#add_executable(PhotonRenderer ${SRC_LIST} ${SHADER_LIST})
#target_link_libraries(PhotonRenderer PRIVATE glfw freetype GLEW::GLEW assimp::assimp imgui::imgui ${OPENGL_gl_LIBRARY})

#if(WIN32)
	#set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT PhotonRenderer)
#endif()
