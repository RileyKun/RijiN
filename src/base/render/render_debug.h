#pragma once
#define RENDER_DEBUG_SETUP() if(render_debug == nullptr) render_debug = new c_render_debug; render_debug->render = render;

#define MAX_OBJECTS 32767 * 2
#define DISPOSE_SLEEP_TIME 1.f
class c_base_render_debug_obj{
public:
  virtual ~c_base_render_debug_obj(){

  }

  c_render_base* render;
  bool  is_disposed;
  float dispose_sleep_time;
  float life_time;

  virtual bool paint(){
    return false;
  }
};

class c_render_debug_obj_line : public c_base_render_debug_obj{
public:
  vec3   pos1;
  vec3   pos2;
  colour col;
  bool   outline;

  bool paint() override;
};

class c_render_debug_obj_box : public c_base_render_debug_obj{
public:
  vec3 pos;
  i32  w;
  i32  h;
  colour col;
  bool filled;

  bool paint() override;
};

class c_render_debug_obj_gradient_box : public c_base_render_debug_obj{
public:
  vec3 pos;
  i32 w;
  i32 h;
  colour top_left;
  colour top_right;
  colour bottom_left;
  colour bottom_right;

  bool paint() override;
};

class c_render_debug_obj_circle : public c_base_render_debug_obj{
public:
  vec3 pos;
  float radius;
  float sides;
  colour col;
  bool filled;

  bool paint() override;
};

class c_render_debug_obj_3dbox : public c_base_render_debug_obj{
public:
  vec3 pos;
  vec3 mins;
  vec3 maxs;
  vec3 angles;
  colour outline_col;
  colour face_col;

  bool paint() override;
};

class c_render_debug_obj_font : public c_base_render_debug_obj{
public:
  vec3    pos;
  colour  face_col;
  i32     offset;
  i8      buf[1028];

  bool paint() override;
};

class c_render_debug_obj_2d_font : public c_base_render_debug_obj{
public:
  vec3    pos;
  colour  face_col;
  i32     offset;
  i8      buf[1028];

  bool paint() override;
};

class c_render_debug{
public:
  c_render_base*     render;
  void*              ws2_pointer;
  c_base_render_debug_obj* objects[MAX_OBJECTS]; // Pointers list to render objects we have.
  c_base_render_debug_obj* object_disposal[MAX_OBJECTS]; // Dead object pointers to be free'd.
  c_font_base* font = nullptr;

  void register_ws2(void* p){
    assert(p != nullptr); // Catch mistakes.
    ws2_pointer = p;
  }

  void setup_fonts(){
    font = render->create_font(WXOR(L"Courier New"), FW_MEDIUM, DEFAULT_QUALITY);
  }

  bool world2screen(vec3 world, vec3i& screen){
    assert(ws2_pointer != nullptr);
    #if defined(__x86_64__)
      return utils::call_fastcall64<bool, vec3i&>(ws2_pointer, (void*)&world, screen);
    #else
      return utils::call_cdecl<bool, vec3, vec3i&>(ws2_pointer, world, screen);
    #endif
  }

  bool add_object(c_base_render_debug_obj* obj){
    if(obj == nullptr)
      return false;

    obj->render = render;
    for(i32 i = 0; i < MAX_OBJECTS; i++){
      if(objects[i] == nullptr){
        objects[i] = obj;
        return true;
      }
    }

    delete obj;
    DBG("[-] render_debug object overflow!\n");
    return false;
  }

  // Disposal system will wait a certain amount of time before deleting.
  // This is important in the event d3d hook is multi-threaded. Which in most cases it is.
  // Though, I haven't had any crashing issues with the time at zero. (Meaning no deletion delay)
  // Look at the MAX_OBJECTS value to see how much objects can be created at the same time.
  bool add_disposal_object(c_base_render_debug_obj* obj){
    if(obj == nullptr)
      return false;

    obj->is_disposed        = true;
    obj->dispose_sleep_time = math::time(false, true) + DISPOSE_SLEEP_TIME;
    for(i32 i = 0; i < MAX_OBJECTS; i++){
      if(object_disposal[i] == nullptr){
        object_disposal[i] = obj;
        return true;
      }
    }

    delete obj;
    DBG("[-] render_debug cannot safely delete object data! DELETING!!\n");
    return false;
  }

