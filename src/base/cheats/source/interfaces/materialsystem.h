#pragma once

class c_texture{
public:
  ALWAYSINLINE i32 get_actual_width(){
    STACK_CHECK_START;
    auto r =  utils::internal_virtual_call<i32>(base_interface->texture_get_actual_width_index, this);
    STACK_CHECK_END;
    return r;
  }

  ALWAYSINLINE i32 get_actual_height(){
    STACK_CHECK_START;
    auto r = utils::internal_virtual_call<i32>(base_interface->texture_get_actual_height_index, this);
    STACK_CHECK_END;
    return r;
  }

  ALWAYSINLINE void increment_reference_count(){
    STACK_CHECK_START;
    utils::internal_virtual_call<void>(base_interface->texture_increment_reference_count_index, this);
    STACK_CHECK_END;
  }
};

class c_material;
class c_render_context{
public:

  i32 release(){
    STACK_CHECK_START;
    auto r = utils::internal_virtual_call<i32>(base_interface->rc_release_index, this);
    STACK_CHECK_END;
    return r;
  }

  void depth_range(float z_near, float z_far){
    STACK_CHECK_START;
    utils::internal_virtual_call<float, float>(base_interface->rc_depth_range_index, this, z_near, z_far);
    STACK_CHECK_END;
  }

#if !defined(SOURCE_2018)
  void set_stencil_enable(bool enable){
    STACK_CHECK_START;
    utils::internal_virtual_call<void, bool>(base_interface->set_stencil_enable_index, this, enable);
    STACK_CHECK_END;
  }

  void set_stencil_fail_operation(shader_stencil_op op){
    STACK_CHECK_START;
    utils::internal_virtual_call<void, shader_stencil_op>(base_interface->set_stencil_fail_operation_index, this, op);
    STACK_CHECK_END;
  }

  void set_stencil_zfail_operation(shader_stencil_op op){
    STACK_CHECK_START;
    utils::internal_virtual_call<void, shader_stencil_op>(base_interface->set_stencil_zfail_operation_index, this, op);
    STACK_CHECK_END;
  }

  void set_stencil_pass_operation(shader_stencil_op op){
    STACK_CHECK_START;
    utils::internal_virtual_call<void, shader_stencil_op>(base_interface->set_stencil_pass_operation_index, this, op);
    STACK_CHECK_END;
  }

  void set_stencil_compare_function(stencil_comparison_function cmpfn){
    STACK_CHECK_START;
    utils::internal_virtual_call<void, stencil_comparison_function>(base_interface->set_stencil_compare_function_index, this, cmpfn);
    STACK_CHECK_END;
  }

  void set_stencil_reference_value(i32 ref){
    STACK_CHECK_START;
    utils::internal_virtual_call<void, i32>(base_interface->set_stencil_reference_value_index, this, ref);
    STACK_CHECK_END;
  }

  void set_stencil_test_mask(u32 mask){
    STACK_CHECK_START;
    utils::internal_virtual_call<void, u32>(base_interface->set_stencil_test_mask_index, this, mask);
    STACK_CHECK_END;
  }

  void set_stencil_write_mask(u32 mask){
    STACK_CHECK_START;
    utils::internal_virtual_call<void, u32>(base_interface->set_stencil_write_mask_index, this, mask);
    STACK_CHECK_END;
  }
#endif

  void set_stencil_state(s_stencil_state* stencil_state);

  void push_render_target_and_viewport(){
    STACK_CHECK_START;
    utils::internal_virtual_call<void>(base_interface->push_render_target_and_viewport_index, this);
    STACK_CHECK_END;
  }

  void pop_render_target_and_viewport(){
    STACK_CHECK_START;
    utils::internal_virtual_call<void>(base_interface->pop_render_target_and_viewport_index, this);
    STACK_CHECK_END;
  }

  void set_render_target(c_texture* texture){
    STACK_CHECK_START;
    utils::internal_virtual_call<void, c_texture*>(base_interface->set_render_target_index, this, texture);
    STACK_CHECK_END;
  }

  void viewport(i32 x, i32 y, i32 w, i32 h){
    STACK_CHECK_START;
    utils::internal_virtual_call<void, i32, i32, i32, i32>(base_interface->rc_viewport_index, this, x, y, w, h);
    STACK_CHECK_END;
  }

