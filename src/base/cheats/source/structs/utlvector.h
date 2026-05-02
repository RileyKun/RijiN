#pragma once

class c_interface_mem_alloc{
public:
  FORCEINLINE void free(void* mem){
    STACK_CHECK_START;
    utils::virtual_call<5, void, void*>(this, mem);
    STACK_CHECK_END;
  }
};

template< class T, class I = i32 >
class c_utlmemory{
public:
  T& operator[ ]( I i ) {
    return memory_ptr[ i ];
  }

  NEVERINLINE void purge(){
    static c_interface_mem_alloc* mem_alloc = *(c_interface_mem_alloc**)utils::get_proc_address(utils::get_module_handle(HASH("tier0.dll")), HASH("g_pMemAlloc"));

    if(memory_ptr != nullptr)
      mem_alloc->free(memory_ptr);

    memory_ptr = nullptr;
  }

  T*  memory_ptr = nullptr;
  i32 allocation_count = 0;
  i32 grow_size = 0;
};

template <class T>
void destruct( T* mem ) {
  mem->~T( );
}

template< class T, class A = c_utlmemory< T > >
class c_utlvector {
  typedef A c_allocator;
public:
  NEVERINLINE void remove_all( ) {
    for( i32 i = 0; i < size; i++)
      destruct( &memory[ i ] );

    size = 0;
  }

  c_allocator memory;
  i32         size = 0;
  T*          elements = nullptr;
};