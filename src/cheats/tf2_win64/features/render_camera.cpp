#include "../link.h"

CLASS_ALLOC(c_render_camera, render_cam);


void c_render_camera::init_material(){
  if(camera_texture == nullptr)
    camera_texture = global->material_system->create_named_render_target_texture_ex(XOR("camera_texture"),
      1, 1,
       RT_SIZE_FULL_FRAME_BUFFER,
        global->material_system->get_back_buffer_format(),
         MATERIAL_RT_DEPTH_SHARED,
         TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT, 1);

  if(camera_material == nullptr){
    c_keyvalues* kv = chams->create_shader_kv(XOR("UnlitGeneric"));
    kv->set_string(XOR("$basetexture"), XOR("camera_texture"));
    camera_material = chams->create_material(XOR("camera_material"), kv);
  }
}