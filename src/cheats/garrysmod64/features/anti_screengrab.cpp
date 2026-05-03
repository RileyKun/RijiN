#include "../link.h"

CLASS_ALLOC(c_anti_screengrab, anti_screengrab);

void c_anti_screengrab::init_materials(){
  cookie_block_check_return();
  if(view_texture == nullptr){
    DBG("[!] Creating view texture\n");
    view_texture = global->material_system->create_named_render_target_texture_ex(XOR("rltex"),
      1, 1,
      RT_SIZE_FULL_FRAME_BUFFER,
      global->material_system->get_back_buffer_format(),
      MATERIAL_RT_DEPTH_SHARED,
      TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT, 1);

    DBG("[!] Created view texture.\n");
  }

  if(view_material == nullptr){
    DBG("[!] Creating view material.\n");
    c_keyvalues* kv = chams->create_shader_kv(XOR("UnlitGeneric"));
    kv->set_string(XOR("$basetexture"), XOR("rltex"));
    view_material = chams->create_material(XOR("rlmat"), kv);
    DBG("[!] Created view material\n");
  }

  if(read_pixels_texture == nullptr){
    DBG("[!] Creating read pixels view texture\n");
    read_pixels_texture = global->material_system->create_named_render_target_texture_ex(XOR("rltex_rp1"),
      1, 1,
      RT_SIZE_FULL_FRAME_BUFFER,
      global->material_system->get_back_buffer_format(),
      MATERIAL_RT_DEPTH_SHARED,
      TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT, 1);

    DBG("[!] Created rp view texture.\n");
  }

  if(read_pixels_material == nullptr){
    DBG("[!] Creating read pixels view material.\n");
    c_keyvalues* kv = chams->create_shader_kv(XOR("UnlitGeneric"));
    kv->set_string(XOR("$basetexture"), XOR("rltex_rp1"));
    read_pixels_material = chams->create_material(XOR("rlmat_rp1"), kv);
    DBG("[!] Created rp view material\n");
  }
}

void c_anti_screengrab::check_game_screenshot_states(){
  cookie_block_check_return();

  assert(global->movie_name_addr != nullptr);
  i8* movie = (i8*)global->movie_name_addr;

  static bool is_movie_recording = false;
  if(is_movie_recording && movie[0] == 0){
    notify->create(NOTIFY_TYPE_WARNING, WXOR(L"Your game is no longer recording a movie."));
    is_movie_recording = false;
  }
  else if(!is_movie_recording && movie[0] != 0){
    notify->create(NOTIFY_TYPE_WARNING, WXOR(L"Your game has started recording a movie.\nUncommon screengrab method."));
    is_movie_recording = true;
  }

  if(global->engine->is_taking_screenshot())
    notify->create(NOTIFY_TYPE_WARNING, WXOR(L"An engine screenshot occured."));
 
  if(movie[0] != 0 || global->engine->is_taking_screenshot()){
    global->info_panel->add_entry(INFO_PANEL_ANTISCREENGRAB, WXOR(L"ANTI-SCREENGRAB"), WXOR(L"INTERNAL SCREENSHOT"), INFO_PANEL_WARNING_CLR);
    last_screenshot_taken_time  = math::time();
  }
}

bool c_anti_screengrab::is_screenshot_taken(){
  return is_screen_grab_taking_place;
}

bool c_anti_screengrab::is_creating_spoofed_view(){
  return is_rendering_new_view && !calcuating_viewport;
}

// Maybe we'll want some code to run or nah?
void c_anti_screengrab::on_screen_grab_start(){
  DBG("[!] c_anti_screengrab::on_screen_grab_start()\n");
  last_screenshot_taken_time  = math::time();
  is_screen_grab_taking_place = true;
  notify->create(NOTIFY_TYPE_WARNING, WXOR(L"The server is grabbing your screen via render.capture"));
}

void c_anti_screengrab::on_screen_grab_end(){
  DBG("[!] c_anti_screengrab::on_screen_grab_end()\n");
  is_screen_grab_taking_place = false;
}

// Maybe we'll want some code to run or nah?
void c_anti_screengrab::on_screen_recording_start(){
  DBG("[!] c_anti_screengrab::on_screen_recording_start()\n");
  last_screenshot_taken_time  = math::time();
  is_screen_grab_taking_place = true;
  notify->create(NOTIFY_TYPE_WARNING, WXOR(L"The server is grabbing your screen via video.record"));
}

