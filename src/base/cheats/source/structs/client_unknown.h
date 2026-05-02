#pragma once


class c_client_renderable;
class c_client_networkable;
class c_client_unknown{
public:

  ALWAYSINLINE c_client_renderable* get_renderable(){ // 5
    return utils::virtual_call<5, void*>(this);
  }

  ALWAYSINLINE c_client_networkable* get_networkable(){ // 4
    return utils::virtual_call<4, void*>(this);
  }

  ALWAYSINLINE c_internal_base_entity* get_base_entity(){ // Index should always be 7.
    return utils::virtual_call<7, void*>(this);
  }
};

class c_client_renderable{
public:
  ALWAYSINLINE c_client_unknown* get_client_unknown(){
    return utils::virtual_call<0, void*>(this);
  }
};


class c_client_networkable{
public:
  ALWAYSINLINE c_client_unknown* get_client_unknown(){
    return utils::virtual_call<0, void*>(this);
  }
};