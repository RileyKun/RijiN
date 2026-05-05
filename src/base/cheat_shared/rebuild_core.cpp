#include "../link.h"

#if defined(INTERNAL_CHEAT)
void* malloc(uptr size){
  if(size == 0)
    return nullptr;

  static void* module = utils::get_module_handle(HASH("ntdll.dll"));

  if(module == nullptr)
    return nullptr;

  static rtl_allocate_heap_fn rtl_allocate_heap = (rtl_allocate_heap_fn)utils::get_proc_address(module, HASH("RtlAllocateHeap"));

  if(rtl_allocate_heap == nullptr)
    return nullptr;

#if defined(__x86_64__)
  static __PEB* peb = (__PEB*)__readgsqword( XOR32(0x60) );
#else
  static __PEB* peb = (__PEB*)__readfsdword( XOR32(0x30) );
#endif

  if ( peb == nullptr || peb->ProcessHeap == nullptr )
    return nullptr;

  return rtl_allocate_heap( peb->ProcessHeap, HEAP_ZERO_MEMORY, (uptr)size );
}

void free(void* buffer){
  if(buffer == nullptr)
    return;

  static void* module = utils::get_module_handle(HASH("ntdll.dll"));

  if(module == nullptr)
    return;

  static rtl_free_heap_fn rtl_free_heap = (rtl_free_heap_fn)utils::get_proc_address(module, HASH("RtlFreeHeap"));

  if(rtl_free_heap == nullptr)
    return;

#if defined(__x86_64__)
  static __PEB* peb = (__PEB*)__readgsqword( XOR32(0x60) );
#else
  static __PEB* peb = (__PEB*)__readfsdword( XOR32(0x30) );
#endif

  if ( peb == nullptr || peb->ProcessHeap == nullptr )
    return;

  rtl_free_heap( peb->ProcessHeap, 0, buffer);
}

void* realloc(void* buffer, uptr size){
  free(buffer);
  
  return malloc(size);
}

void* calloc(uptr count, uptr size){
  return malloc( count * size );
}

void* operator new(uptr size){
  return malloc( size );
}

void* operator new[](uptr size){
  return malloc( size );
}

void operator delete(void* buffer){
  free( buffer );
}

void operator delete[](void* buffer){
  free( buffer );
}

void operator delete(void* buffer, uptr size){
  return operator delete( buffer );
}

ALWAYSINLINE void* memcpy(void* dst, const void* src, uptr len){
//#if defined(__x86_64__)
//  asm volatile(
//    "cld;"
//    "mov %%rcx, %%rdx;"
//    "shr $3, %%rcx;"
//    "rep movsq;"
//    "mov %%rdx, %%rcx;"
//    "and $7, %%rcx;"
//    "rep movsb;"
//    :"=D"(dst), "=S"(src), "=c"(len)
//    :"D"(dst), "S"(src), "c"(len)
//    :"memory");
//#else
//  asm volatile(
//    "cld;"
//    "mov %%ecx, %%edx;"
//    "shr $2, %%ecx;"
//    "rep movsl;"
//    "mov %%edx, %%ecx;"
//    "and $3, %%ecx;"
//    "rep movsb;"
//    :"=D"(dst), "=S"(src), "=c"(len)
//    :"D"(dst), "S"(src), "c"(len)
//    :"memory");
//#endif

  i8* d = dst;
  const i8* s = src;
  while (len--)
    *d++ = *s++;

  return dst;
}

ALWAYSINLINE wchar_t *wmemcpy(wchar_t * __restrict s1, const wchar_t * __restrict s2, size_t n)
{
  return (wchar_t*)memcpy( s1, s2, (uptr)(n * sizeof(wchar_t)));
}

ALWAYSINLINE void* memset(void* dst, i32 val, uptr len){
  asm volatile(
    "cld;"
    "rep stosb;"
    :"=D"(dst), "=a"(val), "=c"(len)
    :"D"(dst), "a"(val), "c"(len));

  return dst;
}

// https://github.com/gcc-mirror/gcc/blob/master/libgcc/memmove.c
ALWAYSINLINE void* memmove(void *dest, const void *src, size_t len)
{
  char *d = dest;
  const char *s = src;
  if (d < s)
    while (len--)
      *d++ = *s++;
  else{
    char *lasts = s + (len-1);
    char *lastd = d + (len-1);
    while (len--)
      *lastd-- = *lasts--;
  }
  return dest;
}

ALWAYSINLINE wchar_t* wmemmove (wchar_t *s1, const wchar_t *s2, size_t n)
{
  return (wchar_t *) memmove ((char *) s1, (char *) s2, n * sizeof (wchar_t));
}

ALWAYSINLINE uptr strlen(const i8* dst)
{
  i8* p = dst;
  while (*p)
    p++;

  return p - dst;
}

ALWAYSINLINE i8* strcpy(i8* dest, const i8* src){
  return memcpy(dest, src, strlen(src) + 1);
}

ALWAYSINLINE uptr wcslen(const wchar_t* dst)
{
  wchar_t* p = dst;
  while (*p)
    p++;

  return p - dst;
}

ALWAYSINLINE wchar_t* wcscpy(wchar_t* dest, const wchar_t* src){
  return memcpy(dest, src, (wcslen(src) * 2) + 1);
}

ALWAYSINLINE void* memchr(const void *s, int c, size_t n){
  size_t    i;
  char    *string;

  if (s == NULL)
    return (NULL);
  i = 0;
  string = (void *)s;
  while (i < n)
  {
    if (string[i] == (char)c)
      return (&string[i]);
    i++;
  }
  return (NULL);
}

ALWAYSINLINE int strcmp(const char *s1, const char *s2){
  int   i;

  i = 0;
  while (s1[i] && s2[i] && s1[i] == s2[i])
    i++;
  return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}

ALWAYSINLINE int memcmp (const void* str1, const void* str2, size_t count){
  register const unsigned char *s1 = (const unsigned char*)str1;
  register const unsigned char *s2 = (const unsigned char*)str2;

  while (count-- > 0)
    {
      if (*s1++ != *s2++)
    return s1[-1] < s2[-1] ? -1 : 1;
    }
  return 0;
}

ALWAYSINLINE char* strstr(const char *s1, const char *s2)
{
  const size_t len = strlen (s2);
  while (*s1)
    {
      if (!memcmp (s1, s2, len))
  return (char *)s1;
      ++s1;
    }
  return (0);
}

#endif