#pragma once

#define RENDER_SETUP() assert(device != nullptr); if(render == nullptr){ render = new c_render_d3d9; } if(render != nullptr) render->setup(device);

#define RENDER_D3D9_MAX_VERTICES 16668
struct c_vertex{
  vec4      pos;
  D3DCOLOR  colour;
  float u, v;
};

struct s_texture_data{
  u32                id_hash;
  IDirect3DTexture9* texture;
  i32 width;
  i32 height;
  i8* pixels;
};

class c_font_d3d9 : public c_font_base{
public:
  vec3i get_size(u32 font_size, std::wstring wstr, bool* success) override;
  CFLAG_Ofast vec3i draw(vec3i pos, u32 font_size, std::wstring wstr, colour col, u8 style, u8 align_x, u8 align_y) override;
};

class c_render_d3d9 : public c_render_base{
public:
  IDirect3DDevice9Ex*     device                    = nullptr;
  ID3DXLine*              line                      = nullptr;
  std::vector<RECT>       scissor_stack;
  IDirect3DVertexBuffer9* vertex_buffer             = nullptr;
  IDirect3DStateBlock9*   state_block               = nullptr;
  c_vertex*               vertex_list               = nullptr;
  u32                     vertex_count              = 0;
  u32                     current_primitive_type;
  IDirect3DTexture9*      current_texture           = nullptr;
  u32                     current_texture_id        = 0;
  bool                    needs_to_wait             = false;
  bool                    is_device_resetting       = false;
  bool                    device_reset              = false;
  bool                    reset_textures            = false;
  s_texture_data*         rijin_logo                = nullptr;

  std::vector<s_texture_data> texture_list;

  c_render_d3d9(){};

  render_mode mode() override{
    return RENDER_MODE_D3D9;
  }

  // 1.22.2024 - Rud
  // Anything created with the D3DPOOL_DEFAULT has to be released before the Device->Reset() call
  // This game does this call when resizing, or alt-tabbing out and back in.

  // UNLOADING / RELOADING CHEAT:
  // However, even when this is handled properly, something in this HACK doesn't get released properly when unloading.
  // Due to that the device->Reset call will always fail as a result.
  void setup_vertex_buffer(){
    assert(device != nullptr && "the device is nullptr");

    // Release if already exists.
    if(vertex_buffer != nullptr){
      vertex_buffer->Release();
      vertex_buffer = nullptr;
      vertex_list = nullptr;
    }

    HRESULT coop_level = device->TestCooperativeLevel();
    if(coop_level == D3DERR_DEVICELOST || coop_level == D3DERR_DEVICENOTRESET){
      DBG("[-] setup_vertex_buffer: TestCooperativeLevel() is an expected error D3DERR_DEVICELOST/D3DERR_DEVICENOTRESET\nMake sure D3D9::Reset Is hooked and call correct functions!!\n");
      return;
    }

    assert(RENDER_D3D9_MAX_VERTICES % 2 == 0 && "RENDER_D3D9_MAX_VERTICES must be divisable by twos and threes");
    assert(RENDER_D3D9_MAX_VERTICES % 3 == 0 && "RENDER_D3D9_MAX_VERTICES must be divisable by twos and threes");

    HRESULT r = device->CreateVertexBuffer(RENDER_D3D9_MAX_VERTICES * sizeof(c_vertex),
                                           (D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY),
                                           (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1), // FVF flags
                                           D3DPOOL_DEFAULT,
                                           &vertex_buffer, nullptr);

    if(r != D3D_OK){
      DBG("[-] setup_vertex_buffer failed: result: %p, size: %u bytes", RENDER_D3D9_MAX_VERTICES * sizeof(c_vertex));
      return;
    }

    if(vertex_buffer == nullptr){
      DBG("[-] setup_vertex_buffer failed: [device->CreateVertexBuffer was succesful, but returned a null vertex_buffer?\n");
      return;
    }

    // Make sure its null.
    vertex_list = nullptr;

    // lock the vertex buffer and give us a pointer to its memory
    r = vertex_buffer->Lock(0, 0, (void**)&vertex_list, D3DLOCK_DISCARD);
    if(r != D3D_OK){
      DBG("[-] Failed vertex_buffer->Lock (0x%X)\n", r);
      return;
    }
  }

