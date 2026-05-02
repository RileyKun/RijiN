#pragma once


#define NAME_ENCRYPTION_KEY HASH("TND")


class gcs_component_base;

enum gcs_hooks{
  GCS_HOOK_SHOULD_ENABLE  = 1 << 1,
  GCS_HOOK_SHOULD_PAINT   = 1 << 2,
  GCS_HOOK_PRE_PAINT      = 1 << 3,
  GCS_HOOK_MAIN_PAINT     = 1 << 4,
  GCS_HOOK_POST_PAINT     = 1 << 5,
  GCS_HOOK_CLICK          = 1 << 6,
  GCS_HOOK_CLICK_SOUND    = 1 << 7,
  GCS_HOOK_INPUT_ENABLED  = 1 << 8,
};

struct gcs_component_info{
  // coords
  vec3i*  pos_ptr; // pos_ptr is designed so we can have the menu position dictated by something like a config value
  vec3i   pos;
  vec3i   size;
  vec3i   clip_pos_offset;
  vec3i   clip_size_offset;

  // name
  std::wstring short_name;
  std::wstring name;

  // custom fields
  bool enabled          = true;
  bool input_enabled    = true;
  bool always_run_think = false;

  // component based
  gcs_scene*          scene   = nullptr;
  gcs_component_base* parent  = nullptr;
  gcs_component_base* module  = nullptr;

  // tamper aspects
  u32 name_hash       = 0;
  u32 short_name_hash = 0;

  // Auto positioning
  bool    auto_positioning                  = false;
  vec3i   auto_positioning_padding          = vec3i(8, 8);
  vec3i   auto_positioning_offset;
  bool    override_auto_positioning         = false;
  i32     total_render_height               = 0;
  i32     auto_positioning_override_height  = 0;
  bool    disable_alpha_vis_fading          = false;

  // extra
  u32                 module_size   = 8;
  void*               paint_param   = nullptr;
  uptr                tick_count    = 0;
  uptr                param_buffer  = 0;
  std::vector<bool*>  render_on_list;
  float               old_alpha     = 1.f;
  std::wstring        help_text;

  ALWAYSINLINE void enable_auto_positioning(){
    auto_positioning = true;
  }

  ALWAYSINLINE void disable_auto_positioning(){
    auto_positioning = false;
  }

  ALWAYSINLINE void disable_alpha_vis_fade(){
    disable_alpha_vis_fading = true;
  }

  ALWAYSINLINE void enable_alpha_vis_fade(){
    disable_alpha_vis_fading = false;
  }

  ALWAYSINLINE void enable_override_auto_positioning(){
    override_auto_positioning = true;
  }

  ALWAYSINLINE void disable_override_auto_positioning(){
    override_auto_positioning = false;
  }

  // child components
  std::vector<gcs_component_base*> child_components;
  std::vector<std::pair<u32, std::function<bool( gcs_component_base* c, void* p )>>> hooks;
};

class gcs_component_base{
private:
  gcs_component_info component_info;
public:

  ALWAYSINLINE gcs_component_info* info(){
    return &component_info;
  }

  ALWAYSINLINE gcs_colour_scheme* scheme(){
    return info()->scene->scheme();
  }

  ALWAYSINLINE gcs_component_base* parent(){
    return info()->parent;
  }

  ALWAYSINLINE gcs_component_base* module(){
    return info()->module;
  }

  // Sets tick_count to 0, forcing a recalculation update of the element
  ALWAYSINLINE void force_update(){
    info()->tick_count = 0;
  }

  ALWAYSINLINE void set_parent(gcs_component_base* new_parent){
    info()->parent = new_parent;
  }

  ALWAYSINLINE void set_scene(gcs_scene* scene){
    info()->scene = scene;
  }

  ALWAYSINLINE bool is_root_component(){
    return parent() == nullptr;
  }

  ALWAYSINLINE void enable(){
    info()->enabled = true;
  }

  ALWAYSINLINE void disable(){
    info()->enabled = false;
  }

