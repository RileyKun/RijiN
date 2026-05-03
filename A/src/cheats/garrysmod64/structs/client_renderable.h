#pragma once

class c_client_renderable{
public:
  void* get_client_unknown(){
    return utils::virtual_call64<0, void*>(this);
  }
};