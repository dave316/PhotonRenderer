project(PhotonRenderer)

cmake_minimum_required(VERSION 3.0)

# required dependencies
find_package(OpenGL REQUIRED)
find_package(glew REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(rapidjson REQUIRED)
find_package(ryml REQUIRED)
find_package(Stb REQUIRED)
find_package(tinyexr REQUIRED)
find_package(tiff REQUIRED)

# optional dependencies
if (WITH_ASSIMP_IMPORTER)
	find_package(assimp REQUIRED)
endif()
if (WITH_FREETYPE_FONTS)
	find_package(freetype CONFIG REQUIRED)
endif()
if (WITH_DRACO_COMPRESSION)
	find_package(draco REQUIRED)
endif()
if (WITH_IMGUI_SUPPORT)
	find_package(imgui REQUIRED)
	find_package(unofficial-imguizmo CONFIG REQUIRED)
endif()
if (WITH_KTX_TEXTURES)
	find_package(Ktx CONFIG REQUIRED)
	find_package(Vulkan REQUIRED)
endif()

set(CMAKE_CXX_STANDARD 17)
set(BUILD_TARGET "PhotonRenderer")
set(SRC_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SRC_3RDPARTY_DIR "3rdParty")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${SRC_3RDPARTY_DIR})

file(GLOB_RECURSE SRC_LIST LIST_DIRECTORIES false "${SRC_ROOT_DIR}/*.cpp" "${SRC_ROOT_DIR}/*.h")
file(GLOB_RECURSE SHADER_LIST LIST_DIRECTORIES false
	"${SRC_ROOT_DIR}/Shaders/*.glsl" 
	"${SRC_ROOT_DIR}/Shaders/*.vert" 
	"${SRC_ROOT_DIR}/Shaders/*.frag" 
	"${SRC_ROOT_DIR}/Shaders/*.geom"
	"${SRC_ROOT_DIR}/Shaders/*.comp"
	)

foreach(SRC IN ITEMS ${SRC_LIST})
	get_filename_component(SRC_PATH "${SRC}" PATH)
	file(RELATIVE_PATH SRC_PATH_REL "${SRC_ROOT_DIR}" "${SRC_PATH}")
	string(REPLACE "/" "\\" GROUP_PATH "${SRC_PATH_REL}")
	source_group("${GROUP_PATH}" FILES ${SRC})
endforeach()

foreach(SRC IN ITEMS ${SHADER_LIST})
	get_filename_component(SRC_PATH "${SRC}" PATH)
	file(RELATIVE_PATH SRC_PATH_REL "${SRC_ROOT_DIR}" "${SRC_PATH}")
	string(REPLACE "/" "\\" GROUP_PATH "${SRC_PATH_REL}")
	source_group("${GROUP_PATH}" FILES ${SRC})
endforeach()
#source_group("Shaders" FILES ${SHADER_LIST})

add_library(libphoton STATIC ${SRC_LIST} ${SHADER_LIST}) # TODO: better handling of 3rdParty libs
target_compile_definitions(libphoton PUBLIC GLM_ENABLE_EXPERIMENTAL)
if (WIN32)
    set_target_properties(libphoton PROPERTIES OUTPUT_NAME "libphoton")
endif()
target_link_libraries(libphoton PRIVATE glfw GLEW::GLEW rapidjson ryml::ryml unofficial::tinyexr::tinyexr TIFF::TIFF ${OPENGL_gl_LIBRARY})

if (WITH_ASSIMP_IMPORTER)
	target_link_libraries(libphoton PRIVATE assimp::assimp)
	target_compile_definitions(libphoton PUBLIC WITH_ASSIMP)
endif()

if (WITH_FREETYPE_FONTS)
	target_link_libraries(libphoton PRIVATE freetype)
	target_compile_definitions(libphoton PUBLIC WITH_FREETYPE)
endif()

if (WITH_DRACO_COMPRESSION)
	target_link_libraries(libphoton PRIVATE draco)
	target_compile_definitions(libphoton PUBLIC WITH_DRACO)
endif()

if (WITH_IMGUI_SUPPORT)
	target_link_libraries(libphoton PRIVATE imgui::imgui unofficial::imguizmo::imguizmo)
	target_compile_definitions(libphoton PUBLIC WITH_IMGUI)
	target_compile_definitions(libphoton PUBLIC IMGUI_DEFINE_MATH_OPERATORS)
endif()

if (WITH_KTX_TEXTURES)
	target_include_directories(libphoton PUBLIC ${Vulkan_INCLUDE_DIR})
	target_link_libraries(libphoton PRIVATE KTX::ktx ${Vulkan_LIBRARY})
	target_compile_definitions(libphoton PUBLIC WITH_KTX)
endif()



