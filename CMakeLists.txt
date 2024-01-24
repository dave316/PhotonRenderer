project(PhotonRenderer)

cmake_minimum_required(VERSION 3.0)

# sample applications
option(BUILD_SIMPLEVIEWER "Simple OLDRenderer with FPS controls" ON)
option(BUILD_GLTFVIEWER "GLTF Model viewer" OFF)
option(BUILD_EDITOR "Simple Scene Editor" OFF)
option(BUILD_DOFEDITOR "DoF Editor" OFF)

# optional libraries
option(WITH_ASSIMP_IMPORTER "Support assimp imported models" OFF)
option(WITH_FREETYPE_FONTS "Support true type font rendering" OFF)
option(WITH_DRACO_COMPRESSION "Support for draco compressed GLTF meshes" OFF)
option(WITH_KTX_TEXTURES "Support for KTX texture file format" OFF)
option(WITH_IMGUI_SUPPORT "Support for imgui" OFF)

add_subdirectory("src")

if (BUILD_SIMPLEVIEWER)
	add_subdirectory("samples/SimpleViewer")
endif()

if (BUILD_GLTFVIEWER)
	add_subdirectory("samples/GLTFViewer")
endif()

if (BUILD_EDITOR)
	add_subdirectory("samples/Editor")
endif()

if (BUILD_DOFEDITOR)
	add_subdirectory("samples/DoFViewer")
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT SimpleViewer)
