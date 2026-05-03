#pragma once

class c_game_event;
class c_game_event_listener{
public:
  virtual void snb(){};
  virtual void fire_game_event(c_game_event* event);
};

class c_game_manager{
public:
  ALWAYSINLINE c_game_event* create_event(std::string name){
    return utils::internal_virtual_call<c_game_event*, const i8*, bool>(base_interface->create_event_index, this, name.c_str(), false);
  }

  ALWAYSINLINE bool fire_event_client_side(c_game_event* event){
    return utils::internal_virtual_call<bool, c_game_event*>(base_interface->fire_event_client_side_index, this, event);
  }

  ALWAYSINLINE bool find_listener(std::string name, void* listener){
    assert(listener != nullptr);
    return utils::internal_virtual_call<bool, void*, const i8*>(base_interface->find_listener_index, this, listener, name.c_str());
  }

  ALWAYSINLINE void remove_listener(void* listener){
  assert(listener != nullptr);
   utils::internal_virtual_call<void, void*>(base_interface->remove_listener_index, this, listener);
  }

  ALWAYSINLINE bool add_listener(std::string name, void* listener){
    if(find_listener(name, listener))
      return true;

    assert(listener != nullptr);
    return utils::internal_virtual_call<bool, void*, const i8*, bool>(base_interface->add_listener_index, this, listener, name.c_str(), false);
  } 
};