#pragma once

#define GCS_MAX_SCENES 8
#define GCS_STACK_ALLOCATE_BLOCK_SIZE 8

class gcs_scene;
namespace gcs{
  // scenes
  static gcs_scene** scenes                   = nullptr;
  static u32         scenes_size              = 0;

  // memory stacking (used so we can keep track of all memory in the gui)
  static uptr** memory_stack                  = nullptr;
  static u32    memory_stack_size             = 0;
  static u32    memory_stack_allocation_size  = 0;

  template < typename T = u8 >
  inline T* alloc_memory(u32 len = 0){
    if(memory_stack_size + 1 >= memory_stack_allocation_size){
      u32     new_size    = memory_stack_allocation_size + GCS_STACK_ALLOCATE_BLOCK_SIZE;
      uptr**  new_buffer  = (uptr**)malloc(new_size * sizeof(memory_stack));

      if(memory_stack != nullptr){
        memcpy(new_buffer, memory_stack, memory_stack_allocation_size * sizeof(memory_stack));
        free(memory_stack);
        memory_stack = nullptr;
      }

      memory_stack_allocation_size = new_size;
      memory_stack                 = new_buffer;

      //DBG("[!] GCS: memory_stack: increased to %i\n", memory_stack_allocation_size);
    }

    //DBG("[+] GCS: alloc_memory: %i %i\n", memory_stack_size + 1, len > 0 ? len : sizeof(T));

    return (T*)( memory_stack[ memory_stack_size++ ] = len > 0 ? (uptr*)new T[ len ] : (uptr*)new T( ) );
  }

  inline void free_memory(uptr* p){
    if(memory_stack == nullptr || p == nullptr)
      return;

    for(u32 i = 0; i < memory_stack_size; i++){
      uptr* entry = memory_stack[i];

      if(entry != p)
        continue;

      delete p;

      memory_stack_size--;
      for(u32 ii = i; ii < memory_stack_size; ii++)
        memory_stack[ii] = memory_stack[ii + 1];

      memory_stack[memory_stack_size] = nullptr;
      break;
    }
  }

  inline void free_memory_stack(){
    if(memory_stack == nullptr)
      return;

    for(u32 i = 0; i < memory_stack_size; i++){
      uptr* entry = memory_stack[i];

      if(entry == nullptr)
        continue;

      delete entry;
      memory_stack[i] = nullptr;
    }

    free(memory_stack);
    memory_stack                  = nullptr;
    memory_stack_size             = 0;
    memory_stack_allocation_size  = 0;
  }

  template<typename T>
  T* add_scene(){
    if(scenes == nullptr)
      scenes = gcs::alloc_memory<gcs_scene*>(1);
    else{
      gcs_scene* new_scenes = (gcs_scene*)gcs::alloc_memory<gcs_scene*>(scenes_size + 1);
      assert(new_scenes != nullptr);

      memcpy(new_scenes, scenes, sizeof(gcs_scene*) * scenes_size);

      void* old_scenes_ptr = scenes;
      scenes = (gcs_scene**)new_scenes;

      gcs::free_memory(old_scenes_ptr);
    }

    T* scene = gcs::alloc_memory<T>();

    // Invoke the scenes setup system
    if(!scene->setup()){
      gcs::free_memory((void*)scene);
      return nullptr;
    }

    scenes[scenes_size++] = scenes_size;

    return scene;
  }

  bool render_scene( gcs_scene* scene, vec3i mouse_pos, c_font_base* font, c_font_base* emoji_font = nullptr );
}

// base scene
#include "gcs_scene.h"

// components
#include "components/gcs_component_base.h"
#include "components/gcs_component_canvas.h"
#include "components/gcs_component_window.h"
#include "components/gcs_component_button.h"
#include "components/gcs_component_scrollbar.h"
#include "components/gcs_component_text.h"
#include "components/gcs_component_toggle.h"
#include "components/gcs_component_sidebar.h"
#include "components/gcs_component_dropdown.h"
#include "components/gcs_component_slider.h"
#include "components/gcs_component_test.h"
#include "components/gcs_component_group.h"
#include "components/gcs_component_group_host.h"
#include "components/gcs_component_hotkey.h"
#include "components/gcs_component_label.h"
#include "components/gcs_component_divider.h"
#include "components/gcs_component_cheat_select_option.h"
#include "components/gcs_component_colour_picker.h"
#include "components/gcs_component_log_window.h"

#include "gcs_helper.h"

#if defined(INTERNAL_CHEAT)
#include "base/gcs_info_panel.h"
#include "base/gcs_menu.h"
#endif

