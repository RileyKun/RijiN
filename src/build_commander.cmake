# custom implementation to only call option if its not defined else show whats on from default
function(add_option var_name desc val)
  if(NOT DEFINED ${var_name})
    option(${add_option} ${var_name} ${desc} ${val})
  else()
    message(STATUS "[${var_name}] = ${${var_name}}")
  endif()
endfunction()

# set options
add_option(BUILD_FLAG_CPU_MODE_X86  "x86 mode"                                                            ON)
add_option(BUILD_FLAG_EXE            "EXE or a DLL?"                                                       OFF)
add_option(BUILD_FLAG_NO_CRT        "STL/CRT?"                                                            ON)
add_option(BUILD_FLAG_USE_SIMD      "Use a SIMD Extensions? (Such as SSE/AVX)"                            OFF)
add_option(BUILD_FLAG_SIMD_NO_TAG   "Do not add prefix simd tag to program name"                          OFF)
add_option(BUILD_FLAG_PRESET_MODE   "Compiler preset mode (GEN_SECURE_DLL_NOCRT/EXE)"                     OFF)
add_option(BUILD_FLAG_RENDER_MODE   "Compiler preset render mode (D3D9, D2D1, D3D11)"                     OFF)
add_option(BUILD_FLAG_GCS_GUI       "Include the graphical component system?"                             OFF)
add_option(BUILD_FLAG_HDE_DISASM    "Include the HDE Disassembler?"                                       OFF)
add_option(BUILD_FLAG_DEV_MODE      "Development mode enabled?"                                           OFF)
add_option(BUILD_FLAG_COPY_TO_SVR   "Copy the binary to the servers bin folder?"                          OFF)
add_option(BUILD_FLAG_PROTECT       "Protect the binary (non debug mode only)"                            OFF)
add_option(BUILD_FLAG_PROTECT_VM    "Project the binary with vm technology (requires BUILD_FLAG_PROTECT)" ON)
add_option(BUILD_FLAG_PROTECT_VMCV  "For EXEs that will be virtualized, run code virtualizer also"        OFF)
add_option(BUILD_FLAG_CONFIDENTIAL  "Is this a confidential build?"                                       OFF)
add_option(BUILD_FLAG_RTP           "Include run time protection libraries"                               OFF)
add_option(BUILD_FLAG_SHIPPING_TEST "Force shipping build"                                                OFF)
add_option(BUILD_FLAG_SOURCE_ENGINE "Add the base source engine files to project"                         OFF)
add_option(BUILD_FLAG_NO_OPTIMIZATIONS "No build optimizations"                                           OFF)
add_option(BUILD_FLAG_WANTS_3D         "Wants 3D objects"                                                 OFF)

# use avx2 by default
if(BUILD_FLAG_USE_SIMD MATCHES OFF)
  set(BUILD_FLAG_USE_SIMD "sse2")
endif()

# Fetch codebase main folder or throw error
if(DEFINED ENV{CODEBASE_MAIN})
  file(TO_CMAKE_PATH $ENV{CODEBASE_MAIN} BUILD_PATH_CODEBASE_MAIN)
else()
  message(FATAL_ERROR "Environment variable \"CODEBASE_MAIN\" must be set, example: F:\\RIJIN\\codebase_main")
endif()

# Check for git, if it exists then get some info
find_package (Git)
if (GIT_FOUND)
  exec_program(
    ${GIT_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ARGS "rev-parse --abbrev-ref HEAD"
    OUTPUT_VARIABLE BUILD_GIT_BRANCH_NAME
  )

  # any mode but shipping is considered development mode
  if(NOT BUILD_GIT_BRANCH_NAME MATCHES shipping)
    set(BUILD_FLAG_DEV_MODE ON)
  endif()

  # any mode but shipping is considered development mode
  if(BUILD_GIT_BRANCH_NAME MATCHES staging)
    set(BUILD_FLAG_DEV_MODE OFF)
    set(BUILD_FLAG_STAGING_MODE ON)
    message(STATUS "[!] BUILD_FLAG_STAGING_MODE")
  endif()

  if(BUILD_FLAG_SHIPPING_TEST MATCHES ON)
    set(BUILD_FLAG_DEV_MODE OFF)
  endif()

  if(BUILD_FLAG_DEV_MODE MATCHES ON)
    set(BUILD_FLAG_USE_SIMD sse2)
  endif()

  if(BUILD_FLAG_STAGING_MODE MATCHES ON)
    set(BUILD_FLAG_USE_SIMD sse2)
  endif()
else()
  message(FATAL_ERROR "Git is not installed, you must have git installed.")
endif()

message(STATUS "${BUILD_GIT_BRANCH_NAME}")

# IMPORTANT!
# CMake includes a bunch of libraries by default
# Which causes a issue where libgcc is not included and other random libraries are
# This causes linker errors, It should also be noted that -l<lib> has no effect in the compiler flags on CMake
if(BUILD_FLAG_NO_CRT MATCHES ON)
  set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "")
  set(CMAKE_CXX_STANDARD_LIBRARIES "")
