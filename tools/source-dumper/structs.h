#pragma once


typedef void* (*create_interface)(i8*, i32*);

class recv_table;
class recv_prop{
public:
  i8*         name;
  i32         type;
  i32         flags;
  i32         str_buffer_size;
  bool        inside_array;
  void*       extra_data;
  recv_prop*  array;
  void*       array_len_proxy;
  void*       proxy;
  void*       data_table_proxy;
  recv_table* data_table;
  u32         offset;
  i32         element_stride;
  i32         elements;
  i8*         parent_array_name;
};

class recv_table{
public:
  recv_prop* props;
  u32        count;
  void*      decoder;
  i8*        name;
  bool       init;
  bool       main_list;
};

class client_class{
public:
  void*         create;
  void*         create_event;
  i8*           name;
  recv_table*   table;
  client_class* next;
  u32           id;
};

//#define L4D_GAME

class hl_client{
public:
  virtual void pad0();
  virtual void pad1();
  virtual void pad2();
  virtual void pad3();
  virtual void pad4();
  virtual void pad6();
  virtual void pad7();
#if !defined(LD4_GAME)
  virtual void pad8();
#endif
  virtual client_class* get_all_classes();
};

static void* get_interface_address(create_interface ci, i8* name){
  if(ci == nullptr || name == nullptr)
    return nullptr;

  i8 buf[128];
  for(u32 i = 0; i <= 128; i++){
    wsprintfA(buf, "%s%03i", name, i);

    void* result = ci(buf, 0);
    if(result != nullptr)
      return result;
  }

  return nullptr;
}