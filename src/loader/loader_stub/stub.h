#pragma once

class c_stub{
public:
  c_stub_data* get_data(i8* cmd_line){
    
    // 1. Open the file map
    void* map_file = I(OpenFileMappingA)(XOR32(FILE_MAP_ALL_ACCESS), false, cmd_line);
    
    if(map_file == nullptr){
      DBG("[!] SILENT ERROR: ERR_STUB_FAILED_TO_OPEN_FILE_MAPPING");
      assert(false && "stub open map file failed");
      return nullptr;
    }
    
    // 2. Map the view of the file
    c_stub_data* buffer_map = (c_stub_data*)I(MapViewOfFile)(map_file, XOR32(FILE_MAP_ALL_ACCESS), 0, 0, XOR32(sizeof(c_stub_data)));
    
    if(buffer_map == nullptr){
      assert(false && "stub open map file failed 2");
      I(CloseHandle)(map_file);
      return nullptr;
    }
    
    // 3. Create our own copy of this data and then zero it
    c_stub_data* new_buffer = (c_stub_data*)malloc(XOR32(sizeof(c_stub_data)));
    memset(new_buffer, 0, XOR32(sizeof(c_stub_data)));
    I(memcpy)(new_buffer, buffer_map, XOR32(sizeof(c_stub_data)));
    
    // 3.1 Zero the original buffer map
    memset(buffer_map, 0, XOR32(sizeof(c_stub_data)));
    buffer_map->loaded = true;
    
    // 4. Now unmap the view of the file so that the page file memory is removed
    I(UnmapViewOfFile)(buffer_map);
    I(CloseHandle)(map_file);
    
    cookie_xor((u8*)&new_buffer->object, XOR32(sizeof(license_object)));
    cookie_xor(new_buffer->key, XOR32(sizeof(c_stub_data::key)));
    return new_buffer;
  }
};

CLASS_EXTERN(c_stub, stub);