endif()

# Set compiler based on cpu mode
if(BUILD_FLAG_CPU_MODE_X86 MATCHES ON)
  set(CMAKE_CXX_COMPILER g++)
elseif(BUILD_FLAG_CPU_MODE_X86 MATCHES OFF)
  set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
endif()

# Add source files into the build list
function(add_cxx_files FILES_STR)
  file(GLOB src_list CONFIGURE_DEPENDS "${FILES_STR}")
  set(BUILD_SOURCES_LIST "${BUILD_SOURCES_LIST}${src_list};" PARENT_SCOPE)
endfunction()

# Add CXX flags for the compiler
function(add_cxx_flags FLAGS_STR)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS_STR}" PARENT_SCOPE)
endfunction()

# Add linker flags for the linker
function(add_linker_flags FLAGS_STR)
  if(BUILD_FLAG_EXE MATCHES ON)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${FLAGS_STR}" PARENT_SCOPE)
  else()
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${FLAGS_STR}" PARENT_SCOPE)
  endif()
endfunction()

# Add library to linker flags
function(add_cxx_lib LIB_STR)
  # Why not just add it to CMAKE_CXX_FLAGS?
  # CMake links libraries based on CMAKE_CXX_STANDARD_LIBRARIES, it seems to remove all -l<x> params
  # From CMAKE_CXX_FLAGS (no idea why)
  set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} ${LIB_STR}" PARENT_SCOPE)
endfunction()

# Add macro definition to compiler flags
function(add_macro MACRO_STR)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D${MACRO_STR}" PARENT_SCOPE)
endfunction()

# Sets the output directory of the binary
function(set_cxx_output_dir OUTPUT_DIR)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR} PARENT_SCOPE)
endfunction()

