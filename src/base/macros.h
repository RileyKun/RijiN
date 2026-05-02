#pragma once

#define EXTERN extern "C"
#define EXPORT EXTERN __attribute((dllexport))
#define NORETURN __attribute__((noreturn))

#if !defined(ALWAYSINLINE)
#define ALWAYSINLINE __attribute__((always_inline))
#endif

#if !defined(NEVERINLINE)
#define NEVERINLINE __attribute__((noinline))
#endif

#define CFLAG_Ofast 	__attribute__((optimize("Ofast")))
#define CFLAG_O3 		__attribute__((optimize("O3")))
#define CFLAG_O2 		__attribute__((optimize("O2")))
#define CFLAG_O1 		__attribute__((optimize("O1")))
#define CFLAG_O0 		__attribute__((optimize("O0")))
#define CFLAG_Os 		__attribute__((optimize("Os")))
#define CFLAG_NO_SSE __attribute__((target("no-sse")))


#define IS_HANDLE_VALID(x) ((HANDLE)x != INVALID_HANDLE_VALUE && x != nullptr)

#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x) STRINGIFY_IMPL(x)

// class name_alloc{}; class* name = &name_alloc;
#define CLASS_ALLOC(c, n) c n##_alloc{}; c* n = &n##_alloc
#define CLASS_EXTERN(c, n) extern c* n

// Padding related
#define CONCAT_IMPL(x, y) 	x##y
#define MACRO_CONCAT(x, y) 	CONCAT_IMPL(x, y)
#define PAD(size) 					u8 MACRO_CONCAT(_pad, __COUNTER__)[size];

#define DATA(offset, data) \
	struct{ \
		PAD(offset); \
		data; \
	}; \

#define VIRTUAL_TABLE_RW_IMPLEMENTS() \
	template<typename T> \
	ALWAYSINLINE T& read(u32 offset){ \
		return *(T*)((uptr)this + (u32)offset); \
	} \
  \
	template<typename T> \
	ALWAYSINLINE void write(u32 offset, T val){ \
		*(T*)((uptr)this + (u32)offset) = val; \
	} \


#define create_server_iden(n, what) \
	constexpr volatile const u32 n##_gen = HASH(what);

constexpr u32 make_xor_key(u32 key_byte) {
  return (key_byte) | (key_byte << 8) | (key_byte << 16) | (key_byte << 24);
}


// Normal read.
#define gen_read(type, name) \
	constexpr volatile const u32 _offset = HASH(name); \
	const uptr address = (uptr)this + (u32)_offset; \
	return *(type*)(address); \

#define gen_read_array(type, name, index) \
	constexpr volatile const u32 _offset = HASH(name); \
	const uptr address = (uptr)(this) + ((u32)(_offset) + ((u32)sizeof(i32) * (u32)index)); \
	return *(type*)(address); \

// Normal read but adds an offset value to the hash.
#define gen_read_offset(type, name, of) \
	constexpr volatile const u32 _offset = HASH(name) + of; \
	const uptr address = (uptr)this + (u32)_offset; \
	return *(type*)(address); \

#define gen_read_pointer(type, name) \
	constexpr volatile const u32 _offset = HASH(name); \
	const uptr address = (uptr)this + (u32)_offset; \
	return (type)(address); \

#define gen_read_other(this_p, type, name) \
	constexpr volatile const u32 _offset = HASH(name); \
	const uptr address = (uptr)this_p + (u32)_offset; \
	return *(type*)(address); \

#define gen_read_other_offset(this_p, type, name, of) \
	constexpr volatile const u32 _offset = HASH(name) + of; \
	const uptr address = (uptr)this_p + (u32)_offset; \
	return *(type*)(address); \

#define gen_read_pointer_other(this_p, type, name) \
	constexpr volatile const u32 _offset = HASH(name); \
	const uptr address = (uptr)this_p + (u32)_offset; \
	return (type)(address); \

#define create_offset_hash(name) \
	volatile u32 const offset = HASH(name); \

#define MAX_WPRINTF_BUFFER_SIZE 1024
#define FORMATA(buffer_name, buffer_size, str, ...) \
	u8 buffer_name[buffer_size]; \
	wsprintfA(buffer_name, str, __VA_ARGS__);

#define FORMATW(buffer_name, buffer_size, str, ...) \
	wchar_t buffer_name[buffer_size]; \
	wsprintfW(buffer_name, str, __VA_ARGS__);