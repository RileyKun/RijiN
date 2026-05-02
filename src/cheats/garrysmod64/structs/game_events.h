#pragma once

class c_game_event{
public:
  ALWAYSINLINE u8* get_name(){
    return utils::virtual_call64<1, u8*>(this);
  }

  ALWAYSINLINE bool get_bool( u8* key_name ){
    return utils::virtual_call64<5, bool, u8*, bool>(this, key_name, false);
  }

  ALWAYSINLINE i32 get_int( u8* key_name, i32 default_value = 0 ){
    return utils::virtual_call64<6, i32, u8*, i32>(this, key_name, default_value);
  }

  ALWAYSINLINE float get_float( u8* key_name, float default_value = 0.f ){
    return utils::virtual_call64<7, float, u8*, float>(this, key_name, default_value);
  }

  ALWAYSINLINE i8* get_string( u8* key_name, u8* default_value = ""){
    return utils::virtual_call64<8, i8*, u8*, u8*>(this, key_name, default_value);
  }

  ALWAYSINLINE void set_bool( u8* key_name, bool value ){
    utils::virtual_call64<9, void, u8*, bool>(this, key_name, value);
  }

  ALWAYSINLINE void set_int( u8* key_name, i32 value ){
    utils::virtual_call64<10, void, u8*, i32>(this, key_name, value);
  }

  ALWAYSINLINE void set_float( u8* key_name, float value ){
    utils::virtual_call64<11, void, u8*, float>(this, key_name, value);
  }

  ALWAYSINLINE void set_string( u8* key_name, u8* value ){
    utils::virtual_call64<12, void, u8*, u8*>(this, key_name, value);
  }
};