# Compiles to a binary
function(cxx_compile)
  if(BUILD_FLAG_EXE MATCHES ON)
    add_executable(${PROJECT_NAME} ${BUILD_SOURCES_LIST})
  else()
    set(CMAKE_SHARED_LIBRARY_PREFIX "" PARENT_SCOPE)
    add_library(${PROJECT_NAME} SHARED ${BUILD_SOURCES_LIST})
  endif()

  if((BUILD_FLAG_DEV_MODE MATCHES OFF))
    add_custom_command(
      TARGET ${PROJECT_NAME}
      POST_BUILD
      COMMAND "${BUILD_PATH_CODEBASE_MAIN}/rijin_clean_binary.exe" -"$<TARGET_FILE:${PROJECT_NAME}>"
    )
  endif()

  if((BUILD_FLAG_DEV_MODE MATCHES OFF) AND (BUILD_FLAG_PROTECT MATCHES ON) AND (BUILD_FLAG_PROTECT_VM MATCHES ON))
    add_custom_command(
      TARGET ${PROJECT_NAME}
      POST_BUILD
      COMMAND echo [!] Starting code virtualizer...
    )

    if(BUILD_FLAG_PRESET_MODE MATCHES GEN_SECURE_DLL_NOCRT)
      add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND "${BUILD_PATH_CODEBASE_MAIN}/vm/themida_cv/Virtualizer.exe" /protect "${BUILD_PATH_CODEBASE_MAIN}/vm/gen_secure_dll_nocrt.cv" /software "${PROJECT_NAME}" /inputfile "$<TARGET_FILE:${PROJECT_NAME}>" /outputfile "$<TARGET_FILE:${PROJECT_NAME}>"
      )
    else()
      if(BUILD_FLAG_PROTECT_VMCV MATCHES ON)
        add_custom_command(
          TARGET ${PROJECT_NAME}
          POST_BUILD
          COMMAND "${BUILD_PATH_CODEBASE_MAIN}/vm/themida_cv/Virtualizer.exe" /protect "${BUILD_PATH_CODEBASE_MAIN}/vm/exe.cv" /software "${PROJECT_NAME}" /inputfile "$<TARGET_FILE:${PROJECT_NAME}>" /outputfile "$<TARGET_FILE:${PROJECT_NAME}>"
        )

        add_custom_command(
          TARGET ${PROJECT_NAME}
          POST_BUILD
          COMMAND echo [!] Starting virtualizer...
        )
      endif()

      if(BUILD_FLAG_CPU_MODE_X86 MATCHES ON)
        add_custom_command(
          TARGET ${PROJECT_NAME}
          POST_BUILD
          COMMAND "${BUILD_PATH_CODEBASE_MAIN}/vm/themida/Themida.exe" /protect "${BUILD_PATH_CODEBASE_MAIN}/vm/exe.tmd" /software "${PROJECT_NAME}" /inputfile "$<TARGET_FILE:${PROJECT_NAME}>" /outputfile "$<TARGET_FILE:${PROJECT_NAME}>"
        )
      else()
        add_custom_command(
          TARGET ${PROJECT_NAME}
          POST_BUILD
          COMMAND "${BUILD_PATH_CODEBASE_MAIN}/vm/themida/Themida64.exe" /protect "${BUILD_PATH_CODEBASE_MAIN}/vm/exe.tmd" /software "${PROJECT_NAME}" /inputfile "$<TARGET_FILE:${PROJECT_NAME}>" /outputfile "$<TARGET_FILE:${PROJECT_NAME}>"
        )
      endif()
    endif()

    add_custom_command(
      TARGET ${PROJECT_NAME}
      POST_BUILD
      COMMAND del /Q "$<SHELL_PATH:$<TARGET_FILE:${PROJECT_NAME}>>.bak"
    )
  endif()

  # Instruct the compiler to copy the file to the loader bin folder after
  if(BUILD_FLAG_COPY_TO_SVR MATCHES ON)
    add_custom_command(
      TARGET ${PROJECT_NAME}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:${PROJECT_NAME}>
        "${BUILD_PATH_CODEBASE_MAIN}/src/loader/loader_server/bin"
    )
  endif()
endfunction()

# Add linker flag to explicitly compile with no standard library
if(BUILD_FLAG_NO_CRT MATCHES ON)
  add_linker_flags("-nostdlib")
endif()

# Let the progrma know we are in debug mode
if(BUILD_FLAG_DEV_MODE MATCHES ON)
  message(STATUS "BUILDING IN DEVELOPMENT MODE")
endif()

# if not shipping and not staging either then assume then add the "DEV_MODE" macro so we can have dev enabled even in like test shipping mode.
if(NOT BUILD_GIT_BRANCH_NAME MATCHES shipping)
  if(NOT BUILD_GIT_BRANCH_NAME MATCHES staging)
    message(STATUS " Adding DEV_MODE macro!")
    add_macro("DEV_MODE")
  endif()
endif()

# Let the progrma know we are in staging mode
if(BUILD_FLAG_STAGING_MODE MATCHES ON)
  message(STATUS "BUILDING IN STAGING MODE")

  add_macro("STAGING_MODE")
endif()