  ALWAYSINLINE void set_paint_param(void* p){
    info()->paint_param = p;
  }

  std::vector<gcs_component_base*>& children(){
    return info()->child_components;
  }

  gcs_component_base* set_render_on(bool* render_on_ptr){
    //info()->render_on = render_on_ptr;

    info()->render_on_list.push_back(render_on_ptr);

    return this;
  }

  gcs_component_base* set_help_text(std::wstring text){
    info()->help_text = text;
    return this;
  }

  ALWAYSINLINE void push_help_text_to_scene(){
    info()->scene->info()->help_text = info()->help_text;
  }

  ALWAYSINLINE void add_hook(u32 type, std::function<bool( gcs_component_base* c, void* p )> func){
    info()->hooks.emplace_back(type, func);
  }

  bool call_hook(u32 type, void* p = nullptr){
    if(info()->hooks.empty())
      return true;

    // use of auto since the decleration is huge
    for(auto& hook : info()->hooks)
      if(hook.first & type)
        return hook.second(this, p);

    return true;
  }

  ALWAYSINLINE void emit_click_sound(u32 type = 0){
    gcs_component_base* comp = this;

    while(comp->info()->parent != nullptr)
      comp = comp->info()->parent;

    if(comp == nullptr)
      return;

    comp->call_hook(GCS_HOOK_CLICK_SOUND, (void*)type);
  }

  ALWAYSINLINE gcs_scene* scene(){
    return info()->scene;
  }

  ALWAYSINLINE c_font_base* font(){
    return info()->scene->info()->font;
  }

  ALWAYSINLINE c_font_base* emoji(){
    assert(info()->scene->info()->font_emoji != nullptr);
    return info()->scene->info()->font_emoji;
  }

  ALWAYSINLINE vec3i mouse_pos(){
    return info()->scene->info()->mouse_pos;
  }

  ALWAYSINLINE u32 input(){
    return info()->scene->info()->input;
  }

  ALWAYSINLINE i32 push_align_offset(){
    return render->rescale(20);
  }

  ALWAYSINLINE void set_pos_ptr(vec3i* new_pos_ptr){
    info()->pos_ptr = new_pos_ptr;
  }

  vec3i pos(bool abs = true){
    vec3i ret = (info()->pos_ptr != nullptr ? *info()->pos_ptr : info()->pos);

    if(!info()->override_auto_positioning && parent() != nullptr && parent()->info()->auto_positioning)
      ret += parent()->info()->auto_positioning_offset;

    if(abs){
      gcs_component_base* comp = this;

      while((comp = comp->parent()) != nullptr)
        ret += comp->pos(false);
    }

    return ret;
  }

  ALWAYSINLINE virtual void set_pos(vec3i new_pos){
    if(info()->pos_ptr != nullptr)
      *info()->pos_ptr  = new_pos;
    else
      info()->pos       = new_pos;
  }

  ALWAYSINLINE virtual vec3i size(){
    return info()->size;
  }

  ALWAYSINLINE virtual void set_size(vec3i new_size){
    info()->size = new_size;
  }

  ALWAYSINLINE std::wstring short_name(){
    return utils::str_xor(info()->short_name, NAME_ENCRYPTION_KEY);
  }

  ALWAYSINLINE void set_short_name(std::wstring new_name){
    if(!info()->short_name_hash)
      info()->short_name_hash = WHASH_RT(new_name.c_str());

    utils::perform_str_xor(new_name, NAME_ENCRYPTION_KEY);
    info()->short_name = new_name;
  }

  ALWAYSINLINE std::wstring name(){
    return utils::str_xor(info()->name, NAME_ENCRYPTION_KEY);
  }

  ALWAYSINLINE virtual void set_name(std::wstring new_name){
    if(!info()->name_hash)
      info()->name_hash = WHASH_RT(new_name.c_str());

    utils::perform_str_xor(new_name, NAME_ENCRYPTION_KEY);
    info()->name = new_name;
  }