  void clear_colour_4ub(u8 r, u8 g, u8 b, u8 a){
    STACK_CHECK_START;
    utils::internal_virtual_call<void, u8, u8, u8, u8>(base_interface->clear_colour_4ub_index, this, r, g, b, a);
    STACK_CHECK_END;
  }

  void clear_buffers(bool clear_colour, bool clear_depth, bool clear_stencil){
    STACK_CHECK_START;
    utils::internal_virtual_call<void, bool, bool, bool>(base_interface->clear_buffers_index, this, clear_colour, clear_depth, clear_stencil);
    STACK_CHECK_END;
  }

  void draw_screen_space_rectangle(c_material* material, i32 dest_x, i32 dest_y, i32 width, i32 height, float src_texture_x0, float src_texture_y0, float src_texture_x1, float src_texture_y1, i32 src_texture_width, i32 src_texture_height, void* client_renderable = nullptr, i32 xdice = 1, i32 ydice = 1){
    assert(material != nullptr);

    STACK_CHECK_START;
    utils::internal_virtual_call<void, c_material*, i32, i32, i32, i32, float, float, float, float, i32, i32, void*, i32, i32>(base_interface->draw_screen_space_rectangle_index, this, material, dest_x, dest_y, width, height, src_texture_x0, src_texture_y0, src_texture_x1, src_texture_y1, src_texture_width, src_texture_height, client_renderable, xdice, ydice);
    STACK_CHECK_END;
  }
};

class c_material_var{
public:
  ALWAYSINLINE void set_float_value(float value){
    STACK_CHECK_START;
    utils::internal_virtual_call<void, float>(base_interface->mv_set_float_value_index, this, value);
    STACK_CHECK_END;
  }

  ALWAYSINLINE void set_int_value(i32 value){
    STACK_CHECK_START;
    utils::internal_virtual_call<void, i32>(base_interface->mv_set_int_value_index, this, value);
    STACK_CHECK_END;
  }

  ALWAYSINLINE void set_string_value(std::string value){
    STACK_CHECK_START;
    utils::internal_virtual_call<void, const i8*>(base_interface->mv_set_string_value_index, this, value.c_str());
    STACK_CHECK_END;
  }

  ALWAYSINLINE i8* get_string_value(){
    STACK_CHECK_START;
    auto r = utils::internal_virtual_call<i8*>(base_interface->mv_get_string_value_index, this);
    STACK_CHECK_END;
    return r;
  }

  ALWAYSINLINE void set_2d(float x, float y){
    STACK_CHECK_START;
    utils::internal_virtual_call<void, float, float>(base_interface->mv_set_2d_index, this, x, y);
    STACK_CHECK_END;
  }

  ALWAYSINLINE void set_3d(vec3 v){
    STACK_CHECK_START;
    utils::internal_virtual_call<void, float, float, float>(base_interface->mv_set_3d_index, this, v.x, v.y, v.z);
    STACK_CHECK_END;
  }

  ALWAYSINLINE void set_4d(vec4 v){
    STACK_CHECK_START;
    utils::internal_virtual_call<void, float, float, float, float>(base_interface->mv_set_4d_index, this, v.x, v.y, v.z, v.w);
    STACK_CHECK_END;
  }
};

class c_material{
public:
  i8* get_name(){
    STACK_CHECK_START;
    auto r = utils::internal_virtual_call<i8*>(base_interface->mat_get_name_index, this);
    STACK_CHECK_END;
    return r;
  }

  i8* get_texture_group_name(){
    STACK_CHECK_START;
    auto r = utils::internal_virtual_call<i8*>(base_interface->get_texture_group_name_index, this);
    STACK_CHECK_END;
    return r;
  }

  c_material_var* find_var(std::string name){
    STACK_CHECK_START;
    bool filler = false;
    auto r = utils::internal_virtual_call<c_material_var*, const i8*, bool*, bool>(base_interface->mat_find_var_index, this, name.c_str(), &filler, false);
    STACK_CHECK_END;
    return r;
  }

  void increment_reference_count(){
    STACK_CHECK_START;
    utils::internal_virtual_call<void>(base_interface->mat_increment_reference_count_index, this);
    STACK_CHECK_END;
  }