  bool delete_object(c_base_render_debug_obj* obj){
    if(obj == nullptr)
      return false;

    if(obj->is_disposed)
      return true;

    for(u32 i = 0; i < MAX_OBJECTS; i++){
      if(objects[i] == obj){
        add_disposal_object(obj);
        objects[i] = nullptr;
        return true;
      }
    }

    return false;
  }

  // Call in d3d hook
  void render_objects(){

    process_dispose();
    float time = math::time(false, true);
    for(i32 i = 0; i < MAX_OBJECTS; i++){
      c_base_render_debug_obj* obj = objects[i];
      if(obj == nullptr)
        continue;

      if(obj->life_time <= time)
        continue;

      obj->paint();
    }
  }

  // Free memory on objects marked for deletion.
  void process_dispose(){
    float time = math::time(false, true);

    // Find objects that need to be cleaned up.
    for(i32 i = 0; i < MAX_OBJECTS; i++){
      c_base_render_debug_obj* obj = objects[i];
      if(obj == nullptr)
        continue;

      if(obj->life_time > time)
        continue;

      // This should never happen. Maybe it was marked in another thread?
      if(obj->is_disposed)
        continue;

      delete_object(obj);
    }

    // Free the object's memory and remove it from the disposal list.
    for(i32 i = 0; i < MAX_OBJECTS; i++){
      c_base_render_debug_obj* obj = object_disposal[i];
      if(obj == nullptr)
        continue;

      if(obj->dispose_sleep_time > time)
        continue;

      object_disposal[i] = nullptr;
      delete obj;
    }
  }

  void draw_static_line(vec3 pos1, vec3 pos2, colour col, bool outline){
    static c_render_debug_obj_line* line = new c_render_debug_obj_line;

    if(col.w <= 0)
      return;

    line->pos1      = pos1;
    line->pos2      = pos2;
    line->col       = col;
    line->outline   = outline;
    line->render    = render;
    line->paint();
  }

  void draw_static_box(vec3 pos, i32 w, i32 h, colour col, bool filled){
    static c_render_debug_obj_box* box = new c_render_debug_obj_box;

    if(col.w <= 0)
      return;

    box->pos       = pos;
    box->w         = w;
    box->h         = h;
    box->col       = col;
    box->filled    = filled;
    box->render    = render;
    box->paint();
  }

  void draw_static_box_gradient(vec3 pos, i32 w, i32 h, colour top_left, colour top_right, colour bottom_left, colour bottom_right){
    static c_render_debug_obj_gradient_box* box = new c_render_debug_obj_gradient_box;

    box->pos          = pos;
    box->w            = w;
    box->h            = h;
    box->top_left     = top_left;
    box->top_right    = top_right;
    box->bottom_left  = bottom_left;
    box->bottom_right = bottom_right;
    box->render       = render;
    box->paint();
  }

  void draw_static_circle(vec3 pos, float radius, float sides, colour col, bool filled){
    static c_render_debug_obj_circle* circle = new c_render_debug_obj_circle;

    if(col.w <= 0)
      return;

    circle->pos       = pos;
    circle->radius    = radius;
    circle->sides     = sides;
    circle->col       = col;
    circle->filled    = filled;
    circle->render    = render;
    circle->paint();
  }

  void draw_static_3dbox(vec3 pos, vec3 mins, vec3 maxs, vec3 angles, colour face_col, colour outline_col){
    static c_render_debug_obj_3dbox* box = new c_render_debug_obj_3dbox;

    box->pos         = pos;
    box->mins        = mins;
    box->maxs        = maxs;
    box->angles      = angles;
    box->outline_col = outline_col;
    box->face_col    = face_col;
    box->render      = render;
    box->paint();
  }

