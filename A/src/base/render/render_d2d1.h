#pragma once

#define RENDER_SETUP() if(render == nullptr) render = new c_render_d2d1;
class c_font_d2d1 : public c_font_base{
public:
  virtual ~c_font_d2d1(){}

  vec3i get_size(u32 font_size, std::wstring wstr, bool* success) override;
  vec3i draw(vec3i pos, u32 font_size, std::wstring wstr, colour col, u8 style, u8 align_x, u8 align_y) override;
};

class c_render_d2d1 : public c_render_base{
public:
  ID2D1Factory*           factory         = nullptr;
  ID2D1HwndRenderTarget*  render_target   = nullptr;
  ID2D1SolidColorBrush*   brush           = nullptr;
  IDWriteFactory*         dwrite_factory  = nullptr;
  IWICImagingFactory*     wic_factory     = nullptr;
  vec3i                   render_offset;

  c_render_d2d1(){};

  virtual ~c_render_d2d1(){}

  render_mode mode() override{
    return RENDER_MODE_D2D1;
  }

  bool setup(void* window_handle){
    u32 status = 0;

    if((status = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory)) != 0){
      DBG("[-] D2D1CreateFactory failed, status: %X\n", status);
      return false;
    }

    RECT rect;
    I(GetClientRect)(window_handle, &rect);

    DBG("[+] Created Render target\n");