void c_anti_screengrab::on_screen_recording_end(){
  DBG("[!] c_anti_screengrab::on_screen_recording_end()\n");
  is_screen_grab_taking_place = false;
}

void c_anti_screengrab::on_render_target_override(c_texture* texture){
  if(texture == nullptr)
    return;
}

// Since screenshots take forever to send over to the server?
bool c_anti_screengrab::should_update_frame(){
  cookie_block_check_return();
  if(acp->is_blocked(FUNC_SCREEN_GRAB))
    return false;

  if(utils::is_screen_shot_taken())
    return true;

  c_base_player* localplayer = utils::localplayer();
  if(localplayer == nullptr)
    return true;

  vec3 origin = localplayer->origin();
  vec3 angles = global->engine->get_view_angles();

  float update_rate = 0.25f;
  if(origin == last_knowledged_origin && angles == last_viewangle)
    update_rate = 0.5f;

  float dist = (origin - last_knowledged_origin).length();
  if(dist > 8912.f)
    update_rate = 0.f;
  else if(dist > 4096.f)
    update_rate = 0.1f;

  
  last_viewangle         = angles;
  last_knowledged_origin = localplayer->origin();
  
  float time = math::time();

  if(last_video_add_frame_time > time)
    return true;

  if(last_update_time > time)
    return false;

  last_update_time = time + update_rate;
  return true;
}

bool c_anti_screengrab::should_override_set_render_target(c_texture* texture){
  if(texture == nullptr || view_texture == nullptr)
    return false;

  if(acp->is_blocked(FUNC_SCREEN_GRAB_NO_2D) || acp->is_blocked(FUNC_SCREEN_GRAB))
    return false;

  i8* name              = texture->get_name();
  i32 width             = texture->get_actual_width();
  i32 height            = texture->get_actual_height();
  bool is_render_target = texture->is_render_target() || texture->is_temp_render_target();
  if(name == nullptr)
    return false;

  // This doesn't really make any sense here, but okay.
  if(!is_render_target)
    return false;

  u32 name_hash = HASH_RT(name);
  for(i32 i = 0; i < 128; i++){
    if(name_hash == blacklisted_render_targets[i]){
      //DBG("[!] (HASH) %s is blacklisted\n", name);
      return true;
    }

    // Check whitelisted entries. The ones that didn't make the blacklist.
    if(name_hash == whitelisted_render_targets[i]){
      //DBG("[!] (HASH) %s is whitelisted\n", name);
      return false;
    }
  }

  // There's actually an anti-cheat that tries crashing cheats by doing something like this.
  // This is the part where we try and ignore epic special effect render targets and what not.
  if(width < 640 || height < 480 || width > 3841 || height > 2161){
    //DBG("[-] Odd render target size %s: %ix%i, not overriding\n", name, width, height);
    return false;
  }

  bool blacklisted = false;

  // Look at any anti-screengrab that calls SetRenderTarget and place its texture name in here.
  // None of the plugins as of 1.14.2024 have any form of randomized text strings.
  if(str_utils::contains(name, XOR("screenshot")) ||
     str_utils::contains(name, XOR("screengrab")) ||
     str_utils::contains(name, XOR("screen_shot")) ||
     str_utils::contains(name, XOR("scrgrab")) ||
     str_utils::contains(name, XOR("scrngrab")) ||
     str_utils::contains(name, XOR("scrshot")) ||
     str_utils::contains(name, XOR("scrnshot")) ||
     str_utils::contains(name, XOR("screen_grab"))){

    DBG("[!] (STRING) %s is now blacklisted\n", name);
    blacklisted = true;
  }
  else{

    i32 width_delta = math::abs(render->screen_size.x - width);
    i32 height_delta = math::abs(render->screen_size.y - height);

    // I know for a fact that one plugin will just subtract -1 from the screen size to bypass a compair check.
    // This for the most part should counter act that funnyness.
    if(width_delta >= 0 && width_delta <= 5 && height_delta >= 0 && height_delta <= 5){
      DBG("[!] (TEXTURE SIZE CHECK) %s is now blacklisted\n", name);
      blacklisted = true;
    }
  }

  // Anything not detected should be whitelisted to reduce any performance overhead.
  // Because the string contains function above isn't exactly that fast.
  if(!blacklisted){
    DBG("[!] Didn't meet blacklist requirements %s %ix%i\n", name, width, height);
    whitelisted_render_targets[whitelisted_cycle] = name_hash;
    whitelisted_cycle++;
    if(whitelisted_cycle >= 128)
      whitelisted_cycle = 0;
  }
  else{
    blacklisted_render_targets[blacklisted_cycle] = name_hash;
    blacklisted_cycle++;
    if(blacklisted_cycle >= 128)
      blacklisted_cycle = 0;
  }

  return blacklisted;
}