  ALWAYSINLINE bool is_in_rect(bool ignore_parent_check = false){
    return is_in_rect(pos(true), size(), ignore_parent_check);
  }

  NEVERINLINE bool is_in_rect(vec3i pos, vec3i size, bool ignore_parent_check = false){
    if(scene()->info()->input_frozen || !info()->input_enabled || !call_hook(GCS_HOOK_INPUT_ENABLED))
      return false;

#if defined(INTERNAL_CHEAT)
    kernel_time_expired_val(true);
#endif

    // TODO: we need to check the parent when we know we are in a scrolling state on the parent
    // To ensure that we dont click buttons half way out the top or the bottom
    // A simple is_in_rect of the parent should solve this problem

    vec3i mpos = mouse_pos();

    gcs_component_base* correct_parent = (parent() != nullptr ? (is_module() ? parent()->parent() : parent()) : nullptr);

    // Solves the issue of mouse rect checks not obeying parent size checks
    // Used primarily for scrolling functionality and so on
    if(!ignore_parent_check && correct_parent != nullptr){
      vec3i parent_pos = correct_parent->pos(true);
      vec3i parent_size = correct_parent->size();

      if(parent_pos.x > pos.x){
        size.x  -= parent_pos.x - pos.x;
        pos.x    = parent_pos.x;
      }

      if(parent_pos.y > pos.y){
        size.y  -= parent_pos.y - pos.y;
        pos.y    = parent_pos.y;
      }

      if(pos.x + size.x > parent_pos.x + parent_size.x)
        size.x -= (pos.x + size.x) - (parent_pos.x + parent_size.x);

      if(pos.y + size.y > parent_pos.y + parent_size.y)
        size.y -= (pos.y + size.y) - (parent_pos.y + parent_size.y);
    }

    //render->outlined_rect(pos, size, rgb(0,255,0));

    return  mpos.x > pos.x &&
            mpos.x < pos.x + size.x &&
            mpos.y > pos.y &&
            mpos.y < pos.y + size.y;
  }

  // Ensures that all 4 corners of the box are not out of bounds of the oposite side of there coordinates
  // I would consider this to be relatively expensive in some scenarios so its only ran when it needs to be.
  ALWAYSINLINE bool is_visible(){
    if(parent() == nullptr)
      return true;

    vec3i this_pos     = pos(true);
    vec3i this_size    = size();
    vec3i parent_pos   = parent()->pos(true);
    vec3i parent_size  = parent()->size();

    return  this_pos.x <= (parent_pos.x + parent_size.x) &&
            this_pos.y <= (parent_pos.y + parent_size.y) &&
            (this_pos.x + this_size.x) >= parent_pos.x &&
            (this_pos.y + this_size.y) >= parent_pos.y;
  }

  ALWAYSINLINE float visible_fraction_y(){
    if(parent() == nullptr)
      return 1.f;

    vec3i this_pos     = pos(true);
    vec3i this_size    = size();
    vec3i parent_pos   = parent()->pos(true);
    vec3i parent_size  = parent()->size();

    if(this_pos.y < parent_pos.y && this_size.y > 0){
      i32 top_delta = this_pos.y - parent_pos.y;

      if(top_delta != 0){
        float vis_delta = math::clamp((float)top_delta / (float)this_size.y, -1.f, 1.f);

        if(vis_delta < 0.f)
          return math::clamp(1.f - math::abs(vis_delta), 0.f, 1.f);
      }
    }
    else if(this_pos.y + this_size.y > parent_pos.y + parent_size.y){
      i32 bottom_delta = (parent_pos.y + parent_size.y) - this_pos.y;

      if(bottom_delta != 0){
        float vis_delta = math::clamp((float)bottom_delta / (float)this_size.y, -1.f, 1.f);

        return math::clamp(math::abs(vis_delta), 0.f, 1.f);
      }
    }

    return 1.f;
  }

