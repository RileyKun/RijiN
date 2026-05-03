#pragma once

#define RENDER_SETUP() if(render == nullptr) render = new render_d3d11_imgui;

class c_font_d3d11 : public c_font_base{
public:
  vec3i get_size(u32 font_size, std::wstring wstr, bool* success) override;
  vec3i draw(vec3i pos, u32 font_size, std::wstring wstr, colour col, u8 style, u8 align_x, u8 align_y) override;
};

class render_d3d11_imgui : public c_render_base{
public:
  ID3D11RenderTargetView* render_target_view;
  ID3D11DeviceContext*    device_context;
  ID3D11Device*           device;
  DXGI_SWAP_CHAIN_DESC    desc;

  render_d3d11_imgui(){};

  render_mode mode() override{
    return RENDER_MODE_D3D11_IMGUI;
  }

  bool setup(IDXGISwapChain* swap_chain){
    if(is_setup)
      return true;

    assert(swap_chain != nullptr);

    if(swap_chain->GetDesc(&desc)){
      assert(false && "Failed to get desc");
      return false;
    }

    // Obtain a pointer to the device from the swapchain
    if(swap_chain->GetDevice(__uuidof(ID3D11Device), (void**)&device)){
      assert(false && "Failed to get device");
      return false;
    }

    // Fetch the device context from the obtained device
    device->GetImmediateContext(&device_context);
    if (device_context == nullptr){
      assert(false && "Failed to create device context");
      return false;
    }

    ID3D11Texture2D* back_buffer;
    {
      // Get the back buffer texture
      if(swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back_buffer)){
        assert(false && "GetBuffer failed");
        return false;
      }

      // Create a render target view instance
      if(device->CreateRenderTargetView(back_buffer, NULL, &render_target_view)){
        assert(false && "CreateRenderTargetView failed");
        return false;
      }

      // Release the temp back buffer
      back_buffer->Release();
    }

    // Get the current process window we are in
    void* window = utils::get_current_process_window();

    if(window == nullptr){
      assert(false && "window nullptr");
      return false;
    }

    // Create the ImGUI instance
    ImGui::CreateContext();
    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(device, device_context);

    ImGuiIO& io = ImGui::GetIO();

    // We need to add the default font before any others
    // If we don't it seems to cause weird texture bugs
    io.Fonts->AddFontDefault();

    // Update the screen size
    update_screen_size();