    // Create render target
    if((status = factory->CreateHwndRenderTarget(
      D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)),
      D2D1::HwndRenderTargetProperties(window_handle, D2D1::SizeU(rect.right, rect.bottom), D2D1_PRESENT_OPTIONS_IMMEDIATELY),
      &render_target)) != 0){
      DBG("[-] factory->CreateHwndRenderTarget failed, status: %X\n", status);
      return false;
    }

    DBG("[+] Created Colour brush\n");

    // Create color brush
    if((status = render_target->CreateSolidColorBrush(D2D1::ColorF(255, 255, 255, 255), &brush)) != 0){
      DBG("[-] render_target->CreateSolidColorBrush failed, status: %X\n", status);
      return false;
    }

    DBG("[+] Created DWrite handle\n");

    // Allocate a dwrite instance
    if((status = I(DWriteCreateFactory)(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&dwrite_factory)) != 0){
      DBG("[-] DWriteCreateFactory failed, status: %X\n", status);
      return false;
    }

    I(CoInitializeEx)(nullptr, COINIT_MULTITHREADED);
    if(I(CoCreateInstance)(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (void**)&wic_factory) != 0){
      DBG("[-] CoCreateInstance failed, status: %X\n", GetLastError());
      return false;
    }

    DBG("[+] D2D1 Instance Created\n");
    is_setup = true;

    return true;
  }

  void strict_no_antialias(bool text = false){
    if(render_target == nullptr)
      return;
    
    render_target->SetAntialiasMode( D2D1_ANTIALIAS_MODE_ALIASED );

    if(text)
      render_target->SetTextAntialiasMode( D2D1_TEXT_ANTIALIAS_MODE_ALIASED );
    else
      render_target->SetTextAntialiasMode( D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE );
  }

  void begin_draw() override{
    if(render_target == nullptr)
      return;

    D2D1_SIZE_F size = render_target->GetSize();
    screen_size = vec3i(size.width, size.height, 0.f);

    render_target->BeginDraw();
    render_target->Clear();
  }

  void end_draw() override{
    if(render_target == nullptr)
      return;

    render_target->EndDraw();
  }

  void unload(bool device_reset) override{
    if(render_target != nullptr){
      render_target->Release();
      render_target = nullptr;
    }

    if(brush != nullptr){
      brush->Release();
      brush = nullptr;
    }

    if(dwrite_factory != nullptr){
      dwrite_factory->Release();
      dwrite_factory = nullptr;
    }

    if(factory != nullptr){
      factory->Release();
      factory = nullptr;
    }
  }

  void apply_render_offset(vec3i& pos){
    pos += render_offset;
  }

  void set_colour(colour col){
    if(brush == nullptr)
      return;

    col = math::clamp(col);

    static u32  last_hex = 0;
    u32         new_hex  = col.tohex();

    if(new_hex != last_hex){
      brush->SetColor(D2D1::ColorF( new_hex ));
      last_hex = new_hex;
    }

    static float last_alpha = 0.f;
    float        new_alpha  = col.a;
    if(last_alpha != new_alpha){
      new_alpha = math::smallest(new_alpha, 255.f);

      brush->SetOpacity( new_alpha > 0.f ? new_alpha / 255.f : 0.f );
      last_alpha = new_alpha;
    }
  }

  ID2D1Bitmap* create_bitmap(void* bitmap_buffer, u32 bitmap_len){
    IWICStream*             wic_stream    = nullptr;
    IWICBitmapDecoder*      wic_decoder   = nullptr;
    IWICBitmapFrameDecode*  wic_frame     = nullptr;
    IWICFormatConverter*    wic_converter = nullptr;
    ID2D1Bitmap*            new_bitmap    = nullptr;

    // Create the stream and load our bitmap buffer into it
    wic_factory->CreateStream(&wic_stream);

    if(wic_stream->InitializeFromMemory(bitmap_buffer, bitmap_len) != 0)
      goto finish;

    if(wic_factory->CreateDecoderFromStream(wic_stream, nullptr, WICDecodeMetadataCacheOnLoad, &wic_decoder) != 0)
      goto finish;

    if(wic_decoder->GetFrame(0, &wic_frame) != 0)
      goto finish;

    if(wic_factory->CreateFormatConverter(&wic_converter) != 0)
      goto finish;

    if(wic_converter->Initialize(wic_frame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.f, WICBitmapPaletteTypeMedianCut) != 0)
      goto finish;

    if(render_target->CreateBitmapFromWicBitmap(wic_converter, nullptr, &new_bitmap) != 0)
      goto finish;

  finish:
    if(wic_decoder != nullptr)
      wic_decoder->Release();

    if(wic_frame != nullptr)
      wic_frame->Release();

    if(wic_stream != nullptr)
      wic_stream->Release();

    if(wic_converter != nullptr)
      wic_converter->Release();

    return new_bitmap;
  }

  void draw_bitmap(ID2D1Bitmap* bitmap, vec3i pos, vec3i size, float opacity = 1.f, bool high_quality = true){
    apply_render_offset(pos);

    assert(bitmap != nullptr);

    render_target->DrawBitmap(bitmap, D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), opacity, high_quality ? D2D1_BITMAP_INTERPOLATION_MODE_LINEAR : D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
    //outlined_rect(pos, size, rgb(0,255,0));
  }

  void filled_rect(vec3i pos, vec3i size, colour col, float rounding = 0.f, u32 rounding_flags = 0) override{
    apply_render_offset(pos);

    if(render_target == nullptr)
      return;

    set_colour(col);

    if(rounding > 0.f)
      render_target->SetAntialiasMode( D2D1_ANTIALIAS_MODE_PER_PRIMITIVE );

    if(rounding > 0.f)
      render_target->FillRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), rounding, rounding), brush);
    else
      render_target->FillRectangle(D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), brush);

    if(rounding > 0.f)
      render_target->SetAntialiasMode( D2D1_ANTIALIAS_MODE_ALIASED );
  }

  void outlined_rect(vec3i pos, vec3i size, colour col, float rounding = 0.f, u32 rounding_flags = 0) override{
    apply_render_offset(pos);

    if(render_target == nullptr)
      return;

    set_colour(col);

    // Credits: nitro
    D2D1_ROUNDED_RECT corrected_rect;
    corrected_rect.rect         = D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y);
    corrected_rect.radiusX      = rounding;
    corrected_rect.radiusY      = rounding;
    corrected_rect.rect.left    += 0.5f;
    corrected_rect.rect.top     += 0.5f;
    corrected_rect.rect.right   -= 0.5f;
    corrected_rect.rect.bottom  -= 0.5f;

    if(rounding > 0.f)
      render_target->SetAntialiasMode( D2D1_ANTIALIAS_MODE_PER_PRIMITIVE );

    render_target->DrawRoundedRectangle( corrected_rect, brush );

    if(rounding > 0.f)
      render_target->SetAntialiasMode( D2D1_ANTIALIAS_MODE_ALIASED );
  }

  void gradient_rect_linear(vec3i pos, vec3i size, vec3i start, vec3i end, colour from, colour to, float rounding = 0.f) override{
    apply_render_offset(pos);

    if(render_target == nullptr)
      return;

    ID2D1GradientStopCollection* gradient_stop_collection = nullptr;
    ID2D1LinearGradientBrush*    linear_gradient_brush;

    D2D1_GRADIENT_STOP gradient_stops[ 2 ];
    gradient_stops[ 0 ].color    = D2D1::ColorF( from.tohex( ), from.a );
    gradient_stops[ 0 ].position = 0.0f;
    gradient_stops[ 1 ].color    = D2D1::ColorF( to.tohex( ), to.a );
    gradient_stops[ 1 ].position = 1.f;

    render_target->CreateGradientStopCollection( gradient_stops, 2, D2D1_GAMMA_2_2, D2D1_EXTEND_MODE_CLAMP, &gradient_stop_collection );
    render_target->CreateLinearGradientBrush( D2D1::LinearGradientBrushProperties( D2D1::Point2F( pos.x, pos.y ), D2D1::Point2F( pos.x, pos.y + size.y ) ), gradient_stop_collection, &linear_gradient_brush );

    linear_gradient_brush->SetOpacity(from.a > 0.f ? from.a / 255.f : 0.f);

    linear_gradient_brush->SetStartPoint(D2D1::Point2F(pos.x + start.x, pos.y + start.y));
    linear_gradient_brush->SetEndPoint(D2D1::Point2F(pos.x + end.x, pos.y + end.y));

    if(rounding > 0.f)
      render_target->FillRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), rounding, rounding), linear_gradient_brush);
    else
      render_target->FillRectangle(D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), linear_gradient_brush);

    linear_gradient_brush->Release( );
    gradient_stop_collection->Release( );
  }

  void progress_circle(vec3i pos, vec3i size, float thickness, float degree, colour circling_bar, colour back_bar = rgba(0.f, 0.f, 0.f, 0.f)){
    apply_render_offset(pos);

    ID2D1GeometryGroup* geometry_group;
    ID2D1RadialGradientBrush* radial_gradient_brush;
    ID2D1EllipseGeometry* ellipse_array[2];

    vec3i correct_pos   = pos + (size / 2);
    vec3i correct_size  = (size / 2) - thickness;

    D2D1_ELLIPSE ellipse[2] = {
      D2D1::Ellipse(D2D1::Point2F(correct_pos.x, correct_pos.y), correct_size.x, correct_size.y),
      D2D1::Ellipse(D2D1::Point2F(correct_pos.x, correct_pos.y), correct_size.x + thickness, correct_size.y + thickness),
    };

    D2D1_GRADIENT_STOP gradient_stops[2];
    gradient_stops[0].color = D2D1::ColorF(back_bar.tohex(), back_bar.a > 0.f ? (back_bar.a / 255.f) : 0.f);
    gradient_stops[0].position = 0.99f;
    gradient_stops[1].color = D2D1::ColorF(circling_bar.tohex(), circling_bar.a > 0.f ? (circling_bar.a / 255.f) : 0.f);
    gradient_stops[1].position = 1.f;

    // Create gradient stops collection
    ID2D1GradientStopCollection* gradient_stops_collection;
    render_target->CreateGradientStopCollection(gradient_stops, 2, D2D1_GAMMA_2_2, D2D1_EXTEND_MODE_CLAMP, &gradient_stops_collection);

    // Create a linear gradient brush to fill in the ellipse
    render_target->CreateRadialGradientBrush(
      D2D1::RadialGradientBrushProperties(
      D2D1::Point2F(correct_size.x + correct_pos.x, correct_size.y + correct_pos.y),
      D2D1::Point2F(0, 0),
      correct_size.x,
      correct_size.y),
      gradient_stops_collection,
      &radial_gradient_brush
    );

    for(i32 i = 0; i < 2; i++)
      factory->CreateEllipseGeometry(ellipse[i], &ellipse_array[i]);

    factory->CreateGeometryGroup(D2D1_FILL_MODE_ALTERNATE, (ID2D1Geometry**)&ellipse_array, ARRAYSIZE(ellipse_array), &geometry_group);

    D2D1_MATRIX_3X2_F new_matrix = D2D1::Matrix3x2F::Rotation(degree, D2D1::Point2F(correct_pos.x, correct_pos.y));
    radial_gradient_brush->SetTransform(new_matrix);

    render_target->SetAntialiasMode( D2D1_ANTIALIAS_MODE_PER_PRIMITIVE );
    render_target->FillGeometry(geometry_group, radial_gradient_brush);
    render_target->SetAntialiasMode( D2D1_ANTIALIAS_MODE_ALIASED );

    for(i32 i = 0; i < 2; i++)
      ellipse_array[i]->Release();

    geometry_group->Release();
    radial_gradient_brush->Release();
    gradient_stops_collection->Release();
  }

  void filled_circle(vec3i pos, float radius, float sides, colour col) override{
    apply_render_offset(pos);

    D2D1_ELLIPSE ellipse1 = D2D1::Ellipse(D2D1::Point2F(pos.x, pos.y), radius, radius);

    set_colour(col);

    render_target->SetAntialiasMode( D2D1_ANTIALIAS_MODE_PER_PRIMITIVE );
    render_target->FillEllipse(ellipse1, brush);
    render_target->SetAntialiasMode( D2D1_ANTIALIAS_MODE_ALIASED );
  }

  void outlined_circle(vec3i pos, float radius, float sides, colour col) override{
    apply_render_offset(pos);

    D2D1_ELLIPSE ellipse1 = D2D1::Ellipse(D2D1::Point2F(pos.x, pos.y), radius, radius);

    set_colour(col);

    render_target->SetAntialiasMode( D2D1_ANTIALIAS_MODE_PER_PRIMITIVE );
    render_target->DrawEllipse(ellipse1, brush);
    render_target->SetAntialiasMode( D2D1_ANTIALIAS_MODE_ALIASED );
  }

  void push_clip_region(vec3i pos, vec3i size) override{
    apply_render_offset(pos);

    if(render_target == nullptr)
      return;

    render_target->PushAxisAlignedClip(D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), D2D1_ANTIALIAS_MODE_ALIASED);
  }

  void pop_clip_region() override{
    if(render_target == nullptr)
      return;

    render_target->PopAxisAlignedClip();
  }

  c_font_base* create_font(std::wstring family_name, u32 font_weight, u32 font_style = DWRITE_FONT_STYLE_NORMAL ) override{
    u32 status = 0;

    if(dwrite_factory == nullptr)
      return nullptr;

    IDWriteTextFormat* font = nullptr;
    IDWriteFontCollection* font_collection = nullptr;

    if((status = dwrite_factory->GetSystemFontCollection(&font_collection)) != 0){
      DBG("[-] dwrite_factory->GetSystemFontCollection failed, status: %X\n", status);
      return nullptr;
    }

    // Create the font base
    c_font_d2d1* font_base = new c_font_d2d1;
    {
      font_base->render_base    = this;

      for(u32 i = 1; i < sizeof(font_base->font) / sizeof(void*); i++){
        if((status = dwrite_factory->CreateTextFormat(&family_name[0], font_collection, font_weight, font_style, DWRITE_FONT_STRETCH_NORMAL, i, WXOR(L"en-us"), &font_base->font[i] )) != 0){
          font_collection->Release();
          delete font_base;
          DBG("[-] dwrite_factory->CreateTextFormat failed, status: %X\n", status);
          return nullptr;
        }
      }

      font_collection->Release();

      // Register it so our rendering system can know we have a font allocated
      register_font_base(font_base);
    }

    return (c_font_base*)font_base;
  }
};

extern c_render_d2d1* render;