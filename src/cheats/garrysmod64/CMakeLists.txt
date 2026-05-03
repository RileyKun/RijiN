cmake_minimum_required(VERSION 3.21.3)
project(garrysmod64 VERSION 1.0.0 LANGUAGES CXX)

# set our custom flags
set(BUILD_FLAG_CPU_MODE_X86 OFF)
set(BUILD_FLAG_EXE          OFF)
set(BUILD_FLAG_NO_CRT       ON)
set(BUILD_FLAG_PRESET_MODE  GEN_SECURE_DLL_NOCRT)
set(BUILD_FLAG_RENDER_MODE  D3D9)
set(BUILD_FLAG_GCS_GUI      ON)
set(BUILD_FLAG_COPY_TO_SVR  ON)
set(BUILD_FLAG_PROTECT      ON)
set(BUILD_FLAG_PROTECT_VM   ON)
set(BUILD_FLAG_HDE_DISASM   ON)

# include build commander
include(${CMAKE_CURRENT_SOURCE_DIR}/../../build_commander.cmake)

add_macro("GMOD")
add_macro("GEN_INTERNAL")
add_macro("CLOUD_CONFIG")

# Version 42 Requirement Specific for TF2
#add_cxx_lib("-ld3dx9_42")
add_cxx_lib("-lgdi32")

# files
add_cxx_files("*.cpp")
add_cxx_files("./structs/*.cpp")
add_cxx_files("./features/*.cpp")
add_cxx_files("./interfaces/*.cpp")

set_cxx_output_dir("${BUILD_PATH_CODEBASE_MAIN}/build/x64")
cxx_compile()