  bool setup(void* new_device){
    if(is_setup)
      return true;

    // set the device
    {
      device = new_device;
      assert(device != nullptr);
    }

    // setup required
    {
      setup_vertex_buffer();
    }

    static bool once = false;
    if(!once){
      add_texture_from_bytes(HASH("RIJIN_LOGO_ICON"), rijin_logo_icon, sizeof(rijin_logo_icon), 512, 512);
      once = true;
    }

    is_setup = true;
    return true;
  }

  ALWAYSINLINE void render_logo(vec3i pos, vec3i size, colour clr){
    render_texture(HASH("RIJIN_LOGO_ICON"), pos, size, clr);
  }

  void strict_no_antialias(bool text = false){
  }

  IDirect3DStateBlock9* begin(){
    if(device == nullptr || !is_setup){
      DBG("[-] c_render_d3d9::begin device is nullptr or not setup.\n");
      return nullptr;
    }

    if(device->TestCooperativeLevel() != D3D_OK){
      DBG("[!] c_render_d3d9::begin: TestCooperativeLevel() != D3D_OK, make sure data is handled during OnLostDevice/OnResetDevice!!\n");
      return nullptr;
    }

    alpha_mod_frac = 1.f;
    scissor_stack.clear();

    if(state_block == nullptr)
      device->CreateStateBlock(D3DSBT_ALL, &state_block);

    if(state_block == nullptr){
      DBG("[-] c_render_d3d9::begin: failed to create state_block\n");
      return nullptr;
    }

    state_block->Capture();

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

    // image related.
    device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
    device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);


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

    device->SetStreamSource(0, vertex_buffer, 0, sizeof(c_vertex));

