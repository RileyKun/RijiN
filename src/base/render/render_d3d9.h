#pragma once
#include "stb_image.h"
// dev test
#if !defined(DBG)
#include "../dbg.h"
#endif

#define RENDER_SETUP() assert(device != nullptr); if(render == nullptr){ render = new c_render_d3d9; } if(render != nullptr) render->setup(device);

#define RENDER_D3D9_MAX_VERTICES 18383

#if defined(DEV_MODE)
//#define D3D9_SCISSOR_DEBUG
#endif

class c_font_d3d9 : public c_font_base{
public:
  c_font_d3d9() {};
  daisy::c_fontwrapper font_data[50];
  bool                 init[50];
  std::string          font_family;
  u32                  font_weight;
  u32                  font_quality;

  vec3i get_size(u32 font_size, std::wstring wstr, bool* success) override;
  CFLAG_Ofast vec3i draw(vec3i pos, u32 font_size, std::wstring wstr, colour col, u8 style, u8 align_x, u8 align_y) override;

  bool valid_font(u32 size){
    if(size == 0 || size >= 50){
      DBG("[-] c_font_d3d9::valid_font illegal font size of %i\n", size);
      return false;
    }

    return init[size];
  }
  
  // Need to be very careful with how many fonts we create.
  void needs_created(u32 size){
    if(size == 0 || size >= 50){
      DBG("[-] c_font_d3d9::needs_created illegal font size of %i\n", size);
      return; 
    }

    if(size == 0 || init[size])
      return;

    if(!font_data[size].create(font_family.c_str(), size, font_quality, font_weight)){
      assert(false && " failed to create font runtime");
      return;
    }

    DBG("\n\n[!] QUICK INIT FONT\nFamily: %s\nWeight: %i, Quality: %i, Size: %i\n\n\n", font_family.c_str(), font_weight, font_quality, size);
    init[size] = true;
  }
};

class c_image_data{
public:
  u32 hash;
  i32 w, h;
  i32 coords;
  std::vector<u8> data;
};

class c_render_d3d9 : public c_render_base{
public:
  IDirect3DDevice9Ex*               device;
  IDirect3DStateBlock9*             state_block;
  std::vector<RECT>                 scissor_stack;
  std::vector<c_font_d3d9*>         fonts;
  daisy::c_renderqueue              queue;
  daisy::c_texatlas                 atlas;
  std::vector<daisy::c_daisy_resettable_object*> g_daisy_objects;
  bool                              needs_to_wait;
  bool                              is_device_resetting;
  bool                              device_reset;
  i32                               coords_count;

  std::vector<c_image_data*>        image_data_list;

  c_render_d3d9() {};

  render_mode mode(){
    return RENDER_MODE_D3D9;
  }

  bool setup(void* new_device){
    if(is_setup)
      return true;

    state_block  = nullptr;
    coords_count = 0;

    // Setup the device.
    {
      device = (IDirect3DDevice9Ex*)new_device;
      assert(device != nullptr);
    }

    daisy::daisy_initialize(device);

    if(!queue.create()){
      assert(false && "unable to setup queue");
      return false;
    }

    const float size = 128.f;
    if(!atlas.create({size, size})){
      assert(false && "unable to setup atlas");
      return false;
    }

    g_daisy_objects.push_back(&queue);
    g_daisy_objects.push_back(&atlas);

    is_setup = true;
    return true;
  }

