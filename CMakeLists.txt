cmake_minimum_required(VERSION 3.10)

project(PhotonRenderer)

# sample applications
option(APPS_SIMPLEVIEWER "Simple Renderer with FPS controls" ON)
option(APPS_GLTFVIEWER "GLTF Model viewer" OFF)
option(APPS_EDITOR "Simple Scene Editor" OFF)

# graphic APIs
option(BACKEND_OPENGL "Build OpenGL backend" ON)
option(BACKEND_VULKAN "Build Vulkan backend" OFF)
option(BACKEND_DX11 "Build DirectX 11 backend" OFF)

# optional libraries
option(WITH_ASSIMP_IMPORTER "Support assimp imported models" OFF)
option(WITH_FREETYPE_FONTS "Support true type font rendering" OFF)
option(WITH_DRACO_COMPRESSION "Support for draco compressed GLTF meshes" OFF)
option(WITH_KTX_TEXTURES "Support for KTX texture file format" OFF)
option(WITH_IMGUI_SUPPORT "Support for imgui" OFF)
option(WITH_UNITY_SUPPORT "Support for importing Unity scenes" OFF)

add_subdirectory("src")
#add_subdirectory("libuimp")

if (APPS_SIMPLEVIEWER)
	add_subdirectory("apps/SimpleViewer")
endif()

if (APPS_GLTFVIEWER)
	add_subdirectory("apps/GLTFViewer")
endif()

if (APPS_EDITOR)
	add_subdirectory("apps/Editor")
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT SimpleViewer)
