cmake_minimum_required(VERSION 3.10)

project(PhotonRenderer)

# sample applications
option(BUILD_SIMPLEVIEWER "Simple Renderer with FPS controls" ON)
option(BUILD_GLTFVIEWER "GLTF Model viewer" OFF)
option(BUILD_EDITOR "Simple Scene Editor" OFF)

# graphic APIs
option(BUILD_BACKEND_VULKAN "Build Vulkan backend" OFF)
option(BUILD_BACKEND_D3D11 "Build Direct3D 11 backend" OFF)

# optional libraries
option(WITH_ASSIMP_IMPORTER "Support assimp imported models" OFF)
option(WITH_FREETYPE_FONTS "Support true type font rendering" OFF)
option(WITH_DRACO_COMPRESSION "Support for draco compressed GLTF meshes" OFF)
option(WITH_KTX_TEXTURES "Support for KTX texture file format" OFF)
option(WITH_IMGUI_SUPPORT "Support for imgui" OFF)
option(WITH_UNITY_SUPPORT "Support for importing Unity scenes" OFF)

add_subdirectory("src")
#add_subdirectory("libuimp")

if (BUILD_SIMPLEVIEWER)
	add_subdirectory("apps/SimpleViewer")
endif()

if (BUILD_GLTFVIEWER)
	add_subdirectory("apps/GLTFViewer")
endif()

if (BUILD_EDITOR)
	add_subdirectory("apps/Editor")
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT SimpleViewer)
