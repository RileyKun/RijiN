#pragma once

class c_memory{
public:

  ALWAYSINLINE void* data(){
    access_time = should_free ? access_time : math::time();
    return should_free ? nullptr : mem;
  }

  // Get the size of the memory allocated.
  ALWAYSINLINE u32 get_size(){
    return size;
  }

  // When was this data last accessed?
  ALWAYSINLINE float get_last_access_time(){
    return access_time;
  }

  // If we no longer need this memory then we must request it to avoid cross-thread issues.
  ALWAYSINLINE void request_release(){
    access_time = math::time();
    should_free = true;
  }

  ALWAYSINLINE bool release(){
    if(!should_free) // Nope, no request to delete it.
      return false;

    // No recent access to the pointer with in a 10 second time window.
    if(math::abs(math::time() - access_time) <= 10.f)
      return false;

    // Must be a valid.
    if(mem == nullptr)
      return false;

    free(mem); // Free the memory and reset variables.
    access_time = -1.f;
    size        = 0;
    mem         = nullptr;
    should_free = false;
    return true;
  }

  void* mem;
  u32   size;
  float access_time;
  bool  should_free;
};

// This is a total of 32kb
class c_memory_manager{
public:
  ALWAYSINLINE void setup(){
    memset(this, 0, sizeof(c_memory_manager));
    init = true;
  }

  ALWAYSINLINE void* get_data(i32 index){
    assert(init == true);
    assert(index >= 0 && index < 2048);
    return data[index].data();
  }

  ALWAYSINLINE bool create_data(i32 index, u32 size){
    assert(init == true);
    assert(index >= 0 && index < 2048);
    assert(size > 0);
    c_memory* obj = &data[index];

    // Was this marked for deletion? If that's the case unmark it
    if(obj->should_free && obj->mem != nullptr){

      // TODO: I have an idea for this. Transfer the data of this c_memory into a another table so we can
      // Create a new one of this NEW instance.
      assert(obj->get_size() <= size);

      memset(obj->mem, 0, obj->get_size());
      obj->access_time = math::time();
      obj->should_free = false;
      obj->size        = size;
      return true;
    }
    else{
      obj->mem         = malloc(size);
      obj->size        = size;
      obj->should_free = false;
      obj->access_time = math::time();
      memset(obj->mem, 0, size);;
      return true;
    }

    return false;
  }

  ALWAYSINLINE bool release_data(i32 index){
    assert(init == true);
    assert(index >= 0 && index < 2048);

    data[index].request_release();
    return true;
  }

  NEVERINLINE void process(){
    float time = math::time();
    if(next_process_time > time)
      return;

    bytes_allocated = 0;
    for(i32 i = 0; i < 2048; i++){
      if(data[i].release())
        continue;

      bytes_allocated += data[i].get_size();
    }

    next_process_time = time + 1.f;
  }

  ALWAYSINLINE i32 get_allocated_bytes(){
    return bytes_allocated;
  }
private:
  c_memory data[2048];
  bool init;
  float next_process_time;
  i32 bytes_allocated;
};