#pragma once

/*
struct c_font_glyph{
  wchar_t ch;
  float   u, v;
  float   width, height;
  i32     adv_x;
  i32     x_offset;
};

struct s_unicode_range{
  u32 start, end;
};

#define ATLAS_SIZE 1024
class c_textured_font{
public:
  std::wstring        font_name;
  u32                 font_name_hash;
  u32                 quality_type;
  IDirect3DDevice9Ex* device;
  HDC                 dc;
  HFONT               font;
  BITMAPINFO          bitmap_info;
  HBITMAP             bitmap;
  TEXTMETRIC          tm;

  std::vector<s_unicode_range> unicode_ranges;
  std::vector<c_font_glyph>    glyphs;

  // Fyi this can easily be changed to support other d3d versions.
  LPDIRECT3DTEXTURE9 d3d_texture;
  
  void take_device(IDirect3DDevice9Ex* p){
    device = p;
  }

  void setup(IDirect3DDevice9Ex* p, std::wstring font_family, i32 font_height, i32 font_weight = FW_MEDIUM, u32 quality = ANTIALIASED_QUALITY, bool italic = false){
    assert(font_family.length() > 0)
    memset(this, 0, sizeof(*this));

    take_device(p);
    font_name      = font_family;
    quality_type   = quality;

    wchar_t tmp[1024];
    wsprintfW(tmp, WXOR(L"%ls_%i_%i_%i_%i"), font_family.c_str(), font_height, font_weight, quality, italic);
    font_name_hash = WHASH_RT(tmp);

    unicode_ranges.clear();
    glyphs.clear();

    {
      // ASCII
      unicode_ranges.push_back({0x20, 0x7F});
      //unicode_ranges.push_back({0x00A0, 0x00FF});
      //unicode_ranges.push_back({0x0100, 0x017F});
      //unicode_ranges.push_back({0x0370, 0x03FF});
      //unicode_ranges.push_back({0x0400, 0x04FF});
      //unicode_ranges.push_back({0x3040, 0x309F});
      //unicode_ranges.push_back({0x30A0, 0x30FF});
      //unicode_ranges.push_back({0x4E00, 0x9FFF});
      //unicode_ranges.push_back({0x2000, 0x206F});
      //unicode_ranges.push_back({0x2200, 0x22FF});
      //unicode_ranges.push_back({0x2500, 0x257F});
      //unicode_ranges.push_back({0x1F600, 0x1F64F});
      //unicode_ranges.push_back({0x2700,  0x27BF});
      //unicode_ranges.push_back({0x1F680, 0x1F6FF});
      //unicode_ranges.push_back({0x1F300, 0x1F5FF});
      //unicode_ranges.push_back({0x1F900, 0x1F9FF});
      //unicode_ranges.push_back({0x2190,  0x21FF});
      //unicode_ranges.push_back({0x1F1E6, 0x1F1FF});
      //unicode_ranges.push_back({0x25A0,  0x25FF});
    }

    dc = CreateCompatibleDC(0);
    if(dc == nullptr){
      DBG('[-] c_textured_font::setup: Failed to init DC\n');
      return;
    }

    {
      bitmap_info.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
      bitmap_info.bmiHeader.biWidth       = ATLAS_SIZE;
      bitmap_info.bmiHeader.biHeight      = -ATLAS_SIZE;
      bitmap_info.bmiHeader.biPlanes      = 1;
      bitmap_info.bmiHeader.biBitCount    = 32; 
      bitmap_info.bmiHeader.biCompression = BI_RGB;
    }

    void* bits = nullptr;
    bitmap = CreateDIBSection(dc, &bitmap_info, DIB_RGB_COLORS, &bits, NULL, 0);
    if (bitmap == nullptr) {
      DeleteDC(dc);
      DBG("[-] c_textured_font::setup: Failed to create bitmap\n");
      return;
    }
    
    // Place the bitmap into the memory DC.
    SelectObject(dc, bitmap);

    font = CreateFontW(font_height, 0, 0, 0, font_weight, italic, false, false, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, quality, FIXED_PITCH | FF_DONTCARE, font_family.c_str());
    if(font == nullptr){
      DeleteObject(bitmap);
      DeleteDC(dc);
      DBG("[-] c_textured_font::setup:: Failed to create font!\n");
      return;
    }

    // Dynamically create a texture.
    if(!generate_atlas_gdi())
      return;


    DBG("[!] Created font succesfully!");
    D3DXSaveTextureToFile("d3d_texture_output.png", D3DXIFF_PNG, d3d_texture, NULL);
  }

  void internal_draw_font(std::wstring text, vec3 pos, colour clr){
    render->set_manual_texture(font_name_hash, d3d_texture);

    u32 count = render->get_primitive_count(render->current_primitive_type, render->vertex_count);
    if(render->current_primitive_type != D3DPT_TRIANGLELIST || render->current_primitive_type == D3DPT_TRIANGLELIST && count > 1800){
      render->flush_to_gpu(render->current_primitive_type);
      render->set_manual_texture(font_name_hash, d3d_texture);
    }

    u32  col     = clr.tohex();
    vec3 cursor = vec3(pos.x, pos.y);

    bool high_quality = (quality_type == ANTIALIASED_QUALITY || quality_type == CLEARTYPE_QUALITY);

    const float half_texel      = (0.5f / ATLAS_SIZE);
    const float baseline_offset = (float)tm.tmAscent;
    const float descent_offset  = (float)tm.tmDescent;

    for(u32 i = 0; i < text.length(); i++){
      if(text[i] == L' '){
        cursor.x += high_quality ? (float)tm.tmAveCharWidth / 2 : (float)tm.tmAveCharWidth;
        continue;
      }

      c_font_glyph* glyph = char_to_glyph(text[i]);
      if(glyph == nullptr)
        continue;

      float w = glyph->width;
      float h = high_quality ? (glyph->height + descent_offset) : glyph->height;

      float u0 = (glyph->u + half_texel) / ATLAS_SIZE;
      float v0 = (glyph->v + half_texel) / ATLAS_SIZE;
      float u1 = (glyph->u + glyph->width - half_texel) / ATLAS_SIZE;
      float v1 = (glyph->v + glyph->height - half_texel) / ATLAS_SIZE;

      float x = cursor.x + (high_quality ? (float)glyph->x_offset : 0.f);
      float y = cursor.y - baseline_offset;

      // The actual pixel size of the glyph.
      if(!high_quality){
        w = (float)glyph->adv_x;
        h = glyph->height - (tm.tmDescent / 2);
      }
      else{
        // temp fix for cleartype and anti-aliased fonts.
         w--;
      }

      // Left triangle
      render->add_vert({{x, y, 0.f, 1.f}, col, u0, v0}, D3DPT_TRIANGLELIST); // top left
      render->add_vert({{x + w, y, 0.f, 1.f}, col, u1, v0}, D3DPT_TRIANGLELIST); // top right
      render->add_vert({{x, y + h, 0.f, 1.f}, col, u0, v1}, D3DPT_TRIANGLELIST); // bottom left

      // Right triangle
      render->add_vert({{x + w, y, 0.f, 1.f}, col, u1, v0}, D3DPT_TRIANGLELIST); // top right
      render->add_vert({{x + w, y + h, 0.f, 1.f}, col, u1, v1}, D3DPT_TRIANGLELIST); // bottom right
      render->add_vert({{x, y + h, 0.f, 1.f}, col, u0, v1}, D3DPT_TRIANGLELIST); // bottom left

      assert(glyph->adv_x > 0);
  
      cursor.x += (float)glyph->adv_x + (high_quality ? 1.f : 1.f);
    }
  }

  void draw_text(std::wstring text,vec3 pos, colour col, u8 style = TSTYLE_NONE){
    if(style == TSTYLE_SHADOW){
      internal_draw_font(text, pos + vec3(1.f, 1.f), colour(0,0,0, col.a > 0 ? col.a / 2 : col.a));
    }
    else if(style == TSTYLE_OUTLINE){
      vec3 draw_offsets[] = {
        {pos.x + 1, pos.y},
        {pos.x - 1, pos.y},
        {pos.x, pos.y + 1},
        {pos.x, pos.y - 1},
      };

       for(u8 i = 0; i < sizeof(draw_offsets) / sizeof(vec3); i++)
        internal_draw_font(text,  draw_offsets[i], colour(0,0,0,col.a > 0 ? col.a / 2 : col.a));
    }

    internal_draw_font(text, pos, col);
  }

private:
  bool generate_atlas_gdi(){
    bool high_quality = (quality_type == ANTIALIASED_QUALITY || quality_type == CLEARTYPE_QUALITY);
    i32  pad = 2;

    SelectObject(dc, font);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(255, 255, 255));

    PatBlt(dc, 0, 0, ATLAS_SIZE, ATLAS_SIZE, BLACKNESS);
    GetTextMetrics(dc, &tm);

    i32 x = 0, y = 0;
    i32 max_row_height = 0;

    i32 baseline_offset = tm.tmAscent;
    i32 metric_height   = tm.tmHeight;

    for(u32 j = 0; j < unicode_ranges.size(); j++){
      for(u32 ch = unicode_ranges[j].start; ch <= unicode_ranges[j].end; ch++){
        ABC abc;
        wchar_t str[2] = {(wchar_t)ch, 0};

        assert(GetCharABCWidthsW(dc, ch, ch, &abc) == true);

        i32 width  = abc.abcA + abc.abcB + abc.abcC;
        assert(width > 0);
        if(quality_type == CLEARTYPE_QUALITY)
          width++;

        i32 height = metric_height;

        // If we get close to the end then wrap around again.
        if(x + width + pad > ATLAS_SIZE){
          x = 0;
          y += max_row_height + pad;
          max_row_height = 0;
          if(y + height + pad > ATLAS_SIZE)
            assert(false && "ATLAS OVERFLOW!");
        }

        max_row_height = math::biggest(max_row_height, height);

        i32 render_x = x - abc.abcA;
        i32 render_y = y + (metric_height - baseline_offset);

        assert(render_x >= 0);
        assert(render_y >= 0);

        TextOutW(dc, render_x, render_y, str, 1);
        c_font_glyph g = {
            (wchar_t)ch, 
            high_quality ? (float)x : (float)render_x, 
            high_quality ? (float)y : (float)render_y, 
            (float)width, 
            (float)height, 
            abc.abcB,
            abc.abcA
        };

        glyphs.push_back(g);

        x += (abc.abcA + abc.abcB + abc.abcC) + pad;
      }
    }

    return convert_to_d3d9();
  }

  bool convert_to_d3d9(){
    if(device == nullptr){
      DBG("[-] c_textured_font::convert_to_d3d9: device is nullptr\n");
      return false;
    }

    if(d3d_texture != nullptr)
      d3d_texture->Release();

    d3d_texture = nullptr;

    DBG("[!] Creating texture\n");
    HRESULT r = device->CreateTexture(ATLAS_SIZE, ATLAS_SIZE, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &d3d_texture, nullptr);
    if(r != D3D_OK){
      DBG("[-] c_textured_font::convert_to_d3d9: failed to create texture!\n");
      return false;
    }

    DBG("[!] Locking texture\n");
    D3DLOCKED_RECT locked_rect;
    if(d3d_texture->LockRect(0, &locked_rect, NULL, D3DLOCK_DISCARD) != D3D_OK){
      DBG("[-] c_textured_font::convert_to_d3d9 failed to lock texture!\n");
      d3d_texture->Release();
      d3d_texture = nullptr;
      return false;
    }

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = ATLAS_SIZE;
    bmi.bmiHeader.biHeight      = -ATLAS_SIZE;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits = malloc(ATLAS_SIZE * ATLAS_SIZE * 4);
    if(bits == nullptr){
      DBG("[-] c_textured_font::convert_to_d3d9 out of memory!\n");
      return false;
    }

    DBG("[!] Getting bitmap bits: %p - %p\n", dc, bitmap);
    if(!GetDIBits(dc, bitmap, 0, ATLAS_SIZE, bits, &bmi, DIB_RGB_COLORS)){
      DBG("[-] c_textured_font::convert_to_d3d9 failed getdibits\n");
      free(bits);
      d3d_texture->UnlockRect(0);
      d3d_texture->Release();
      d3d_texture = nullptr;
      return false;
    }

    i32 num_pixels = ATLAS_SIZE * ATLAS_SIZE;
    for (i32 i = 0; i < num_pixels; ++i) {
      u8* pixel = (u8*)(uptr)bits + (uptr)i * (uptr)4;

      u8 red   = pixel[2];
      u8 green = pixel[1];
      u8 blue  = pixel[0];

      // Calculate luminance using weighted average for human perception. (It works, so leave it?) 
      pixel[3] = (u8)((0.299f * red) + (0.587f * green) + (0.114f * blue));
      
      
      pixel[0] = 255;
      pixel[1] = 255;
      pixel[2] = 255;

      i8 r = pixel[0];
      pixel[0] = pixel[2];
      pixel[2] = r;
    }

    memcpy(locked_rect.pBits, bits, ATLAS_SIZE * ATLAS_SIZE * 4);

    if(d3d_texture->UnlockRect(0))
      return false;
    
    free(bits);
    DeleteObject(bitmap);
    DeleteDC(dc);

    bitmap = nullptr;
    dc     = nullptr;

    DBG("[!] Succesfully converted bitmap to d3d9\n");
    return true;
  }

  c_font_glyph* char_to_glyph(wchar_t ch){
    if(glyphs.size() == 0)
      return nullptr;

    for(i32 i = 0; i < glyphs.size(); i++){
      if(glyphs[i].ch == ch)
        return &glyphs[i];
    }

    return nullptr;
  }

  HFONT create_font()
};
*/