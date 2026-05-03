cmake_minimum_required(VERSION 3.21.3)
project(stub VERSION 1.0.0 LANGUAGES CXX)

# set our custom flags
set(BUILD_FLAG_CPU_MODE_X86 OFF)
set(BUILD_FLAG_EXE          OFF)
set(BUILD_FLAG_NO_CRT       OFF)
set(BUILD_FLAG_USE_SIMD     sse2)
set(BUILD_FLAG_SIMD_NO_TAG  ON)
set(BUILD_FLAG_PRESET_MODE  EXE)
set(BUILD_FLAG_HDE_DISASM   ON)
set(BUILD_FLAG_COPY_TO_SVR  ON)
set(BUILD_FLAG_PROTECT      OFF)
set(BUILD_FLAG_PROTECT_VM   OFF)
set(BUILD_FLAG_PROTECT_VMCV OFF)
set(BUILD_FLAG_RTP          ON)

# include build commander
include(${CMAKE_CURRENT_SOURCE_DIR}/../../build_commander.cmake)

# required libs
add_cxx_lib("-Lwininet")   # WinSock2
add_cxx_lib("-lcredui")    # Credui

# use printf
add_macro("DEBUG_USE_PRINTF")
add_macro("LOADER_STUB")
add_macro("GEN_INTERNAL_NETWORK")

# disable inlining on debug builds
if(BUILD_FLAG_DEV_MODE MATCHES ON)
  add_macro("ALWAYSINLINE=")
endif()

# files
add_cxx_files("*.cpp")

set_cxx_output_dir("${BUILD_PATH_CODEBASE_MAIN}/build/x64")
cxx_compile()