    return state_block;
  }

  void end(IDirect3DStateBlock9* _state_block){
    flush_to_gpu(current_primitive_type);

    if(state_block == nullptr)
      return;

    if(device != nullptr)
      device->SetRenderState(D3DRS_SRGBWRITEENABLE, true);

    state_block->Apply();
  }

  void unload(bool reset_device = false) override{
    if(!reset_device)
      device    = nullptr;

    line      = nullptr;
    is_setup  = false;

    // free the vertex buffer.
    if(vertex_buffer != nullptr){
      vertex_buffer->Release();
      vertex_buffer = nullptr;
    }

    if(state_block != nullptr){
      state_block->Release();
      state_block = nullptr;
    }

    if(texture_list.size() > 0){
      for(u32 i = 0; i < texture_list.size(); i++){
        if(texture_list[i].texture == nullptr)
          continue;

        texture_list[i].texture->Release();
      }
    }

    texture_list.clear();

    // Free all related fonts
    for(u32 i = 0; i < sizeof(fonts) / sizeof(c_font_base*); i++){
      c_font_base* font_base = fonts[i];

      if(font_base == nullptr)
        continue;

      for(u32 ii = 0; ii < sizeof(font_base->font) / sizeof(void*); ii++){
        ID3DXFont* font = font_base->font[ii];

        if(font == nullptr)
          continue;

        font->OnResetDevice(); // call this to trick the GPU into flushing all references
        font->Release();
        font_base->font[ii] = nullptr;
      }

      delete fonts[i];
      fonts[i] = nullptr;
    }
  }

  void on_device_reset(){
    DBG("[!] c_render_d3d9::on_device_reset\n");
    is_setup = false;

    // Required for the ResetEx function to work.
    if(vertex_buffer != nullptr){
      vertex_buffer->Release();
      vertex_buffer = nullptr;
    }

    if(state_block != nullptr){
      state_block->Release();
      state_block = nullptr;
    }

    for(u32 i = 0; i < texture_list.size(); i++){
      if(texture_list[i].texture != nullptr){
        texture_list[i].texture->Release();
        texture_list[i].texture = nullptr;
      }
    }

    for(u32 i = 0; i < sizeof(fonts) / sizeof(c_font_base*); i++){
      c_font_base* font_base = fonts[i];

      if(font_base == nullptr)
        continue;

      for(u32 ii = 0; ii < sizeof(font_base->font) / sizeof(void*); ii++){
        ID3DXFont* font = font_base->font[ii];

        if(font == nullptr)
          continue;

        font->OnResetDevice();
      }
    }
  }

  void on_device_lost(){
    DBG("[!] c_render_d3d9::on_device_lost\n");
    is_setup = false;

    // Required for the ResetEx function to work.
    if(vertex_buffer != nullptr){
      vertex_buffer->Release();
      vertex_buffer = nullptr;
    }

    if(state_block != nullptr){
      state_block->Release();
      state_block = nullptr;
    }

    for(u32 i = 0; i < sizeof(fonts) / sizeof(c_font_base*); i++){
      c_font_base* font_base = fonts[i];

      if(font_base == nullptr)
        continue;

      for(u32 ii = 0; ii < sizeof(font_base->font) / sizeof(void*); ii++){
        ID3DXFont* font = font_base->font[ii];

        if(font == nullptr)
          continue;

        font->OnLostDevice();
      }
    }
  }

  void post_reset_textures_init(){
    if(is_device_resetting)
      return;


    // We gotta do this to fix our textures.
    for (auto& texture_data : texture_list) {
      DBG("[!] Recreating texture: %X\n", texture_data.id_hash);
      HRESULT result = device->CreateTexture(
          texture_data.width,
          texture_data.height,
          1,
          D3DUSAGE_DYNAMIC,
          D3DFMT_A8R8G8B8,
          D3DPOOL_DEFAULT,
          &texture_data.texture,
          NULL
      );

      if (FAILED(result)) {
        assert(false && "uh oh, we failed to recreate a texture!!! on_device_lost\n");
        continue;
      }

      // Now copy the pixel data back into the texture
      D3DLOCKED_RECT locked_rect;
      if (SUCCEEDED(texture_data.texture->LockRect(0, &locked_rect, NULL, D3DLOCK_DISCARD))) {
        for (int y = 0; y < texture_data.height; ++y)
          memcpy((BYTE*)locked_rect.pBits + y * locked_rect.Pitch, &texture_data.pixels[y * texture_data.width * 4], texture_data.width * 4);

        texture_data.texture->UnlockRect(0);
      }
      else{
        assert(false && "uh oh, we failed to copy bytes over to a texture!!! on_device_lost\n");
      }
    }
  }

  D3DCOLOR convert_rgba_to_bgra(D3DCOLOR rgba_color) {
    i8 r = (rgba_color >> 24) & 0xFF;
    i8 g = (rgba_color >> 16) & 0xFF;
    i8 b = (rgba_color >> 8)  & 0xFF;
    i8 a = rgba_color & 0xFF;

    // Swap R and B to convert to BGRA
    D3DCOLOR bgraColor = (a << 24) | (r << 16) | (g << 8) | b;
    return bgraColor;
  }

  void convert_image_data_rgba_to_bgra(i8* image_data, i32 width, i32 height) {
    i32 num_pixels = width * height;
    for (i32 i = 0; i < num_pixels; ++i) {
      i8* pixel = image_data + i * 4;

      i8 r = pixel[0];
      pixel[0] = pixel[2];
      pixel[2] = r;
    }
  }

  // Only pass PNG image data.
  // This should only be called once.
  bool add_texture_from_bytes(u32 hash, i8* bytes, u32 size, i32 width, i32 height){
    if(bytes == nullptr || size == 0){
      assert(false && "add_texture_from_bytes: bytes or size is invalid");
      return false;
    }

    if(width <= 0 || height <= 0){
      assert(false && "add_texture_from_bytes: width and height are invalid values.");
      return false;
    }

    // It can be anything as long as the width and height is equal to each other.
    /*
        12x12
        64x64
        43x43
        57x57
        512x512
        1024x1024

        cannot be

        1x12
        513x516
        193x121
    */
    if(width != height){
      assert(false && "add_texture_from_bytes: width & height need to be equal.");
      return false;
    }

    // You must enter the correct width and height of the PNG image.
    i32 expected_size = (width * height) * 4;
    if(expected_size != size){
      assert(false && "add_texture_from_bytes: The width and height you provided does not match the size of the image.");
    }

    // D3D9 devs decided to smoke crack and use some chinese color scheme.
    convert_image_data_rgba_to_bgra(bytes, width, height);

    IDirect3DTexture9* texture = nullptr;

    if(FAILED(device->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture, NULL))){
      DBG("[-] failed to create texture: bytes: 0x%p, %ix%i\n", bytes, width, height);
      return false;
    }

    assert(texture != nullptr);

    // Lock the texture to copy data
    D3DLOCKED_RECT locked_rect;
    if(SUCCEEDED(texture->LockRect(0, &locked_rect, NULL, D3DLOCK_DISCARD))) {
      u8* dst = (u8*)locked_rect.pBits;
      u8* src = (u8*)bytes;
      for(i32 y = 0; y < height; ++y)
        memcpy(dst + y * locked_rect.Pitch, src + y * width * 4, width * 4);

      texture->UnlockRect(0);
    }
    else{
      DBG("[-] failed to image bits to d3d9 texture\n");
      return false;
    }


    DBG("[!] Pushed texture to RijiN Texture list.\n");
    texture_list.push_back({hash, texture, width, height, bytes});
    return true;
  }

  s_texture_data* get_texture(u32 hash){
    if(!hash)
      return nullptr;

    for(i32 i = 0; i < texture_list.size(); i++){
      if(texture_list[i].id_hash == hash)
        return &texture_list[i];
    }

    return nullptr;
  }

  // Can be any width.
  void render_texture(u32 texture_id, vec3i pos, vec3i size, colour color){
    flush_to_gpu(current_primitive_type);

    s_texture_data* data = get_texture(texture_id);
    if(data == nullptr){
      assert(false && "did not find texture");
      return;
    }

    u32 col = color.tohex();

    set_texture(texture_id);
    add_vert({vec4(pos.x, pos.y, 0.0f, 1.0f), col, 0.0f, 0.0f}, D3DPT_TRIANGLESTRIP);
    add_vert({vec4(pos.x + size.x, pos.y, 0.0f, 1.0f), col, 1.0f, 0.0f}, D3DPT_TRIANGLESTRIP);
    add_vert({vec4(pos.x, pos.y + size.y, 0.0f, 1.0f), col, 0.0f, 1.0f}, D3DPT_TRIANGLESTRIP);
    add_vert({vec4(pos.x + size.x, pos.y + size.y, 0.0f, 1.0f), col, 1.0f, 1.0f}, D3DPT_TRIANGLESTRIP);
  }

  void set_manual_texture(u32 texture_id, IDirect3DTexture9* texture, bool dont_flush = false){
    if(current_texture_id != texture_id){
      if(!dont_flush)
        flush_to_gpu(current_primitive_type);

      current_texture_id = texture_id;
      current_texture = texture;
     }
  }

  void set_texture(u32 texture_id) {
    if (current_texture_id != texture_id) {
      flush_to_gpu(current_primitive_type);
      current_texture_id = texture_id;
      current_texture = get_texture(texture_id)->texture;
    }
  }

  CFLAG_Ofast u32 get_primitive_count(u32 primitive_type, u32 vertex_count){
    if(vertex_count == 0)
      return 0;

    if(primitive_type == D3DPT_LINESTRIP)
      return vertex_count - 1;

    // 6 - 2 = 4 (v0v4v1)
    if(primitive_type == D3DPT_TRIANGLEFAN)
      return vertex_count - 2;

    if(primitive_type == D3DPT_POINTLIST)
      return vertex_count;

    if(primitive_type == D3DPT_LINELIST || primitive_type == D3DPT_TRIANGLESTRIP)
      return vertex_count / 2;

    if(primitive_type == D3DPT_TRIANGLELIST)
      return vertex_count / 3;

    return 0;
  }

  void flush_to_gpu(u32 primitive_type){
    if(vertex_count == 0)
      return;

    assert(vertex_buffer != nullptr);
    u32 count = get_primitive_count(primitive_type, vertex_count);

    if(count > 0){
      assert(vertex_buffer->Unlock() == D3D_OK);

      if(current_texture == nullptr)
        device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
      else
        device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);

      device->SetTexture(0, current_texture);
      device->DrawPrimitive(primitive_type, 0, count);

      if(current_texture != nullptr){
        current_texture    = nullptr;
        current_texture_id = 0;
      }

      vertex_count = 0;
      assert(vertex_buffer->Lock(0, 0, (void**)&vertex_list, D3DLOCK_DISCARD) == D3D_OK);
    }
    else
      assert(false);
  }

  void add_vert(c_vertex vert, u32 primitive_type){
    if(vertex_count >= RENDER_D3D9_MAX_VERTICES){
      assert(false && "RENDER_D3D9_MAX_VERTICES exceeded!");
      flush_to_gpu(current_primitive_type);
    }

    if(vertex_count > 0 && current_primitive_type != primitive_type)
      flush_to_gpu(current_primitive_type);

    *vertex_list++ = vert;
    vertex_count++;
    current_primitive_type = primitive_type;
  }

 CFLAG_Ofast void filled_rect(vec3i pos, vec3i size, colour col, float rounding = 0.f, u32 rounding_flags = 0) override {
    col.a *= alpha_mod_frac;
    col = math::clamp(col);

    #if !defined(DEV_MODE)
      rounding = 0.f;
    #endif

    // bug: all of the corners expect the left one are not rendered correctly.
    if(rounding > 0.0f && rounding != 8.f)
      rounding = 8.f;

    u32 hex_colour = col.tohex();
    if(rounding <= 0.f){
      c_vertex verts[6] = {
        {vec4(pos.x, pos.y, 0.f, 1.f), hex_colour},
        {vec4(pos.x + size.x, pos.y, 0.f, 1.f), hex_colour},
        {vec4(pos.x, (pos.y + size.y), 0.f, 1.f), hex_colour},

        {vec4(pos.x + size.x, pos.y, 0.f, 1.f), hex_colour},
        {vec4(pos.x + size.x, pos.y + size.y, 0.f, 1.f), hex_colour},
        {vec4(pos.x, pos.y + size.y, 0.f, 1.f), hex_colour},
      };

      for(u32 i = 0; i < 6; i++)
        add_vert(verts[i], D3DPT_TRIANGLELIST);

      return;
    }

    const i32 segments = 16;  // Number of segments for rounding
    float step = math::pi() * 0.5f / segments;

    // Function to add rounded corners
    auto add_rounded_corner = [&](float center_x, float center_y, float start_angle, u32 ignore_flag, vec3i corner_pos) {
      if(rounding_flags & ignore_flag){
        c_vertex rect_corner[6] = {
          {vec4(corner_pos.x, corner_pos.y, 0.f, 1.f), hex_colour},
          {vec4(corner_pos.x + rounding, corner_pos.y, 0.f, 1.f), hex_colour},
          {vec4(corner_pos.x, corner_pos.y + rounding, 0.f, 1.f), hex_colour},

          {vec4(corner_pos.x + rounding, corner_pos.y, 0.f, 1.f), hex_colour},
          {vec4(corner_pos.x + rounding, corner_pos.y + rounding, 0.f, 1.f), hex_colour},
          {vec4(corner_pos.x, corner_pos.y + rounding, 0.f, 1.f), hex_colour}
        };

        for (int i = 0; i < 6; i++)
          add_vert(rect_corner[i], D3DPT_TRIANGLELIST);

        return;
      }

      c_vertex prev_point = {vec4(center_x + rounding * math::cos(start_angle), center_y + rounding * math::sin(start_angle), 0.f, 1.f), hex_colour};

      for (i32 i = 1; i <= segments; ++i) {
          float theta = start_angle + i * step;
          c_vertex new_point = {vec4(center_x + rounding * math::cos(theta), center_y + rounding * math::sin(theta), 0.f, 1.f), hex_colour};

          add_vert({vec4(center_x, center_y, 0.f, 1.f), hex_colour}, D3DPT_TRIANGLELIST);
          add_vert(prev_point, D3DPT_TRIANGLELIST);
          add_vert(new_point, D3DPT_TRIANGLELIST);

          prev_point = new_point;
      }
    };

    // Render order: Top-left corner -> Top edge -> Top-right corner -> Right edge -> Bottom-right corner -> Bottom edge -> Bottom-left corner -> Left edge

    // Top-left corner
    {
      add_rounded_corner(pos.x + rounding, pos.y + rounding, math::pi(), ROUNDING_IGNORE_TOP_LEFT, vec3i(pos.x, pos.y, 0));
    }

    // Top edge (between corners)
    {
      add_vert({vec4(pos.x + rounding, pos.y, 0.f, 1.f), hex_colour}, D3DPT_TRIANGLELIST);
      add_vert({vec4(pos.x + size.x - rounding, pos.y, 0.f, 1.f), hex_colour}, D3DPT_TRIANGLELIST);
      add_vert({vec4(pos.x + rounding, pos.y + rounding, 0.f, 1.f), hex_colour}, D3DPT_TRIANGLELIST);

      add_vert({vec4(pos.x + size.x - rounding, pos.y, 0.f, 1.f), hex_colour}, D3DPT_TRIANGLELIST);
      add_vert({vec4(pos.x + size.x - rounding, pos.y + rounding, 0.f, 1.f), hex_colour}, D3DPT_TRIANGLELIST);
      add_vert({vec4(pos.x + rounding, pos.y + rounding, 0.f, 1.f), hex_colour}, D3DPT_TRIANGLELIST);
    }

    // Top-right corner
    add_rounded_corner(pos.x + size.x - rounding, pos.y + rounding, 1.5f * math::pi(), ROUNDING_IGNORE_TOP_RIGHT, vec3i(pos.x + size.x - rounding, pos.y, 0));

    // Right edge (between corners)
    // No additional triangles needed because it's just a single straight line

    // Bottom-right corner
    add_rounded_corner(pos.x + size.x - rounding, pos.y + size.y - rounding, 0.f, ROUNDING_IGNORE_BOTTOM_RIGHT, vec3i(pos.x + size.x - rounding, pos.y + size.y - rounding, 0));

    // Bottom edge (between corners)
    {
      add_vert({vec4(pos.x + rounding, pos.y + size.y - rounding, 0.f, 1.f), hex_colour}, D3DPT_TRIANGLELIST);
      add_vert({vec4(pos.x + size.x - rounding, pos.y + size.y - rounding, 0.f, 1.f), hex_colour}, D3DPT_TRIANGLELIST);
      add_vert({vec4(pos.x + rounding, pos.y + size.y, 0.f, 1.f), hex_colour}, D3DPT_TRIANGLELIST);

      add_vert({vec4(pos.x + size.x - rounding, pos.y + size.y - rounding, 0.f, 1.f), hex_colour}, D3DPT_TRIANGLELIST);
      add_vert({vec4(pos.x + size.x - rounding, pos.y + size.y, 0.f, 1.f), hex_colour}, D3DPT_TRIANGLELIST);
      add_vert({vec4(pos.x + rounding, pos.y + size.y, 0.f, 1.f), hex_colour}, D3DPT_TRIANGLELIST);
    }

    // Bottom-left corner
    add_rounded_corner(pos.x + rounding, pos.y + size.y - rounding, 0.5f * math::pi(), ROUNDING_IGNORE_BOTTOM_LEFT, vec3i(pos.x, pos.y + size.y - rounding, 0));

    {
      float width_adjustment = 0.f;
      float height_adjustment = rounding;

      rounding = 0.f;
      c_vertex center_rect[6] = {
        {vec4(pos.x + rounding - width_adjustment, pos.y + rounding + height_adjustment, 0.f, 1.f), hex_colour},
        {vec4(pos.x + size.x - rounding + width_adjustment, pos.y + rounding + height_adjustment, 0.f, 1.f), hex_colour},
        {vec4(pos.x + rounding - width_adjustment, pos.y + size.y - rounding - height_adjustment, 0.f, 1.f), hex_colour},

        {vec4(pos.x + size.x - rounding + width_adjustment, pos.y + rounding + height_adjustment, 0.f, 1.f), hex_colour},
        {vec4(pos.x + size.x - rounding + width_adjustment, pos.y + size.y - rounding - height_adjustment, 0.f, 1.f), hex_colour},
        {vec4(pos.x + rounding - width_adjustment, pos.y + size.y - rounding - height_adjustment, 0.f, 1.f), hex_colour}
      };

      for (int i = 0; i < 6; i++)
        add_vert(center_rect[i], D3DPT_TRIANGLELIST);
    }
  }

  void solid_line(vec3i pos1, vec3i pos2, colour col, bool anti_alias = false, float width = 1.f) override{
    col.a *= alpha_mod_frac;
    col    = math::clamp(col);

    u32 hex_colour = col.tohex();
    add_vert({vec4(pos1.x, pos1.y, 0.f, 1.f), hex_colour}, D3DPT_LINELIST);
    add_vert({vec4(pos2.x, pos2.y, 0.f, 1.f), hex_colour}, D3DPT_LINELIST);
  }

  CFLAG_Ofast void outlined_rect(vec3i pos, vec3i size, colour col, float rounding = 0.f, u32 rounding_flags = 0) override {
    col.a *= alpha_mod_frac;
    col = math::clamp(col);
    size  -= vec3i(1, 1);
    #if !defined(DEV_MODE)
      rounding = 0.f;
    #endif

    if(rounding <= 0.f){
      solid_line(vec3i(pos.x, pos.y), vec3i(pos.x + size.x, pos.y), col, false, 0.f);  // Top edge
      solid_line(vec3i(pos.x, pos.y), vec3i(pos.x, pos.y + size.y), col, false, 0.f);  // Left edge
      solid_line(vec3i(pos.x, pos.y + size.y), vec3i(pos.x + size.x + 1, pos.y + size.y), col, false, 0.f);  // Bottom edge
      solid_line(vec3i(pos.x + size.x, pos.y), vec3i(pos.x + size.x, pos.y + size.y), col, false, 0.f);  // Right edge
      return;
    }

     // bug: all of the corners expect the left one are not rendered correctly.
    if(rounding > 0.0f && rounding != 8.f)
      rounding = 8.f;

    const int segments = 16;  // Number of segments for rounding
    float step = math::pi() * 0.5f / segments;

    // Function to add rounded corners or square corners based on flags
    auto draw_rounded_corner_or_square = [&](float center_x, float center_y, float startAngle, u32 ignore_flag, vec3i corner_pos, vec3i edge_offset) {
      if (rounding_flags & ignore_flag) {
        vec3i square_corner2 = corner_pos + vec3i(rounding, 0, 0);
        vec3i square_corner3 = corner_pos + vec3i(rounding, rounding, 0);
        vec3i square_corner4 = corner_pos + vec3i(0, rounding, 0);

        if(ignore_flag & ROUNDING_IGNORE_TOP_RIGHT || ignore_flag & ROUNDING_IGNORE_TOP_LEFT)
          solid_line(corner_pos, square_corner2, col, false, 0.f);

        if(ignore_flag & ROUNDING_IGNORE_TOP_RIGHT || ignore_flag & ROUNDING_IGNORE_BOTTOM_RIGHT)
          solid_line(square_corner2, square_corner3, col, false, 0.f);

        if(ignore_flag & ROUNDING_IGNORE_BOTTOM_RIGHT || ignore_flag & ROUNDING_IGNORE_BOTTOM_LEFT)
          solid_line(square_corner3, square_corner4, col, false, 0.f);

        if(ignore_flag & ROUNDING_IGNORE_TOP_LEFT || ignore_flag & ROUNDING_IGNORE_BOTTOM_LEFT)
          solid_line(square_corner4, corner_pos, col, false, 0.f);
      }
      else {
        vec3i prev_point = vec3i(center_x + rounding * math::cos(startAngle), center_y + rounding * math::sin(startAngle), 0);
        for (int i = 1; i <= segments; ++i) {
          float theta = startAngle + i * step;
          vec3i new_point = vec3i(center_x + rounding * math::cos(theta), center_y + rounding * math::sin(theta), 0);

          solid_line(prev_point, new_point, col, false, 0.f);
          prev_point = new_point;
        }
      }
    };

    {
      // Top-left corner
      draw_rounded_corner_or_square(
        pos.x + rounding, pos.y + rounding,
        math::pi(), ROUNDING_IGNORE_TOP_LEFT,
        vec3i(pos.x, pos.y, 0),
        vec3i(1, 1, 0)
      );

      // Top-right corner
      draw_rounded_corner_or_square(
        pos.x + size.x - rounding, pos.y + rounding,
        1.5f * math::pi(), ROUNDING_IGNORE_TOP_RIGHT,
        vec3i(pos.x + size.x - rounding, pos.y, 0),
        vec3i(-1, 1, 0)
      );

      // Bottom-right corner
      draw_rounded_corner_or_square(
        pos.x + size.x - rounding, pos.y + size.y - rounding,
        0.f, ROUNDING_IGNORE_BOTTOM_RIGHT,
        vec3i(pos.x + size.x - rounding, pos.y + size.y - rounding, 0),
        vec3i(-1, -1, 0)
      );

      // Bottom-left corner
      draw_rounded_corner_or_square(
        pos.x + rounding, pos.y + size.y - rounding,
        0.5f * math::pi(), ROUNDING_IGNORE_BOTTOM_LEFT,
        vec3i(pos.x, pos.y + size.y - rounding, 0),
        vec3i(1, -1, 0)
      );

      // Fix weird rounding issue with lines.
      rounding -= 1.f;

      solid_line(vec3i(pos.x, pos.y + rounding), vec3i(pos.x, pos.y + size.y - rounding), col, false, 0.f);  // Left edge
      solid_line(vec3i(pos.x + rounding, pos.y), vec3i(pos.x + size.x - rounding, pos.y), col, false, 0.f);  // Top edge
      solid_line(vec3i(pos.x + rounding, pos.y + size.y), vec3i(pos.x + size.x - rounding, pos.y + size.y), col, false, 0.f);  // Bottom edge
      solid_line(vec3i(pos.x + size.x, pos.y + rounding), vec3i(pos.x + size.x, pos.y + size.y - rounding), col, false, 0.f);  // Right edge
    }
  }

  void gradient_rect_linear(vec3i pos, vec3i size, vec3i start, vec3i end, colour from, colour to, float rounding = 0.f) override{
  }

  void gradient_rect(vec3i pos, vec3i size, colour top_left, colour top_right, colour bottom_left, colour bottom_right, float rounding = 0.f) override{
    c_vertex verts[6] = {
      {vec4(pos.x, pos.y, 0.f, 1.f), top_left.tohex()},
      {vec4(pos.x + size.x, pos.y, 0.f, 1.f), top_right.tohex()},
      {vec4(pos.x, pos.y + size.y, 0.f, 1.f), bottom_left.tohex()},

      {vec4(pos.x + size.x, pos.y, 0.f, 1.f), top_right.tohex()},
      {vec4(pos.x + size.x, pos.y + size.y, 0.f, 1.f), bottom_right.tohex()},
      {vec4(pos.x, pos.y + size.y, 0.f, 1.f), bottom_left.tohex()},
    };

    for(u32 i = 0; i < 6; i++)
      add_vert(verts[i], D3DPT_TRIANGLELIST);
  }

  void filled_circle(vec3i pos, float radius, float sides, colour col) override{
    col.a *= alpha_mod_frac;
    col    = math::clamp(col);

    u32  hex_colour = col.tohex();
    vec4 vp;
    for(float i = 0.f; i < math::pi2(); i += math::pi2() / sides){
      vp.x = pos.x + (math::cos(i) * radius);
      vp.y = pos.y + (math::sin(i) * radius);
      add_vert({vp, hex_colour}, D3DPT_TRIANGLEFAN);
    }

    // flush to gpu
    flush_to_gpu(current_primitive_type);
  }

  void outlined_circle(vec3i pos, float radius, float sides, colour col) override{
    col.a *= alpha_mod_frac;
    col    = math::clamp(col);

    u32  hex_colour = col.tohex();
    vec4 vp;
    for(float i = 0.f; i < math::pi2(); i += math::pi2() / sides){
      vp.x = pos.x + (math::cos(i) * radius);
      vp.y = pos.y + (math::sin(i) * radius);
      add_vert({vp, hex_colour}, D3DPT_LINESTRIP);
    }

    // connect the final strip to the start
    vp.x = pos.x + (math::cos(0.f) * radius);
    vp.y = pos.y + (math::sin(0.f) * radius);
    add_vert({vp, hex_colour}, D3DPT_LINESTRIP);

    // flush to gpu
    flush_to_gpu(current_primitive_type);
  }

  void push_clip_region(vec3i pos, vec3i size) override{
    if(device == nullptr)
      return;

    // send the batch off before scissor takes place
    flush_to_gpu(current_primitive_type);

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

    // send the batch off before scissor takes place
    flush_to_gpu(current_primitive_type);

    device->SetScissorRect(&scissor_stack.back());
    scissor_stack.pop_back();
  }

  c_font_base* create_font(std::wstring family_name, u32 font_weight, u32 font_style = ANTIALIASED_QUALITY) override{
    if(device == nullptr){
      DBG("[-] failed to create font device is nullptr\n");
      return nullptr;
    }

    u32 status = 0;

    // Attempt to import d3dx manually if it doesnt exist
    LoadLibraryA(XOR("d3dx9_42.dll"));
    decltype(&D3DXCreateFontW) create_font = (decltype(&D3DXCreateFontW))utils::get_proc_address(utils::get_module_handle(HASH("d3dx9_42.dll")), HASH("D3DXCreateFontW"));

    // Create the font base
    c_font_d3d9* font_base = new c_font_d3d9;
    {
      font_base->render_base    = this;

      for(u32 i = 1; i < sizeof(font_base->font) / sizeof(void*); i++){
        u32 status = create_font(device, i, 0, font_weight, false, false, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, font_style, DEFAULT_PITCH | FF_MODERN, family_name.c_str(), (ID3DXFont**)&font_base->font[i]);

        if(status != 0){
          delete font_base;
          DBG("[-] D3DXCreateFont failed, status: %X\n", status);
          return nullptr;
        }
      }

      // Register it so our rendering system can know we have a font allocated
      register_font_base(font_base);
    }

    return (c_font_base*)font_base;
  }

  c_font_base* create_font(std::string family_name, i32 font_height, u32 font_weight, u32 font_style = ANTIALIASED_QUALITY){
    return create_font(convert::str2wstr(family_name), font_weight, font_style);
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
    post_reset_textures_init();
    if(is_setup)
      device_reset = false;



    return true;
  }
};

extern c_render_d3d9* render;