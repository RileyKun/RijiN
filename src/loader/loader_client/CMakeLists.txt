cmake_minimum_required(VERSION 3.21.3)
project(loader VERSION 3.3.0 LANGUAGES CXX)

# set our custom flags
set(BUILD_FLAG_CPU_MODE_X86 OFF)
set(BUILD_FLAG_EXE          ON)
set(BUILD_FLAG_NO_CRT       OFF)
set(BUILD_FLAG_USE_SIMD     sse2)
set(BUILD_FLAG_HDE_DISASM   ON)
set(BUILD_FLAG_SIMD_NO_TAG  ON)
set(BUILD_FLAG_PRESET_MODE  EXE)
set(BUILD_FLAG_RENDER_MODE  D2D1)
set(BUILD_FLAG_GCS_GUI      ON)
set(BUILD_FLAG_PROTECT      ON)
set(BUILD_FLAG_PROTECT_VM   ON)
set(BUILD_FLAG_PROTECT_VMCV ON)
set(BUILD_FLAG_RTP          ON)

# include build commander
include(${CMAKE_CURRENT_SOURCE_DIR}/../../build_commander.cmake)

# required libs
add_cxx_lib("-lcredui")   # Credui
add_cxx_lib("-lgdi32")    # Gdi32
add_cxx_lib("-liphlpapi") # Iphlpapi
add_cxx_lib("-lRpcrt4")   # Rpcrt4
add_cxx_lib("-lWs2_32")   # Ws2_32
add_cxx_lib("-ladvapi32") # Advapi32
add_cxx_lib("-lwindowscodecs") # windowscodecs
add_cxx_lib("-lWininet") # Wininet
add_cxx_lib("-ldwmapi") # Dwmapi

# use printf
add_macro("DEBUG_USE_PRINTF")
add_macro("LOADER_CLIENT")

# files
add_cxx_files("*.cpp")

set_cxx_output_dir("${BUILD_PATH_CODEBASE_MAIN}/build/x64")
cxx_compile()