bool c_anti_screengrab::is_rijin_texture(c_texture* texture){
  if(texture == nullptr)
    return false;

  i8* name = texture->get_name();
  if(name == nullptr)
    return false;

  u32 name_hash = HASH_RT(name);
  if(name_hash == HASH("rltex")
    || name_hash == HASH("rltex_rp1")
    || name_hash == HASH("tgb1")
    || name_hash == HASH("tgb2"))
    return true;

  return false;
}

void c_anti_screengrab::create_spoofed_view(void* rcx, s_view_setup* view, i32 flags, i32 what_to_draw){
  assert(rcx != nullptr);
  if(!should_update_frame())
    return;

  // Removed the old s_view_setup, because i think our struct is not correctly sized in both x64 and x86.
  // It was causing weird effects in the game, which might explain the weird Cam3D glitches and other view matrix issues in darkRP.
  // Modern Warfare weapons still flicker, must be either a setupbones check or a frame check.

  init_materials();
  check_game_screenshot_states();
 
  c_render_context* rc = global->material_system->get_render_context();
  if(rc == nullptr)
    return;

  //rc->clear_buffers(true, true, true);
  for(i32 i = 1; i <= global->entity_list->get_highest_index(); i++){
    c_base_entity* entity = global->entity_list->get_entity(i);
    if(i == global->engine->get_local_player())
      continue;

    if(entity == nullptr)
      continue;

    if(entity->clr_render_a != 254)
      continue;

    chams->set_alpha(entity, 1.f, true);
  }
  // Pretty much re-rendering the game again and then copying the texture it created over.
  // Now pretty much all we have todo is verify that any lua callbacks are disabled and do not affect rendering with in the fake image such as.
  // - The hud has to be properly drawn
  // - The viewmodel has to be properly drawn and not call any callbacks in lua. (Avoid detection)
  // - Players look normal and have no weird visual issues.

  rc->begin_render();
  // If we do custom FOV, we can't just replace the FOV in the view setup and call it a day.
  // We'll need to calculate a few other variables in the view setup in order to do properly.
  // Otherwise, we'll have a very odd looking view.

  is_rendering_new_view = true;
  utils::call_fastcall64<void, s_view_setup*, i32, i32>(gen_internal->decrypt_asset(global->crenderview_renderview_hook_trp), rcx, view, flags, what_to_draw);
  is_rendering_new_view = false;

  rc->copy_render_target_to_texture(view_texture);
  rc->end_render();
}

// Sets the fake render target.
// You should be warned that many lua addons will remove any active RenderTargets.
// So you'll need to hook the functions they're using after doing such to get around it.
// Such as render.capture and likely more.
void c_anti_screengrab::push(){
  cookie_block_check_return();
  DBG("[!] c_anti_screengrab::push\n");
  c_render_context* rc = global->material_system->get_render_context();
  if(rc == nullptr)
    return;

  if(view_texture == nullptr)
    return;

  rc->push_render_target_and_viewport();
  rc->set_render_target(view_texture);
  has_pushed = true;

  global->info_panel->add_entry(INFO_PANEL_ANTISCREENGRAB, WXOR(L"ANTI-SCREENGRAB"), WXOR(L"LUA METHOD"), INFO_PANEL_WARNING_CLR);
}

void c_anti_screengrab::pop(){
  cookie_block_check_return();
  DBG("[!] c_anti_screengrab::pop\n");
  c_render_context* rc = global->material_system->get_render_context();
  if(rc == nullptr)
    return;

  if(!has_pushed)
    return;

  rc->pop_render_target_and_viewport();
  has_pushed = false;
}