  void alpha_modulate(float alpha){
    STACK_CHECK_START;
    utils::internal_virtual_call<void, float>(base_interface->mat_alpha_modulate_index, this, alpha);
    STACK_CHECK_END;
  }

  void colour_modulate(colour clr){
    STACK_CHECK_START;
    utils::internal_virtual_call<void, float, float, float>(base_interface->mat_colour_modulate_index, this, (float)clr.r, (float)clr.g, (float)clr.b);
    STACK_CHECK_END;
  }

  void set_material_var_flag(material_flags flag, bool on){
    STACK_CHECK_START;
    utils::internal_virtual_call<void, material_flags, bool>(base_interface->set_material_var_flag_index, this, flag, on);
    STACK_CHECK_END;
  }

  bool get_material_var_flag(material_flags flag){
    STACK_CHECK_START;
    auto r = utils::internal_virtual_call<bool, material_flags>(base_interface->get_material_var_flag_index, this, flag);
    STACK_CHECK_END;
    return r;
  }

  bool is_error_material(){
    STACK_CHECK_START;
    auto r = utils::internal_virtual_call<bool>(base_interface->is_error_material_index, this);
    STACK_CHECK_END;
    return r;
  }

  float get_alpha_modulate(){
    STACK_CHECK_START;
    auto r = utils::internal_virtual_call<float>(base_interface->get_alpha_modulate_index, this);
    STACK_CHECK_END;
    return r;
  }
};

class c_interface_material_system{
public:
  c_material* create_material(i8* material_name, void* keyvalues){
    STACK_CHECK_START;
    auto r = utils::internal_virtual_call<c_material*, i8*, void*>(base_interface->create_material_index, this, material_name, keyvalues);
    STACK_CHECK_END;
    return r;
  }

  c_material* find_material(i8* material_name, i8* texture_group_name, bool complain = false, i8* complain_prefix = nullptr){
    STACK_CHECK_START;
    auto r = utils::internal_virtual_call<c_material*, i8*, i8*, bool, i8*>(base_interface->find_material_index, this, material_name, texture_group_name, complain, complain_prefix);
    STACK_CHECK_END;
    return r;
  }

  c_texture* find_texture(i8* texture_name, i8* texture_group_name, bool complain = true, i32 additional_creation_flags = 0){
    STACK_CHECK_START;
    auto r = utils::internal_virtual_call<c_texture*, i8*, i8*, bool, i32>(base_interface->find_texture_index, this, texture_name, texture_group_name, complain, additional_creation_flags);
    STACK_CHECK_END;
    return r;
  }

  c_texture* create_named_render_target_texture_ex(i8* name, i32 w, i32 h, render_target_size_mode size_mode, image_format format, material_render_target_depth depth, i32 texture_flags, i32 render_target_flags){
    #if defined(SOURCE_2018)
      assert(false && "We have issues freeing textures during a d3d9 reset in games like l4d2, utilize an existing texture!");
      return nullptr;
    #endif
    STACK_CHECK_START;
    auto r = utils::internal_virtual_call<c_texture*, i8*, i32, i32, render_target_size_mode, image_format, material_render_target_depth, i32, i32>(base_interface->create_named_render_target_texture_ex_index, this, name, w, h, size_mode, format, depth, texture_flags, render_target_flags);
    STACK_CHECK_END;
    return r;
  }

  c_render_context* get_render_context(){
    STACK_CHECK_START;
    auto r = utils::internal_virtual_call<c_render_context*>(base_interface->get_render_context_index, this);
    STACK_CHECK_END;
    return r;
  }

  i32 get_back_buffer_format(){
    STACK_CHECK_START;
    auto r = utils::internal_virtual_call<i32>(base_interface->get_back_buffer_format_index, this);
    STACK_CHECK_END;
    return r;
  }

  //ALWAYSINLINE void begin_render_target_allocation(){
  //  *(i32*)((uptr)this + (uptr)0x2A94) = 0;
  //  utils::virtual_call<84, void>(this);
  //}
//
  //ALWAYSINLINE void end_render_target_allocation(){
  //  utils::virtual_call<85, void>(this);
  //}
};