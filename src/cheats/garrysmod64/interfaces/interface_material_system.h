#pragma once

// OK = vtable index verified.
class c_texture{
public:
  i8* get_name(){
    return utils::virtual_call64<0, i8*>(this);
  }

  i32 get_actual_width(){ // OK
    return utils::virtual_call64<3, i32>(this);
  }

  i32 get_actual_height(){ // OK
    return utils::virtual_call64<4, i32>(this);
  }

  bool is_translucent(){
    return utils::virtual_call64<6, bool>(this);
  }

  void increment_reference_count(){
    utils::virtual_call64<10, void>(this);
  }

  bool is_render_target(){
    return utils::virtual_call64<21, bool>(this);
  }

  bool is_temp_render_target(){
    return utils::virtual_call64<22, bool>(this);
  }
};

class c_material;
class c_render_context{
public:
  void begin_render(){
    utils::virtual_call64<0, void>(this);
  }

  void end_render(){
    utils::virtual_call64<1, void>(this);
  }

  void depth_range(float z_near, float z_far){ // OK
    utils::virtual_call64<9, float, float>(this, z_near, z_far);
  }

  void set_stencil_enable(bool enable){ // OK
    utils::virtual_call64<115, void, bool>(this, enable);
  }

  void set_stencil_fail_operation(shader_stencil_op op){ // OK
    utils::virtual_call64<116, void, shader_stencil_op>(this, op);
  }

  void set_stencil_zfail_operation(shader_stencil_op op){ // OK
    utils::virtual_call64<117, void, shader_stencil_op>(this, op);
  }

  void set_stencil_pass_operation(shader_stencil_op op){ // OK
    utils::virtual_call64<118, void, shader_stencil_op>(this, op);
  }

  void set_stencil_compare_function(stencil_comparison_function cmpfn){ // OK
    utils::virtual_call64<119, void, stencil_comparison_function>(this, cmpfn);
  }

  void set_stencil_reference_value(i32 ref){ // OK
    utils::virtual_call64<120, void, i32>(this, ref);
  }

  void set_stencil_test_mask(u32 mask){ // OK
    utils::virtual_call64<121, void, u32>(this, mask);
  }

  void set_stencil_write_mask(u32 mask){ // OK
    utils::virtual_call64<122, void, u32>(this, mask);
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
  void push_render_target_and_viewport(){ // OK
    utils::virtual_call64<106, void>(this);
  }

  void pop_render_target_and_viewport(){ // OK
    utils::virtual_call64<107, void>(this);
  }

  void set_render_target(c_texture* texture){ // OK
    utils::virtual_call64<4, void, c_texture*>(this, texture);
  }

  c_texture* get_render_target(){ // OK?! Idk
    return utils::virtual_call64<5, c_texture*>(this);
  }

  // OK 
  void viewport(i32 x, i32 y, i32 w, i32 h){
    utils::virtual_call64<36, void, i32, i32, i32, i32>(this, x, y, w, h);
  }

  //CopyRenderTargetToTextureEx
  void copy_render_target_to_texture(c_texture* texture){
    utils::virtual_call64<109, void, c_texture*, i64, i64, i64>(this, texture, 0, 0, 0);
  }

  //CopyTextureToRenderTargetEx
  void copy_texture_to_render_target(c_texture* texture, i32 render_target = 0){
    utils::virtual_call64<110, void, i32, c_texture*, void*, void*>(this, render_target, texture, nullptr, nullptr);
  }

  // OK 
  void clear_colour_4ub(u8 r, u8 g, u8 b, u8 a){
    utils::virtual_call64<71, void, u8, u8, u8, u8>(this, r, g, b, a);
  }

  // OK (Is it really), yea it is?
  void clear_buffers(bool clear_colour, bool clear_depth, bool clear_stencil){
    utils::virtual_call64<10, void, bool, bool, bool>(this, clear_colour, clear_depth, clear_stencil);
  }

  // DO NOT USE ME (Just used it to break point into l0l)
  void copy_render_target_to_texture_actual(c_texture* texture, bool error = true){
    assert(error == false);
    utils::virtual_call64<15, void, c_texture*>(this, texture);
  }

  void push_render_target_and_viewport_p5(c_texture* texture, i64 x, i64 y, i64 w, i64 h, bool error = true){
    assert(error == false)
    utils::virtual_call64<104, void, c_texture*, i64, i64, i64, i64>(this, texture, x, y, w, h);
  }

  void push_render_target_and_viewport_p1(c_texture* texture, bool error = true){
    //assert(error == false)
    utils::virtual_call64<105, void, c_texture*>(this, texture);
  }

  // OK
  void draw_screen_space_rectangle(c_material* material, i32 dest_x, i32 dest_y, i32 width, i32 height, float src_texture_x0, float src_texture_y0, float src_texture_x1, float src_texture_y1, i32 src_texture_width, i32 src_texture_height, void* client_renderable = nullptr, i32 xdice = 1, i32 ydice = 1){
    utils::virtual_call64<101, void, c_material*, i32, i32, i32, i32, float, float, float, float, i32, i32, void*, i32, i32>(this, material, dest_x, dest_y, width, height, src_texture_x0, src_texture_y0, src_texture_x1, src_texture_y1, src_texture_width, src_texture_height, client_renderable, xdice, ydice);
  }
};

class c_material_var{
public:
  ALWAYSINLINE void set_float_value(float value){
    utils::virtual_call64<3, void, float>(this, value);
  }