# Add HDE if required
if(BUILD_FLAG_HDE_DISASM MATCHES ON)
  add_macro("HDE_DISASM")
  add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/hde/*.cpp")
endif()

# If its the source engine then add the macro for it.
if(BUILD_FLAG_SOURCE_ENGINE MATCHES ON)
  add_macro("SOURCE_CHEAT")
  add_macro("GEN_INTERNAL")
  add_macro("CLOUD_CONFIG")
endif()

# Add some build version information
add_macro("BUILD_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}")
add_macro("BUILD_VERSION_MINOR=${PROJECT_VERSION_MINOR}")
add_macro("BUILD_VERSION_PATCH=${PROJECT_VERSION_PATCH}")
add_macro("BUILD_VERSION_TWEAK=${PROJECT_VERSION_TWEAK}")
add_macro("BUILD_VERSION_BRANCH=${BUILD_GIT_BRANCH_NAME}")
add_definitions(-DBUILD_VERSION=${PROJECT_VERSION_MAJOR}${PROJECT_VERSION_MINOR}${PROJECT_VERSION_PATCH})

# calculate tweak text
if(NOT PROJECT_VERSION_TWEAK STREQUAL "")
  set(PROJECT_VERSION_TWEAK "HOTFIX-${PROJECT_VERSION_TWEAK}.")
endif()

string(TIMESTAMP BUILD_VERSION_STR "%y%m%d ${BUILD_FLAG_USE_SIMD}/${BUILD_GIT_BRANCH_NAME}")
string(TOUPPER ${BUILD_VERSION_STR} BUILD_VERSION_STR)
message(STATUS "Version: ${BUILD_VERSION_STR}")
add_definitions(-DBUILD_VERSION_STR=L"${BUILD_VERSION_STR}")

# Decide what compiler settings we are using
if(BUILD_FLAG_PRESET_MODE MATCHES GEN_SECURE_DLL_NOCRT) # Build with preset cheat compiler/linker settings
  # Compiler settings
  add_cxx_flags("-std=c++20")           # C++20
  add_cxx_flags("-s")                   # Remove symbol table & relocation info

  if(BUILD_FLAG_NO_OPTIMIZATIONS MATCHES OFF)
    if(BUILD_FLAG_CPU_MODE_X86 MATCHES ON)
      add_cxx_flags("-O2")                  # strict optimisations
    else()
      add_cxx_flags("-Os")                  # strict optimisations
      add_cxx_flags("-falign-functions -falign-jumps -falign-labels -falign-loops -freorder-blocks-algorithm=stc") # strict optimisations
    endif()
  else()
    add_cxx_flags("-O0")
  endif()

  add_cxx_flags("-mnop-fun-dllimport")  # exports are demangled
  add_cxx_flags("-w")                   # disable all warnings
  add_cxx_flags("-fpermissive")         # disable some stupid errors like cast warnings (i.e: c_test* test = (void*)malloc(1))
  add_cxx_flags("-Wall")
  add_cxx_flags("-Wextra")

  # Ignore annoying warnings
  add_cxx_flags("-Wno-sign-compare")
  add_cxx_flags("-Wno-deprecated-copy")
  add_cxx_flags("-Wno-unused-parameter")
  add_cxx_flags("-Wno-write-strings")
  add_cxx_flags("-Wno-unused-value")
  add_cxx_flags("-Wno-narrowing")
  add_cxx_flags("-Wno-parentheses")
  add_cxx_flags("-Wno-unused-but-set-parameter")
  add_cxx_flags("-Wno-class-memaccess")
  add_cxx_flags("-Wno-unused-function")
  add_cxx_flags("-Wno-missing-field-initializers")
  add_cxx_flags("-Wno-conversion")
  add_cxx_flags("-Wno-unused-variable")
  add_cxx_flags("-Wno-int-to-pointer-cast")

  add_cxx_flags("-Werror=return-type") # Functions that need a return but lack one.
  add_cxx_flags("-Werror=delete-non-virtual-dtor") # Can cause heap issues if not addressed.

  add_cxx_flags("-fno-exceptions")      # disable try/catch
  add_cxx_flags("-fno-rtti") # Disable RTTI very important for anti-reversing.

  # If an exception occurs, this will make it harder to debug issues with our cheats.
  add_cxx_flags("-fno-unwind-tables")
  add_cxx_flags("-fno-asynchronous-unwind-tables")

  # This stops the compiler from unrolling loops, which can break code sometimes.
  add_cxx_flags("-fno-unroll-loops")

  # No math error return types (pretty sure)
  add_cxx_flags("-fno-math-errno")

  # If you have like a string with the same text, all refs will use the same address.
  add_cxx_flags("-fmerge-all-constants")
  add_cxx_flags("-fno-ident")

  add_cxx_flags("-fno-threadsafe-statics")


  add_cxx_flags("-march=x86-64")
  add_cxx_flags("-mtune=generic")

  # Messes with our generator
  if(BUILD_FLAG_NO_CRT MATCHES ON)
    add_cxx_flags("-fno-strict-aliasing -fno-fast-math -fno-tree-ccp -fno-strength-reduce -fno-peephole -fno-peephole2 -fno-tree-sra -fno-reorder-blocks -fno-ipa-ra")
  endif()

  # -fno-tree-ccp (without)
  #  int a = 5;
  #  int b = a * 10;  // Without `-fno-tree-ccp`, GCC will precompute this as `b = 50;`

  # -fno-strength-reduce (without)
  # x * 2;  // Might be optimized to x << 1;
  # x / 4;  // Might be optimized to x >> 2;


  # Linker settings
  add_linker_flags("-nostdlib") # Explicitly always no STL/CRT

  if(BUILD_FLAG_DEV_MODE MATCHES ON)
    add_linker_flags("-Wl,--export-all-symbols,--kill-at,--entry=0,--image-base=0x0") # Export ALL symbols, less strict mangles, no entry point, image base 0
  else()
    add_linker_flags("-Wl,--exclude-all-symbols,--kill-at,--entry=0,--image-base=0x0") # Exclude ALL symbols, less strict mangles, no entry point, image base 0
  endif()

  # Default libraries
  add_cxx_lib("-lgcc")      # required for things like chckstk_ms/chckstk_ms
  add_cxx_lib("-lkernel32") # required
  add_cxx_lib("-luser32")   # required
  #add_cxx_lib("-lshlwapi")

  # Let the program know we are in cheat mode
  add_macro("INTERNAL_CHEAT")
  add_macro("RENDER_3D")

  # Add support for GCC 13.1.0 -senator
  add_macro("_GLIBCXX_USE_CXX11_ABI=0")

  # Include all base files required for cheats
  add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/*.cpp")
  add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/math/*.cpp")
  add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/cheat_shared/*.cpp")
  add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/shared_globally/*.cpp")

  if(BUILD_FLAG_SOURCE_ENGINE MATCHES ON)
    add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/cheats/source/*.cpp")
    add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/cheats/source/interfaces/*.cpp")
    add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/cheats/source/features/*.cpp")
    add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/cheats/source/managers/*.cpp")
    add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/cheats/source/structs/*.cpp")
  endif()

  # handle obfuscations
  if((BUILD_FLAG_DEV_MODE MATCHES OFF) AND (BUILD_FLAG_PROTECT MATCHES ON))
    add_macro("ANTI_REVERSE_STRING_ENCRYPTION")
    add_macro("ANTI_REVERSE_POINTER_ENCRYPTION")

    # VM Engine
    if(BUILD_FLAG_PROTECT_VM MATCHES ON)
      add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/protections/themida/themida_stealth.cpp")
      add_macro("ANTI_REVERSE_VM_ENGINE")
    endif()

    # ANTI_REVERSE_FUNC_OBFUSCATION is not possible without crt implementation
  endif()
elseif(BUILD_FLAG_PRESET_MODE MATCHES EXE) # Build with the default compiler mode settings (CRT & (EXE/DLL))
  # Compiler settings
  add_cxx_flags("-std=c++20")           # C++20
  add_cxx_flags("-s")                   # Remove symbol table & relocation info
  add_cxx_flags("-O2")                  # size optimisations
  add_cxx_flags("-w")                   # disable all warnings
  add_cxx_flags("-fpermissive")         # disable some stupid errors like cast warnings (i.e: c_test* test = (void*)malloc(1))
  add_cxx_flags("-fcheck-new")          # compiler sanity checks
  add_cxx_flags("-fno-exceptions")      # disable try/catch
  add_cxx_flags("-fno-rtti -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-math-errno -fno-unroll-loops -fmerge-all-constants -fno-ident")
  add_cxx_flags("-static")              # staticly link all includes (like so c++ redists are not needed)
  add_cxx_flags("-march=x86-64")
  add_cxx_flags("-mtune=generic")
  add_cxx_flags("-mwindows")

  # Linker settings
  if(BUILD_FLAG_DEV_MODE MATCHES ON)
    add_linker_flags("-Wl,--export-all-symbols")
  else()
    add_linker_flags("-Wl,--exclude-all-symbols")
  endif()

  # Include all base files required for cheats
  add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/*.cpp")
  add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/math/*.cpp")
  add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/loader_shared/*.cpp")
  add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/shared_globally/*.cpp")

if(BUILD_FLAG_RTP MATCHES ON)
  add_macro("BUILD_FLAG_RTP")
  add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/protections/rtp/rtp.cpp")
endif()

  # handle obfuscations
  if((BUILD_FLAG_DEV_MODE MATCHES OFF) AND (BUILD_FLAG_PROTECT MATCHES ON))
    add_macro("ANTI_REVERSE_STRING_ENCRYPTION")
    add_macro("ANTI_REVERSE_POINTER_ENCRYPTION")
    add_macro("ANTI_REVERSE_FUNC_OBFUSCATION")

    # VM Engine
    if(BUILD_FLAG_PROTECT_VM MATCHES ON)
      add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/protections/themida/themida_stealth.cpp")
      add_macro("ANTI_REVERSE_VM_ENGINE")
    endif()
  endif()

elseif(NOT BUILD_FLAG_PRESET_MODE MATCHES OFF)
  message(FATAL_ERROR "BUILD_FLAG_PRESET_MODE is not recognised!")
endif()

if(BUILD_FLAG_SHIPPING_TEST MATCHES ON)
  add_macro("BUILD_FLAG_TALK_TO_LOCALHOST")
endif()

if(BUILD_FLAG_CONFIDENTIAL MATCHES ON)
  add_macro("CONFIDENTIAL_BUILD")
endif()

# Include render modes (if we are using any)
if(BUILD_FLAG_RENDER_MODE MATCHES D3D9)
  add_macro("RENDERING_ENABLE")
  add_macro("RENDERING_MODE_D3D9")
  add_cxx_lib("-ld3d9")

  add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/render/*.cpp")
elseif(BUILD_FLAG_RENDER_MODE MATCHES D2D1)
  add_macro("RENDERING_ENABLE")
  add_macro("RENDERING_MODE_D2D1")
  add_cxx_lib("-ld3d11")
  add_cxx_lib("-ld2d1")
  add_cxx_lib("-ldwrite")

  add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/render/*.cpp")
elseif(BUILD_FLAG_RENDER_MODE MATCHES D3D11_IMGUI)
  add_macro("RENDERING_ENABLE")
  add_macro("RENDERING_MODE_D3D11_IMGUI")

  add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/render/*.cpp")

  # Include all the imgui files required
  add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/render/imgui/font_segoeui.cpp")
  add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/render/imgui/font_tahomabd.cpp")
  add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/render/imgui/imgui.cpp")
  add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/render/imgui/imgui_draw.cpp")
  add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/render/imgui/imgui_impl_dx11.cpp")
  add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/render/imgui/imgui_impl_win32.cpp")
  add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/render/imgui/imgui_widgets.cpp")
  add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/render/imgui/imgui_tables.cpp")

elseif(NOT BUILD_FLAG_RENDER_MODE MATCHES OFF)
  message(FATAL_ERROR "BUILD_FLAG_RENDER_MODE is not recognised!")
endif()

# Include GCS GUI System
if(BUILD_FLAG_GCS_GUI MATCHES ON)
  add_macro("GCS")
  add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/render/gui/*.cpp")
  add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/render/gui/base/*.cpp")
  add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/render/gui/components/*.cpp")

  if(BUILD_FLAG_SOURCE_ENGINE MATCHES ON)
    add_cxx_files("${BUILD_PATH_CODEBASE_MAIN}/src/base/cheats/source/gcs/*.cpp")
  endif()
endif()

if(BUILD_FLAG_WANTS_3D MATCHES ON)
  add_macro("RENDER_3D")
endif()

# Are we going to be usign a simd set?
if(NOT BUILD_FLAG_USE_SIMD MATCHES OFF)
  add_cxx_flags("-m${BUILD_FLAG_USE_SIMD}")

  # Add extension onto name
  if(BUILD_FLAG_SIMD_NO_TAG MATCHES OFF)
    set(PROJECT_NAME "${PROJECT_NAME}_${BUILD_FLAG_USE_SIMD}")
  endif()

  # Remove from cache
  unset(BUILD_FLAG_USE_SIMD CACHE)
endif()