  // Heads up, it needs to be the actual image file, not the pixel data. Like, literally the PNG file in a byte as an example.
  bool add_image(u32 name, u8* data, u32 data_size){
    if(data == nullptr)
      return false;

    if(!image_data_list.empty()){
      for(u32 i = 0; i < image_data_list.size(); i++){
        if(image_data_list[i]->hash == name){
          DBG("[-] c_render_d3d9::add_image %X %p dupe entry!\n", name, data);
          return false;
        }
      }
    }

    i32 channel = 0;
    i32 w       = 0;
    i32 h       = 0;

    stbi_set_flip_vertically_on_load(false);
    u8* text_data = stbi_load_from_memory(data, data_size, &w, &h, &channel, 4);
    if(text_data == nullptr){
      assert(false && "Failed to stbi_load_from_memory failed!");
      return false;
    }

    c_image_data* image = new c_image_data();

    // i dunno if this is actually needed or not.
    image->data.resize(w * h * 4);
    memcpy(image->data.data(), text_data, w * h * 4);
    stbi_image_free(text_data);

    if(image->data.empty()){
      free(image);
      assert(false && "No data was transferred to std::vector!");
      return false;
    }

    image->coords = coords_count;
    image->w      = w;
    image->h      = h;
    image->hash   = name;

    atlas.append(coords_count, {(float)image->w, (float)image->h}, image->data.data(), image->data.size());

    coords_count++;
    image_data_list.push_back(image);

    DBG("[+] Assigned image: %i - %X\n", image->coords, name);
    return true;
  }

  c_image_data* get_image(u32 name){
    if(image_data_list.empty()){
      assert(false && "Wow no images found");
      return nullptr;
    }

    for(u32 i = 0; i < image_data_list.size(); i++){
      if(image_data_list[i]->hash == name)
        return image_data_list[i];
    }

    assert(false && "Did not find valid image");
    return nullptr;
  }

  void setup_states(){
    // texture related
    device->SetPixelShader(nullptr);
    device->SetVertexShader(nullptr);
    device->SetTexture(0, nullptr);

    device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
    device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, false);
    device->SetRenderState(D3DRS_SRGBWRITEENABLE, false); // must be reset on end!
    device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    device->SetRenderState(D3DRS_LIGHTING, false);
    device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    device->SetRenderState(D3DRS_ALPHABLENDENABLE, true );
    device->SetRenderState(D3DRS_SCISSORTESTENABLE, true );
    device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    device->SetRenderState(D3DRS_SPECULARENABLE, false);

