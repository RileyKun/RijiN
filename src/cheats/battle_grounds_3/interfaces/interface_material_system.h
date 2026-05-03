#pragma once

class c_texture{
public:
  i32 get_actual_width(){
    return utils::virtual_call<3, i32>(this);
  }

  i32 get_actual_height(){
    return utils::virtual_call<4, i32>(this);
  }

  void increment_reference_count(){
    utils::virtual_call<10, void>(this);
  }
};

class c_material;
class c_render_context{
public:
  void depth_range(float z_near, float z_far){
    utils::virtual_call<11, float, float>(this, z_near, z_far);
  }

  void set_stencil_enable(bool enable){
    utils::virtual_call<117, void, bool>(this, enable);
  }

  void set_stencil_fail_operation(shader_stencil_op op){
    utils::virtual_call<118, void, shader_stencil_op>(this, op);
  }

  void set_stencil_zfail_operation(shader_stencil_op op){
    utils::virtual_call<119, void, shader_stencil_op>(this, op);
  }

  void set_stencil_pass_operation(shader_stencil_op op){
    utils::virtual_call<120, void, shader_stencil_op>(this, op);
  }

  void set_stencil_compare_function(stencil_comparison_function cmpfn){
    utils::virtual_call<121, void, stencil_comparison_function>(this, cmpfn);
  }

  void set_stencil_reference_value(i32 ref){
    utils::virtual_call<122, void, i32>(this, ref);
  }

  void set_stencil_test_mask(u32 mask){
    utils::virtual_call<123, void, u32>(this, mask);
  }

  void set_stencil_write_mask(u32 mask){
    utils::virtual_call<124, void, u32>(this, mask);
  }

  // You can see r_flashlightclip to find all of these.
  // Use CShadowMgr::SetStencilAndScissor for ref
  void set_stencil_state(s_stencil_state* stencil_state){
    assert(stencil_state != nullptr);

    set_stencil_enable(stencil_state->enable);
    set_stencil_fail_operation(stencil_state->fail_op);
    set_stencil_zfail_operation(stencil_state->zfail_op);
    set_stencil_pass_operation(stencil_state->pass_op);
    set_stencil_compare_function(stencil_state->compare_func);
    set_stencil_reference_value(stencil_state->reference_value);
    set_stencil_test_mask(stencil_state->test_mask);
    set_stencil_write_mask(stencil_state->write_mask);
  }

  // Find "CDebugViewRender::Draw2DDebuggingInfo" by using
  // debug/debugcamerarendertarget or mat_showframebuffertexture
  // Use the function above as ref in finding the push&pop function calls here.
  void push_render_target_and_viewport(){
    utils::virtual_call<108, void>(this);
  }

  void pop_render_target_and_viewport(){
    utils::virtual_call<109, void>(this);
  }

  void set_render_target(c_texture* texture){
    utils::virtual_call<6, void, c_texture*>(this, texture);
  }

  void viewport(i32 x, i32 y, i32 w, i32 h){
    utils::virtual_call<38, void, i32, i32, i32, i32>(this, x, y, w, h);
  }

  void clear_colour_4ub(u8 r, u8 g, u8 b, u8 a){
    utils::virtual_call<73, void, u8, u8, u8, u8>(this, r, g, b, a);
  }

  void clear_buffers(bool clear_colour, bool clear_depth, bool clear_stencil){
    utils::virtual_call<12, void, bool, bool, bool>(this, clear_colour, clear_depth, clear_stencil);
  }

  void draw_screen_space_rectangle(c_material* material, i32 dest_x, i32 dest_y, i32 width, i32 height, float src_texture_x0, float src_texture_y0, float src_texture_x1, float src_texture_y1, i32 src_texture_width, i32 src_texture_height, void* client_renderable = nullptr, i32 xdice = 1, i32 ydice = 1){
    utils::virtual_call<103, void, c_material*, i32, i32, i32, i32, float, float, float, float, i32, i32, void*, i32, i32>(this, material, dest_x, dest_y, width, height, src_texture_x0, src_texture_y0, src_texture_x1, src_texture_y1, src_texture_width, src_texture_height, client_renderable, xdice, ydice);
  }
};

class c_material{
public:
  i8* get_name(){
    return utils::virtual_call<0, i8*>(this);
  }

  i8* get_texture_group_name(){
    return utils::virtual_call<1, i8*>(this);
  }

  // You can use "voice/icntlk_pl" in client.dll to find the vtable index of this.
  // Also you can find the FindMaterial vtable index.
  void increment_reference_count(){
    utils::virtual_call<12, void>(this);
  }

  void alpha_modulate(float alpha){
    utils::virtual_call<27, void, float>(this, alpha);
  }

  // You can use "decals/rendermodelshadow" in client.dll to find the vtable index of this.
  void colour_modulate(colour clr){
    utils::virtual_call<28, void, float, float, float>(this, (float)clr.r, (float)clr.g, (float)clr.b);
  }

  void set_material_var_flag(material_flags flag, bool on){
    utils::virtual_call<29, void, material_flags, bool>(this, flag, on);
  }

  bool get_material_var_flag(material_flags flag){
    return utils::virtual_call<30, bool, material_flags>(this, flag);
  }

  bool is_error_material(){
    return utils::virtual_call<42, bool>(this);
  }

  float get_alpha_modulate(){
    return utils::virtual_call<44, float>(this);
  }
};

class c_interface_material_system{
public:
  c_material* create_material(i8* material_name, void* keyvalues){
    return utils::virtual_call<70, c_material*, i8*, void*>(this, material_name, keyvalues);
  }

  c_material* find_material(i8* material_name, i8* texture_group_name, bool complain = false, i8* complain_prefix = nullptr){
    return utils::virtual_call<71, c_material*, i8*, i8*, bool, i8*>(this, material_name, texture_group_name, complain, complain_prefix);
  }

  c_texture* find_texture(i8* texture_name, i8* texture_group_name, bool complain = true, i32 additional_creation_flags = 0){
    return utils::virtual_call<79, c_texture*, i8*, i8*, bool, i32>(this, texture_name, texture_group_name, complain, additional_creation_flags);
  }

  c_texture* create_named_render_target_texture_ex(i8* name, i32 w, i32 h, render_target_size_mode size_mode, image_format format, material_render_target_depth depth, i32 texture_flags, i32 render_target_flags){
    return utils::virtual_call<85, c_texture*, i8*, i32, i32, render_target_size_mode, image_format, material_render_target_depth, i32, i32>(this, name, w, h, size_mode, format, depth, texture_flags, render_target_flags);
  }

  c_render_context* get_render_context(){
    return utils::virtual_call<98, c_render_context*>(this);
  }
};