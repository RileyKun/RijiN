// You can't use std::vectors, std::string, std::wstring, std::deques in your structure.
template <typename T> class c_dynamic_indexed_memory{
public:

  c_dynamic_indexed_memory(i32 count = 32){
    init(count);
  }

  c_dynamic_indexed_memory(void* buf, u32 size_of_entry, u32 size_of_buf, i32 _item_count = -1){
    if(buf == nullptr || size_of_entry >= size_of_buf || size_of_entry < 4 || size_of_buf < 4)
      return;
    
    // This will cause issues.
    if(size_of_entry != sizeof(T)){
      assert(false && "size_of_entry mismatch with (T) size");
      return;
    } 

    // Get the count of entries.
    i32 count = (i32)(size_of_buf / size_of_entry);
    if(count == 0){
      assert(false && "failed to resolve entry count");
      return;
    }

    u32 max_new_amount = 8;
    init(count + max_new_amount);
    if(data == nullptr)
      return;

    if(_item_count >= 0)
      item_count = _item_count;
    else{
      // The data will be valid to whatever was passed from the 'buffer' paramater.
      // item_count is considered to be the same as max_item_count now when creating memory this way.
      item_count = count;
    }

    memcpy(data, buf, size_of_buf);
  }

  void init(i32 count = 32){
    // Current count of items in list.
    item_count     = 0;

    // If the data had to shrunken or resized this is how much it'll increase or decrease by.
    push_size      = 128;

    // How much entries can be stored before needing a resize.
    max_item_count = count;

    // Track the original size of what each entry should be.
    struct_size    = sizeof(T);
    
    // The current size in bytes of the data block.
    size = sizeof(T) * max_item_count;

    data = malloc(size);
    memset(data, 0, size);
  }

  void add_entry(T entry){
    if(data == nullptr)
      return;

    assert(item_count >= 0);
    memcpy(&data[item_count], &entry, sizeof(T));
    item_count++;
    resize();
  }

  T* get_entry(u32 pos){
    if(data == nullptr || pos >= item_count)
      return nullptr;

    assert(pos >= 0);
    return (T*)&data[pos];
  }

  T* end(){
    return get_entry(item_count - 1);
  }

  bool pop(){
    return remove(item_count - 1);
  }

  bool pop_front(){
    return remove(0);
  }

  bool remove(u32 pos){
    if(data == nullptr)
      return false;

    if(item_count == 0 || pos >= item_count)
      return false;

    assert(struct_size > 0);
    if(pos == item_count - 1){
      memset(&data[item_count - 1], 0, struct_size);
      item_count--;
      return true;
    }

    for(i32 i = pos; i < item_count - 1; i++){
      if(pos == item_count)
        break;
      
      data[i] = data[i + 1];
    }

    memset(&data[item_count - 1], 0, struct_size);
    item_count--;
    return true;
  }

  void resize(){
    if(data == nullptr)
      return;

    assert(item_count >= 0);
    assert(max_item_count >= 0);
    assert(struct_size >= 4);
    assert(push_size >= 1);

    // Perform resize.
    u32   old_count  = max_item_count;
    u32   old_size   = size;
    void* old_data   = data;

    if(item_count >= max_item_count){
      // Or automatically increase size.
      max_item_count += push_size;
      size           = max_item_count * struct_size;
    }
    else{
      if(max_item_count <= 0)
        return;

      float item_push_ratio = item_count > 0 ? item_count / push_size : 0.f;
      float max_push_ratio  = max_item_count / push_size;
      if(math::abs(item_push_ratio - max_push_ratio) <= 1.f)
        return;
    
      max_item_count -= push_size;
      if(max_item_count < push_size)
        max_item_count = push_size;

      size = max_item_count * struct_size;   
    }

    if(old_size == size)
      return;

    // Create a new block
    void* new_data_block = malloc(size);
    memcpy(new_data_block, old_data, (old_size < size) ? old_size : size);

    // Swap pointer to new block.
    data = new_data_block;

    // Delete old block.
    free(old_data);
    old_data = nullptr;
  }

  i32 get_item_count(){
    return item_count;
  }

  i32 get_max_item_count(){
    return max_item_count - 1;
  }

  void* get_data(){
    return data;
  }

  i32 get_data_size(){
    return size;
  }

  // All of the functions in here check if data is nullptr.
  // So rather than have a case where they'll be reading free'd memory.
  // We do this.
  void dealloc(){
    void* old_data = data;
    data = nullptr;
    free(old_data);
    old_data = nullptr;
  }

private:
  T*    data;
  u32   item_count;
  i32   max_item_count;
  u32   size;
  u32   struct_size;
  u32   push_size;
};