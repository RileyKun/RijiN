
class c_interface_entity_list{
public:
  ALWAYSINLINE c_internal_base_entity* get_entity(i32 index){
    STACK_CHECK_START;
    auto r = index >= 0 ? utils::internal_virtual_call<c_internal_base_entity*, i32>(base_interface->get_entity_index, this, index) : nullptr;
    STACK_CHECK_END;
    return r;
  }

  ALWAYSINLINE c_internal_base_entity* get_entity_handle(i32 handle){
    if(handle <= 0)
      return nullptr;

    #if defined(__x86_64__)
      return utils::internal_virtual_call<c_internal_base_entity*, i32*>(base_interface->get_entity_handle_index, this, &handle);
    #else
      return utils::internal_virtual_call<c_internal_base_entity*, i32>(base_interface->get_entity_handle_index, this, handle);
    #endif
  }

  ALWAYSINLINE i32 get_highest_index(){
    return utils::internal_virtual_call<i32>(base_interface->get_highest_index_index, this);
  }
};