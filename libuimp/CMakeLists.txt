cmake_minimum_required(VERSION 3.10)

project(UnityImporter)

# required dependencies
find_package(assimp REQUIRED)
find_package(glm REQUIRED)
find_package(Stb REQUIRED)
find_package(ryml REQUIRED)
find_package(tinyexr REQUIRED)
find_package(TIFF REQUIRED)
find_package(WebP REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(BUILD_TARGET "UnityImporter")
set(SRC_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE SRC_LIST LIST_DIRECTORIES false "${SRC_ROOT_DIR}/*.c" "${SRC_ROOT_DIR}/*.cpp" "${SRC_ROOT_DIR}/*.h")
	
foreach(SRC IN ITEMS ${SRC_LIST})
	get_filename_component(SRC_PATH "${SRC}" PATH)
	file(RELATIVE_PATH SRC_PATH_REL "${SRC_ROOT_DIR}" "${SRC_PATH}")
	string(REPLACE "/" "\\" GROUP_PATH "${SRC_PATH_REL}")
	source_group("${GROUP_PATH}" FILES ${SRC})
endforeach()

add_library(libuimp STATIC ${SRC_LIST})
target_compile_definitions(libuimp PUBLIC GLM_ENABLE_EXPERIMENTAL)
target_compile_definitions(libuimp PUBLIC NOMINMAX)
if (WIN32)
    set_target_properties(libuimp PROPERTIES OUTPUT_NAME "libuimp")
endif()
target_link_libraries(libuimp PUBLIC assimp::assimp glm::glm ryml::ryml TIFF::TIFF unofficial::tinyexr::tinyexr WebP::webp WebP::webpdecoder WebP::webpdemux)
