#pragma once

class c_game_event{
public:
  ALWAYSINLINE i8* get_name(){
    return utils::internal_virtual_call<i8*>(base_interface->ge_get_name_index, this);
  }

  ALWAYSINLINE bool get_bool(std::string key_name){
    return utils::internal_virtual_call<bool, const i8*, bool>(base_interface->ge_get_bool_index, this, key_name.c_str(), false);
  }

  ALWAYSINLINE i32 get_int(std::string key_name, i32 default_value = 0){
    return utils::internal_virtual_call<i32, const i8*, i32>(base_interface->ge_get_int_index, this, key_name.c_str(), default_value);
  }

  ALWAYSINLINE float get_float(std::string key_name, float default_value = 0.f){
    return utils::internal_virtual_call<float, const i8*, float>(base_interface->ge_get_float_index, this, key_name.c_str(), default_value);
  }

  ALWAYSINLINE i8* get_string(std::string key_name, std::string default_value = ""){
    return utils::internal_virtual_call<i8*, const i8*, const i8*>(base_interface->ge_get_string_index, this, key_name.c_str(), nullptr);
  }

  ALWAYSINLINE void set_bool(std::string key_name, bool value){
    utils::internal_virtual_call<void, const i8*, bool>(base_interface->ge_set_bool_index, this, key_name.c_str(), value);
  }

  ALWAYSINLINE void set_int(std::string key_name, i32 value){
    utils::internal_virtual_call<void, const i8*, i32>(base_interface->ge_set_int_index, this, key_name.c_str(), value);
  }

  ALWAYSINLINE void set_float(std::string key_name, float value){
    utils::internal_virtual_call<void, const i8*, float>(base_interface->ge_set_float_index, this, key_name.c_str(), value);
  }

  ALWAYSINLINE void set_string(std::string key_name, std::string value){
    utils::internal_virtual_call<void, const i8*, const i8*>(base_interface->ge_set_string_index, this, key_name.c_str(), value.c_str());
  }
};