  ALWAYSINLINE void alpha_start(){
    if(info()->disable_alpha_vis_fading)
      return;

    info()->old_alpha = render->alpha_mod_frac;
    render->alpha_mod_frac = visible_fraction_y();
  }

  ALWAYSINLINE void alpha_stop(){
    if(info()->disable_alpha_vis_fading)
      return;
    
    render->alpha_mod_frac = info()->old_alpha;
  }

  template<typename T>
  ALWAYSINLINE T* add(){
    gcs_component_base* comp = gcs::alloc_memory<T>();
    assert(comp != nullptr);

    comp->setup();

    children( ).push_back( comp );

    return (T*)comp;
  }

  template<typename T>
  ALWAYSINLINE T* add_module(){
    gcs_component_base* comp = gcs::alloc_memory<T>();

    if(!comp->is_module()){
      DBG("[!] GCS: add_module: failed to add module because component isnt a module\n");
      gcs::free_memory((uptr*)comp);
      return nullptr;
    }

    comp->setup();

    set_size(size() - vec3i(comp->info()->module_size, 0));

    info()->module = comp;

    return (T*)comp;
  }

  ALWAYSINLINE virtual bool is_module(){
    return false;
  }

  // When the component is created, its setup function is called so we can set information by default
  // Or carry out some processes
  ALWAYSINLINE virtual void setup(){

  }

  // This is only going to be called if is_in_rect is met, but you can override this
  // By setting info()->always_run_think to true to bypass this functionality
  virtual void think(vec3i pos, vec3i size){

  }

  // Called always before any render clipping takes place
  // Normally useful for things like windows where you want to draw out of the clipping
  // But not have to mess with the entire box (cheap hack but it works fine)
  virtual void draw_anywhere(vec3i pos, vec3i size){

  }

  // Called when a frame is drawn
  virtual void draw(vec3i pos, vec3i size){

  }

  // When scene()->info()->final_draw_comp is set to a component
  // This function is called allowing you to draw above anything else
  // This function also ignores input freezing since its mostly used for
  // Dropdowns, colour pickers, text input, etc
  // WARNING: Ensure to restore final_draw_comp to a nullptr after usage
  virtual void draw_last(vec3i pos, vec3i size){

  }

  // Called when this component is going to be processed
  virtual void pre_run_child(){

  }

  // Called when a child is about to be ran
  ALWAYSINLINE virtual void run_child(gcs_component_base* comp){
    if(comp == nullptr)
      return;

    comp->run();
  }

  // Can be used for component related hacks
  virtual void pre_run(){

  }

  // Can be used for component related hacks
  virtual void post_run(){

  }