    is_setup = true;
    return true;
  }

  void update_screen_size(){
    RECT screen_rect;
    GetClientRect(desc.OutputWindow, &screen_rect);

    // Now set the calculated screen size
    screen_size = vec3i(screen_rect.right - screen_rect.left, screen_rect.bottom - screen_rect.top);
  }

  void begin(){
    update_screen_size();

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(screen_size.x, screen_size.y));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    ImGui::Begin(XOR("root"), nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs);

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    assert(draw_list != nullptr);

    // Stop anti aliased lines from using a texture buffer
    if(draw_list->Flags & ImDrawListFlags_AntiAliasedLinesUseTex)
      draw_list->Flags &= ~ImDrawListFlags_AntiAliasedLinesUseTex;
  }

  void end(){
    assert(device_context != nullptr);

    // Set the render target to our correct render target view
    device_context->OMSetRenderTargets(XOR32(1), &render_target_view, nullptr);

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::EndFrame();
    ImGui::Render();

    // Render all the batches of drawcmds
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
  }

  c_font_base* create_font(std::string ttf_file, u32 render_px, bool merge_mode = false){
    ImFontConfig font_cfg;
    font_cfg.FontDataOwnedByAtlas = false;
    font_cfg.MergeMode = merge_mode;

    ImGuiIO& io = ImGui::GetIO();
    ImFont* font = io.Fonts->AddFontFromFileTTF(ttf_file.c_str(), (float)render_px, &font_cfg, io.Fonts->GetGlyphRangesDefault());

    if(font == nullptr){
      assert(false && "font nullptr");
      return nullptr;
    }

    c_font_d3d11* font_base = new c_font_d3d11;
    {
      font_base->render_base = this;
      font_base->font[0]     = font;
    }

    return font_base;
  }

  c_font_base* create_font_from_memory(void* ttf_buffer, u32 render_px, bool merge_mode = false){
    ImFontConfig font_cfg;
    font_cfg.FontDataOwnedByAtlas = false;
    font_cfg.MergeMode = merge_mode;

    ImGuiIO& io = ImGui::GetIO();
    ImFont* font = io.Fonts->AddFontFromMemoryCompressedBase85TTF(ttf_buffer, (float)render_px, &font_cfg, io.Fonts->GetGlyphRangesDefault());

    if(font == nullptr){
      assert(false && "font nullptr");
      return nullptr;
    }

    c_font_d3d11* font_base = new c_font_d3d11;
    {
      font_base->render_base = this;
      font_base->font[0]     = font;
    }

    return font_base;
  }

  void build_all_fonts(){
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Build();
  }

  void push_clip_region(vec3i pos, vec3i size) override{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    assert(draw_list != nullptr);

    draw_list->PushClipRect(ImVec2(pos.x, pos.y), ImVec2(pos.x + size.x + 1, pos.y + size.y + 1), true);
  }

  void pop_clip_region() override{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    assert(draw_list != nullptr);

    draw_list->PopClipRect();
  }

  void filled_rect(vec3i pos, vec3i size, colour col, float rounding = 0.f, u32 rounding_flags = 0) override{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    assert(draw_list != nullptr);

    draw_list->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + size.x, pos.y + size.y), ImColor((i32)col.r, (i32)col.g, (i32)col.b, (i32)col.a), rounding);
  }

  void outlined_rect(vec3i pos, vec3i size, colour col, float rounding = 0.f, u32 rounding_flags = 0) override{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    assert(draw_list != nullptr);

    // NOTE: ImDrawListFlags_AntiAliasedLinesUseTex should be turned off before calling this
    draw_list->AddRect(ImVec2(pos.x, pos.y), ImVec2(pos.x + size.x, pos.y + size.y), ImColor((i32)col.r, (i32)col.g, (i32)col.b, (i32)col.a), rounding);
  }

  void gradient_rect(vec3i pos, vec3i size, colour top_left, colour top_right, colour bottom_left, colour bottom_right, float rounding = 0.f) override{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    assert(draw_list != nullptr);

    if(gradient_rounding_hack){
      gradient_rounding_hack = false;

      // https://github.com/ocornut/imgui/issues/346#issuecomment-172087814
      // AddRectFilledMultiColor doesn't apply rounding so the gradient blending looks like shit
      // Using this trick we can solve this issue with the only downside beingn
      draw_list->AddRectFilledMultiColor(ImVec2(pos.x, pos.y), ImVec2(pos.x + size.x, pos.y + size.y),
        ImColor((i32)top_left.r, (i32)top_left.g, (i32)top_left.b, (i32)top_left.a),
        ImColor((i32)top_right.r, (i32)top_right.g, (i32)top_right.b, (i32)top_right.a),
        ImColor((i32)top_right.r, (i32)top_right.g, (i32)top_right.b, (i32)top_right.a),
        ImColor((i32)top_left.r, (i32)top_left.g, (i32)top_left.b, (i32)top_left.a)
      );

      draw_list->AddRectFilledMultiColor(ImVec2(pos.x, pos.y), ImVec2(pos.x + size.x, pos.y + size.y),
        ImColor(0, 0, 0, 0),
        ImColor(0, 0, 0, 0),
        ImColor((i32)bottom_right.r, (i32)bottom_right.g, (i32)bottom_right.b, (i32)bottom_right.a),
        ImColor((i32)bottom_left.r, (i32)bottom_left.g, (i32)bottom_left.b, (i32)bottom_left.a)
      );
    }
    else{
      draw_list->AddRectFilledMultiColor(ImVec2(pos.x, pos.y), ImVec2(pos.x + size.x, pos.y + size.y),
        ImColor((i32)top_left.r, (i32)top_left.g, (i32)top_left.b, (i32)top_left.a),
        ImColor((i32)top_right.r, (i32)top_right.g, (i32)top_right.b, (i32)top_right.a),
        ImColor((i32)bottom_right.r, (i32)bottom_right.g, (i32)bottom_right.b, (i32)bottom_right.a),
        ImColor((i32)bottom_left.r, (i32)bottom_left.g, (i32)bottom_left.b, (i32)bottom_left.a)
      );
    }
  }

  void solid_line(vec3i pos1, vec3i pos2, colour col, bool anti_alias, float width) override{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    assert(draw_list != nullptr);

    draw_list->AddLine(ImVec2(pos1.x, pos1.y), ImVec2(pos2.x, pos2.y), ImColor((i32)col.r, (i32)col.g, (i32)col.b, (i32)col.a), width);
  }

  void filled_circle(vec3i pos, float radius, float sides, colour col) override{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    assert(draw_list != nullptr);

    draw_list->AddCircleFilled(ImVec2(pos.x, pos.y), radius, ImColor((i32)col.r, (i32)col.g, (i32)col.b, (i32)col.a), (i32)sides);
  }

  void outlined_circle(vec3i pos, float radius, float sides, colour col) override{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    assert(draw_list != nullptr);

    draw_list->AddCircle(ImVec2(pos.x, pos.y), radius, ImColor((i32)col.r, (i32)col.g, (i32)col.b, (i32)col.a), (i32)sides);
  }

  // Copied from imgui code
  void tick_mark(vec3i pos, colour col, float px) override{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    assert(draw_list != nullptr);

    float thickness = math::biggest(px / 5.0f, 1.0f);
    px -= thickness * 0.5f;
    pos += vec3i(thickness * 0.25f, thickness * 0.25f);

    float third = px / 3.0f;
    float bx = pos.x + third;
    float by = pos.y + px - third * 0.5f;
    draw_list->PathLineTo(ImVec2(bx - third, by - third));
    draw_list->PathLineTo(ImVec2(bx, by));
    draw_list->PathLineTo(ImVec2(bx + third * 2.0f, by - third * 2.0f));
    draw_list->PathStroke(ImColor((i32)col.r, (i32)col.g, (i32)col.b, (i32)col.a), 0, thickness);
  }
};

extern render_d3d11_imgui* render;