    // Copied from IMGUI
    device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);

    // This seemed to be the cause of the LOD issue with the fonts.
    device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

    // If its not D3DSHADE_GOURAUD there will be flat unblended shapes everywhere.
    device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);

    // If the game sets this to anything that isn't D3DFILL_SOLID it'll make it wireframed or in tiny points.
    device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

    // colour related
    device->SetRenderState(D3DRS_COLORWRITEENABLE, 0xFFFFFFFF);
    device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

    device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

    daisy::daisy_prepare();
  }

  CFLAG_O0 IDirect3DStateBlock9* begin(){
    if(device == nullptr || !is_setup){
      DBG("[-] device is nullptr or not setup.\n");
      assert(false && "device is nullptr or not setup");
      return nullptr;
    }

    if(device->TestCooperativeLevel() != D3D_OK){
      DBG("[!] c_old_render_d3d9::begin: TestCooperativeLevel() != D3D_OK, make sure data is handled during OnLostDevice/OnResetDevice!!");
      return nullptr;
    }

    if(state_block == nullptr)
      device->CreateStateBlock(D3DSBT_ALL, &state_block);

    if(state_block == nullptr){
      DBG("[-] failed to create state_block\n");
      assert(false && "state_block couldn't be created\n");
      return nullptr;
    }

    alpha_mod_frac = 1.f;
    scissor_stack.clear();

    state_block->Capture();

    // Setup our render states incase do any of our own magic.
    setup_states();

    return state_block;
  }

  ALWAYSINLINE void flush_to_gpu(){
    queue.flush();
    queue.clear();
  }

  void end(IDirect3DStateBlock9* state_block){

    // Render stuff..
    flush_to_gpu();

    if(state_block == nullptr){
      assert(false && "Uh, no state_block\n");
      return;
    }

    if(device != nullptr)
      device->SetRenderState(D3DRS_SRGBWRITEENABLE, true);

    state_block->Apply();
  }

  void unload(bool reset_device = false){
    if(!reset_device)
      device = nullptr;

    if(state_block != nullptr){
      state_block->Release();
      state_block = nullptr;
    }

    if(!reset_device){
      for(auto i : g_daisy_objects){
        i->reset(true);
      }
    }

    if(!reset_device){
      flush_to_gpu();

      for(u32 i = 0; i < fonts.size(); i++){
        for(u32 j = 0; j < 50; j++){
          if(!fonts[i]->init[j])
            continue;
          fonts[i]->font_data[j].erase();
        }
      }
    }
  }

  void on_device_lost(){
    DBG("\non_device_lost\n");

    for(auto i : g_daisy_objects){
      i->reset(true);
    }

    unload(true);
  }

  void on_device_reset(){
    DBG("\non_device_reset\n");

    for(auto i : g_daisy_objects){
      i->reset(false);
    }

  }

  void filled_rect(vec3i pos, vec3i size, colour col, float rounding = 0.f, u32 rounding_flags = 0) override{
    col.a *= alpha_mod_frac;
    col = math::clamp(col);

    rounding       = 0.f;
    rounding_flags = 0;

    queue.push_filled_rectangle({pos.x, pos.y}, {size.x, size.y}, {col.r, col.g, col.b, col.a});
  }
  
  void solid_line(vec3i pos1, vec3i pos2, colour col, bool anti_alias = false, float width = 1.f) override{
    col.a *= alpha_mod_frac;
    col    = math::clamp(col);
    width  = math::biggest(1.f, width);

    queue.push_line({pos1.x, pos1.y}, {pos2.x, pos2.y}, {col.r, col.g, col.b, col.a}, width);
  }

  void outlined_rect(vec3i pos, vec3i size, colour col, float rounding = 0.f, u32 rounding_flags = 0) override{
    col.a *= alpha_mod_frac;
    col    = math::clamp(col);
    size  -= vec3i(1, 1);

    solid_line(vec3i(pos.x, pos.y), vec3i(pos.x + size.x, pos.y), col, false, 0.f);  // Top edge
    solid_line(vec3i(pos.x, pos.y), vec3i(pos.x, pos.y + size.y), col, false, 0.f);  // Left edge
    solid_line(vec3i(pos.x, pos.y + size.y), vec3i(pos.x + size.x + 1, pos.y + size.y), col, false, 0.f);  // Bottom edge
    solid_line(vec3i(pos.x + size.x, pos.y), vec3i(pos.x + size.x, pos.y + size.y), col, false, 0.f);  // Right edge
  }

  void gradient_rect(vec3i pos, vec3i size, colour top_left, colour top_right, colour bottom_left, colour bottom_right, float rounding = 0.f) override{
    queue.push_gradient_rectangle({pos.x, pos.y}, {size.x, size.y}, 
      {top_left.r, top_left.g, top_left.b, top_left.a},
      {top_right.r, top_right.g, top_right.b, top_right.a},
      {bottom_left.r, bottom_left.g, bottom_left.b, bottom_left.a},
      {bottom_right.r, bottom_right.g, bottom_right.b, bottom_right.a});
  }

  void filled_circle(vec3i pos, float radius, float sides, colour col) override{
    if(radius <= 0.0f || sides <= 0.0f){
      DBG("[!] d3d9::filled_circle: needs a radius or sides value above zero!\n");
      return;
    }
    col.a *= alpha_mod_frac;
    col    = math::clamp(col);

    queue.push_filled_circle({pos.x, pos.y}, radius, (i32)sides, {col.r, col.g, col.b, col.a}, {col.r, col.g, col.b, col.a});
  }

  void outlined_circle(vec3i pos, float radius, float sides, colour col) override{
    if(radius <= 0.0f || sides <= 0.0f){
      DBG("[!] d3d9::outlined_circle: needs a radius or sides value above zero!\n");
      return;
    }

    col.a *= alpha_mod_frac;
    col    = math::clamp(col);

    const float pi2  = math::pi2();
    const float step = (pi2 / sides);


    for(float i = 0.f; i < pi2; i += step){
      float pred_i = math::smallest(pi2, i + step);
      if(pred_i == i)
        break;

      vec3i p1 = vec3i(pos.x + (math::cos(i) * radius), pos.y + (math::sin(i) * radius));
      vec3i p2 = vec3i(pos.x + (math::cos(pred_i) * radius), pos.y + (math::sin(pred_i) * radius));
      solid_line(p1, p2, col);
    }
  }

  void draw_image(vec3i pos, vec3i size, colour col, c_image_data* image){
    if(image == nullptr){
      assert(false && "image is nullptr");
      return;
    }

    if(size == vec3i())
      size = vec3i(image->w, image->h);

    auto coords = atlas.coords(image->coords);
    queue.push_filled_rectangle({pos.x, pos.y}, {size.x, size.y}, {col.r, col.g, col.b, col.a}, atlas.texture_handle(), {coords[0], coords[1]}, {coords[2], coords[3]});
  }

  void push_clip_region(vec3i pos, vec3i size) override{
    if(device == nullptr)
      return;

    flush_to_gpu();

    RECT old_rect;
    device->GetScissorRect(&old_rect);
    scissor_stack.push_back(old_rect);

    // Clamp the rectangle clip to the parent rectangle
    // Because if we don't the clip is useless and the next clip we apply wont follow the last one
    RECT rect = {
      math::biggest( pos.x,  old_rect.left),
      math::biggest( pos.y,  old_rect.top),
      math::smallest(pos.x + size.x, old_rect.right),
      math::smallest(pos.y + size.y, old_rect.bottom)
    };

    device->SetScissorRect(&rect);
  }

  void pop_clip_region() override{
    if(device == nullptr || scissor_stack.empty())
      return;

    flush_to_gpu();

    device->SetScissorRect(&scissor_stack.back());
    scissor_stack.pop_back();
  }

  // We can't do the OG alloc 50 fonts, since this creates an atlas per size.
  c_font_base* create_font(std::wstring font_family, u32 font_weight, u32 font_style = D3DRS_ANTIALIASEDLINEENABLE) override{
    return create_font(convert::wstr2str(font_family), 12, font_weight, font_style);
  }

  c_font_base* create_font(std::string font_family, u32 font_height, u32 font_weight, u32 font_style = D3DRS_ANTIALIASEDLINEENABLE){
    if(device == nullptr){
      assert(false && "epic sauce");
      return nullptr;
    }

    assert(font_height >= 1 && font_height <= 50);

    if(font_weight <= FW_MEDIUM)
      font_weight = daisy::FONT_DEFAULT;
    else if(font_weight < FW_BOLD)
      font_weight = daisy::FONT_SEMIBOLD;
    else
      font_weight = daisy::FONT_BOLD;

    c_font_d3d9* font_base = new c_font_d3d9;
    assert(font_base != nullptr);

    font_base->render_base = this;

    if(!font_base->font_data[font_height].create(font_family.c_str(), font_height, font_style, font_weight)){
      assert(false && " failed to create font");
      return nullptr;
    }
    
    g_daisy_objects.push_back(&font_base->font_data[font_height]);

    font_base->font_family  = font_family;
    font_base->font_weight  = font_weight;
    font_base->font_quality = font_style;
    font_base->init[font_height] = true;

    fonts.push_back(font_base);
    register_font_base(font_base);
    return (c_font_base*)font_base;
  }

  // return true to 'cancel' out the d3d9 hook.
  NEVERINLINE bool handle_device_states(void* device, vec3i new_screen_size){
    if(needs_to_wait || is_device_resetting){
      DBG("[+] c_render_d3d9::handle_device_states: waiting for device reset, if this hangs for more a than a few seconds, its something not being handled.\n");
      return true;
    }

    if(!device_reset)
      return false;

    DBG("[+] c_render_d3d9::handle_device_states: re-init.\n");
    screen_size = new_screen_size;
    setup(device);
    if(is_setup)
      device_reset = false;

    return true;
  }
};

extern c_render_d3d9* render;