  bool run(){
    if(!call_hook(GCS_HOOK_SHOULD_ENABLE))
      return false;

    pre_run();
    call_hook(GCS_HOOK_PRE_PAINT);

    // render component
    {
      vec3i _pos  = pos(true);
      vec3i _size = size();

      // If we have a module attached to this component
      // We must calculate its data before the first run of rendering is made
      // If we don't do this the first frame will not be setup correctly and will cause a visual bug
      // For 1 frame
      if(module() != nullptr && info()->tick_count == 0)
      {
        gcs_component_base* current_module = module();

        i32 mod_size = current_module->info()->module_size;
        current_module->info()->enable_override_auto_positioning();
        current_module->set_pos(vec3i(_size.x, 0));
        current_module->set_size(vec3i(mod_size, _size.y));
      }

      // Used so we can draw anywhere, should be considered a bad function to use
      // Apart from using for things like windows and so on
      if(call_hook(GCS_HOOK_SHOULD_PAINT))
        draw_anywhere(_pos, _size);

      render->push_clip_region(_pos + info()->clip_pos_offset, _size + info()->clip_size_offset + (module() != nullptr ? vec3i(module()->size().x, 0) + module()->info()->clip_size_offset : vec3i()));
      {
        if(info()->tick_count == 0 || info()->always_run_think || is_in_rect())
          think(_pos, _size);

        if(call_hook(GCS_HOOK_SHOULD_PAINT))
          draw(_pos, _size);

        call_hook(GCS_HOOK_MAIN_PAINT);

        //render->outlined_rect(_pos, _size, rgb(0,255,0));

        info()->tick_count++;

        // handle module (if we have one)
        if(module() != nullptr){
          gcs_component_base* current_module = module();

          bool should_render = true;
          if(!current_module->info()->render_on_list.empty()){
            for(bool* render_on_ptr : current_module->info()->render_on_list){
              if(render_on_ptr == nullptr)
                continue;

              if(!*render_on_ptr){
                should_render = false;
                break;
              }
            }
          }

          if(should_render){
            current_module->set_parent(this);
            current_module->set_scene(scene());

            _pos  = current_module->pos(true);
            _size = current_module->size();

            if(current_module->info()->tick_count == 0 || current_module->info()->always_run_think || current_module->is_in_rect())
              current_module->think(_pos, _size);

            current_module->draw(_pos, _size);

            current_module->info()->tick_count++;
          }
        }
      }

      // render children
      {
        gcs_scene*          _scene  = scene();
        gcs_component_info* _info   = info();

        _info->auto_positioning_offset = _info->auto_positioning_padding + vec3i(1, _info->auto_positioning_override_height + 1);

        for(u32 i = 0; i < children().size(); i++){
          gcs_component_base* comp = children()[i];

          if(comp == nullptr || !comp->info()->enabled)
            continue;

          comp->set_parent(this);
          comp->set_scene(_scene);

          // handle if we should render this component
          {
            bool should_render = true;

            if(!comp->info()->render_on_list.empty()){
              for(bool* render_on_ptr : comp->info()->render_on_list){
                if(render_on_ptr == nullptr)
                  continue;

                if(!*render_on_ptr){
                  should_render = false;
                  break;
                }
              }

              if(!should_render)
                continue;
            }

            //if(comp->info()->render_on != nullptr)
            //  if(!*comp->info()->render_on)
            //    continue;

            if(!comp->call_hook(GCS_HOOK_SHOULD_ENABLE))
              continue;
          }

          comp->alpha_start();

          comp->pre_run_child();

          vec3i old_size = comp->size();
          if(_info->auto_positioning && !comp->info()->override_auto_positioning)
            comp->set_size(vec3i((size().x - (_info->auto_positioning_offset.x * 2)) + old_size.x, old_size.y));

          // We only want object vis checking when we have auto positioning enabled to conserve resources if they use any
          if(!_info->auto_positioning || comp->info()->override_auto_positioning || comp->is_visible())
            run_child(comp);

          if(_info->auto_positioning && !comp->info()->override_auto_positioning){
            _info->auto_positioning_offset.y += comp->size().y + _info->auto_positioning_padding.y;
            comp->set_size(old_size);
          }

          comp->alpha_stop();
        }

        // calculate the total rendering size
        {
          if(module() != nullptr){
            gcs_component_base* current_module = module();

            current_module->info()->total_render_height = _info->auto_positioning_offset.y - info()->auto_positioning_override_height;
          }

          info()->total_render_height = _info->auto_positioning_offset.y - info()->auto_positioning_override_height;
        }
      }
      render->pop_clip_region();
    }

    call_hook(GCS_HOOK_POST_PAINT, info()->paint_param);
    post_run();

    // We are root frame
    // Handle last component drawing for things like dropdowns, colour pickers etc
    if(parent() == nullptr && scene() != nullptr && scene()->info()->final_draw_comp != nullptr){
      gcs_scene_info* _info = scene()->info();
      gcs_component_base* _comp = _info->final_draw_comp;

      _info->last_input_frozen = _info->input_frozen;

      // Fake the fact that our input is not frozen for this element, because its most likely the case
      // That we will freeze all other input in the GUI anyway so we want this to be exceptional
      _info->input_frozen = false;
      {
        _comp->draw_last(_comp->pos(true), _comp->size());
      }
      _info->input_frozen = _info->last_input_frozen;
    }

    return true;
  }

};