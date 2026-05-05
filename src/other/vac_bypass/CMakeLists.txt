cmake_minimum_required(VERSION 3.21.3)
project(vac_bypass VERSION 1.0.0 LANGUAGES CXX)

# set our custom flags
set(BUILD_FLAG_CPU_MODE_X86 OFF)
set(BUILD_FLAG_EXE          OFF)
set(BUILD_FLAG_NO_CRT       OFF)
set(BUILD_FLAG_USE_SIMD     sse2)
set(BUILD_FLAG_SIMD_NO_TAG  ON)
set(BUILD_FLAG_PRESET_MODE  GEN_SECURE_DLL_NOCRT)
set(BUILD_FLAG_COPY_TO_SVR  ON)
set(BUILD_FLAG_PROTECT      OFF)
set(BUILD_FLAG_PROTECT_VM   OFF)
set(BUILD_FLAG_HDE_DISASM   ON)
set(BUILD_FLAG_DEV_MODE     ON)

# include build commander
include(${CMAKE_CURRENT_SOURCE_DIR}/../../build_commander.cmake)

add_macro("NEED_SIG_SCANNER")
add_macro("VAC_BYPASS_MODULE")
add_macro("GEN_INTERNAL")

# files
add_cxx_files("*.cpp")

set_cxx_output_dir("${BUILD_PATH_CODEBASE_MAIN}/build/x86")
cxx_compile()