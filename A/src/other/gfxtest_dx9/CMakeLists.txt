cmake_minimum_required(VERSION 3.21.3)
project(gfxtest_dx9 VERSION 1.0.0 LANGUAGES CXX)

# set our custom flags
set(BUILD_FLAG_CPU_MODE_X86 ON)
set(BUILD_FLAG_EXE          OFF)
set(BUILD_FLAG_NO_CRT       ON)
set(BUILD_FLAG_PRESET_MODE  GEN_SECURE_DLL_NOCRT)
set(BUILD_FLAG_RENDER_MODE  D3D9)
set(BUILD_FLAG_GCS_GUI      ON)
set(BUILD_FLAG_COPY_TO_SVR  ON)
set(BUILD_FLAG_PROTECT      ON)
set(BUILD_FLAG_PROTECT_VM   ON)

# include build commander
include(${CMAKE_CURRENT_SOURCE_DIR}/../../build_commander.cmake)

# Version 42 Requirement Specific for TF2
add_cxx_lib("-ld3dx9_42")
add_cxx_lib("-lgdi32")

# files
add_cxx_files("*.cpp")

set_cxx_output_dir("${BUILD_PATH_CODEBASE_MAIN}/build/x86")
cxx_compile()