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
option(LIBS_ASSIMP_IMPORTER "Support assimp imported models" OFF)
option(LIBS_UNITY_IMPORTER "Support for importing Unity scenes" OFF)
option(LIBS_FREETYPE_FONTS "Support true type font rendering" OFF)
option(LIBS_DRACO_COMPRESSION "Support for draco compressed GLTF meshes" OFF)
option(LIBS_IMGUI_SUPPORT "Support for imgui" OFF)

# optional image formats
option(IMAGE_TIFF "Support for TIFF file format" OFF)
option(IMAGE_EXR "Support for EXR file format" OFF)
option(IMAGE_WEBP "Support for WebP file format" OFF)
option(IMAGE_KTX "Support for KTX file format" OFF)

add_subdirectory("src")
#add_subdirectory("libuimp")

if (APPS_SIMPLEVIEWER)
	add_subdirectory("apps/SimpleViewer")
endif()

if (APPS_GLTFVIEWER)
	set(LIBS_IMGUI_SUPPORT ON CACHE BOOL "Support for imgui" FORCE)
	add_subdirectory("apps/GLTFViewer")
endif()

if (APPS_EDITOR)
	set(LIBS_IMGUI_SUPPORT ON CACHE BOOL "Support for imgui" FORCE)
	add_subdirectory("apps/Editor")
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT SimpleViewer)
