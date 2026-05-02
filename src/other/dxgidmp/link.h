#pragma once

// base
#include "../../base/link.h"

// base: additional
#include "../../base/loader_shared/link.h"

// rendering
#include "../../base/render/link.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <windows.h>
#include "typedefs.h"

#include <d3d9.h>
#include <d3d11.h>
#include <d3dcompiler.h>

static i32  arg_count;
static i8** arg_array;

namespace utils{
  inline i8* get_arg( std::string name ){
    if ( arg_count < 1 || arg_array == nullptr )
      return nullptr;
    
    for ( u32 i = 0; i < arg_count; i++ ) {
      const i8* str = arg_array[ i ];
    
      if ( strcmp( str, name.c_str( ) ) == 0 && i + 1 <= arg_count ) {
        return arg_array[ i + 1 ];
      }
    }
    
    return nullptr;
  }

  inline std::string format( const i8* format, ... ) {
    va_list list;
    va_start( list, format );

    i8 buffer[ 4096 ] = { 0 };
    vsnprintf( buffer, 4096, format, list );

    va_end( list );

    return std::string( buffer );
  }

  inline std::string replace( std::string main_str, const std::string& search, const std::string& replace ) {
    u64 pos = 0;

    while ( ( pos = main_str.find( search, pos ) ) != std::string::npos ) {
      main_str.replace( pos, search.length( ), replace );
      pos += replace.length( );
    }

    return main_str;
  }

  inline bool write_file( const std::string& file_name, void* buffer, const u32 len ) {
    void* const handle = CreateFileA( file_name.c_str( ), GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr );

    if ( handle == nullptr )
      return false;

    u32 wrote_bytes;
    WriteFile( handle, buffer, len, reinterpret_cast<ul64*>( &wrote_bytes ), nullptr );

    CloseHandle( handle );
    return true;
  }

}