  ALWAYSINLINE void set_int_value(i32 value){
    utils::virtual_call64<4, void, i32>(this, value);
  }

  ALWAYSINLINE void set_string_value(std::string value){
    utils::virtual_call64<5, void, i8*>(this, value.c_str());
  }

  ALWAYSINLINE i8* get_string_value(){
    return utils::virtual_call64<6, i8*>(this);
  }

  ALWAYSINLINE void set_2d(float x, float y){
    utils::virtual_call64<10, void, float, float>(this, x, y);
  }

  ALWAYSINLINE void set_3d(vec3 v){
    utils::virtual_call64<11, void, float, float, float>(this, v.x, v.y, v.z);
  }

  ALWAYSINLINE void set_4d(vec4 v){
    utils::virtual_call64<12, void, float, float, float, float>(this, v.x, v.y, v.z, v.w);
  }

};

class c_material{
public:
  i8* get_name(){
    return utils::virtual_call64<0, i8*>(this);
  }

  i8* get_texture_group_name(){
    return utils::virtual_call64<1, i8*>(this);
  }

  // You can use "voice/icntlk_pl" in client.dll to find the vtable index of this.
  // Also you can find the FindMaterial vtable index.
  void increment_reference_count(){ // OK
    utils::virtual_call64<12, void>(this);
  }

  void alpha_modulate(float alpha){
    utils::virtual_call64<27, void, float>(this, alpha);
  }

  // You can use "decals/rendermodelshadow" in client.dll to find the vtable index of this.
  void colour_modulate(colour clr){
    utils::virtual_call64<28, void, float, float, float>(this, (float)clr.r, (float)clr.g, (float)clr.b);
  }

  void set_material_var_flag(material_flags flag, bool on){
    utils::virtual_call64<29, void, material_flags, bool>(this, flag, on);
  }

  bool get_material_var_flag(material_flags flag){
    return utils::virtual_call64<30, bool, material_flags>(this, flag);
  }

  bool is_error_material(){
    return utils::virtual_call64<42, bool>(this);
  }

  float get_alpha_modulate(){
    return utils::virtual_call64<44, float>(this);
  }

  c_material_var* find_var(std::string name){
    bool filler = false;
    return utils::virtual_call64<11, c_material_var*, i8*, bool*, bool>(this, name.c_str(), &filler, false);
  }
};

class c_interface_material_system{
public:
  // OK
  c_material* create_material(i8* material_name, void* keyvalues){
    return utils::virtual_call64<74, c_material*, i8*, void*>(this, material_name, keyvalues);
  }

  // OK
  c_material* find_material(i8* material_name, i8* texture_group_name, bool complain = false, i8* complain_prefix = nullptr){
    return utils::virtual_call64<75, c_material*, i8*, i8*, bool, i8*>(this, material_name, texture_group_name, complain, complain_prefix);
  }

  // OK
  c_texture* find_texture(i8* texture_name, i8* texture_group_name, bool complain = true, i32 additional_creation_flags = 0){
    return utils::virtual_call64<83, c_texture*, i8*, i8*, bool, i32>(this, texture_name, texture_group_name, complain, additional_creation_flags);
  }

  c_texture* create_named_render_target_texture_ex(i8* name, i32 w, i32 h, render_target_size_mode size_mode, image_format format, material_render_target_depth depth, i32 texture_flags, i32 render_target_flags){
    return utils::virtual_call64<89, c_texture*, i8*, i32, i32, render_target_size_mode, image_format, material_render_target_depth, i32, i32>(this, name, w, h, size_mode, format, depth, texture_flags, render_target_flags);
  }

  c_render_context* get_render_context(){
    return utils::virtual_call64<102, c_render_context*>(this);
  }

  // in client.dll
  /*
      get_back_buffer_format = (*(*a2 + 288))(a2);
      v12 = (*(v10 + 728))(a2, "_rt_Camera", a5, a5, 1, get_back_buffer_format, 0, 0, 1);
  */
  i32 get_back_buffer_format(){
    return utils::virtual_call64<36, i32>(this);
  }
};