  void draw_line(vec3 pos1, vec3 pos2, colour col, bool outline, float life_time){
    c_render_debug_obj_line* line = new c_render_debug_obj_line;

    if(col.w <= 0)
      return;

    line->pos1      = pos1;
    line->pos2      = pos2;
    line->col       = col;
    line->life_time = math::time(false, true) + life_time;
    line->outline   = outline;
    line->render    = render;
    add_object(line);
  }

  void draw_box(vec3 pos, i32 w, i32 h, colour col, bool filled, float life_time){
    c_render_debug_obj_box* box = new c_render_debug_obj_box;

    if(col.w <= 0)
      return;

    box->pos       = pos;
    box->w         = w;
    box->h         = h;
    box->col       = col;
    box->filled    = filled;
    box->life_time = math::time(false, true) + life_time;
    box->render    = render;
    add_object(box);
  }

  void draw_box_gradient(vec3 pos, i32 w, i32 h, colour top_left, colour top_right, colour bottom_left, colour bottom_right, float life_time){
    c_render_debug_obj_gradient_box* box = new c_render_debug_obj_gradient_box;

    box->pos          = pos;
    box->w            = w;
    box->h            = h;
    box->top_left     = top_left;
    box->top_right    = top_right;
    box->bottom_left  = bottom_left;
    box->bottom_right = bottom_right;
    box->life_time    = math::time(false, true) + life_time;
    box->render       = render;
    add_object(box);
  }

  void draw_circle(vec3 pos, float radius, float sides, colour col, bool filled, float life_time){
    c_render_debug_obj_circle* circle = new c_render_debug_obj_circle;

    if(col.w <= 0)
      return;

    circle->pos       = pos;
    circle->radius    = radius;
    circle->sides     = sides;
    circle->col       = col;
    circle->filled    = filled;
    circle->life_time = math::time(false, true) + life_time;
    add_object(circle);
  }

  void draw_3dbox(vec3 pos, vec3 mins, vec3 maxs, vec3 angles, colour face_col, colour outline_col, float life_time){
    c_render_debug_obj_3dbox* box = new c_render_debug_obj_3dbox;

    box->pos         = pos;
    box->mins        = mins;
    box->maxs        = maxs;
    box->angles      = angles;
    box->outline_col = outline_col;
    box->face_col    = face_col;
    box->life_time   = math::time(false, true) + life_time;

    add_object(box);
  }

  void draw_3dbox(vec3 pos, colour face_col, float life_time){
    c_render_debug_obj_3dbox* box = new c_render_debug_obj_3dbox;

    box->pos         = pos;
    box->mins        = vec3(-2.f, -2.f, -2.f);
    box->maxs        = vec3(2.f, 2.f, 2.f);
    box->angles      = vec3(0.f, 0.f, 0.f);
    box->outline_col = colour(0, 0, 0, 255);
    box->face_col    = face_col;
    box->life_time   = math::time(false, true) + life_time;

    add_object(box);
  }

  void draw_text(vec3 pos, colour face_col, i8* text, i32 offset, float life_time){
    assert(strlen(text) < 1028);
    if(text == nullptr)
      return;

    c_render_debug_obj_font* box = new c_render_debug_obj_font;

    box->offset       = offset;
    box->pos          = pos;
    box->face_col     = face_col;
    box->life_time    = math::time(false, true) + life_time;
    strcpy(box->buf, text);

    add_object(box);
  }

  void draw_2d_text(vec3 pos, colour face_col, i8* text, i32 offset, float life_time){
    assert(strlen(text) < 1028);
    if(text == nullptr)
      return;

    c_render_debug_obj_2d_font* box = new c_render_debug_obj_2d_font;

    box->offset       = offset;
    box->pos          = pos;
    box->face_col     = face_col;
    box->life_time    = math::time(false, true) + life_time;
    strcpy(box->buf, text);

    add_object(box);
  }
};

CLASS_EXTERN(c_render_debug, render_debug);