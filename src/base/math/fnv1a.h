#pragma once

#define FNV1A_BASIS_64   0xcbf29ce484222325ULL
#define FNV1A_PRIME_64   0x100000001b3ULL

// https://en.wikipedia.org/wiki/Fowler–Noll–Vo hash function
#define HASH_RT(str)   FNV1A_STR_RT(str)
#define HASH_RTL(str)  FNV1A_STR_RT(str, true)
#define HASH(str)      FNV1A_STR_CT(str)
#define WHASH_RT(wstr) FNV1A_WSTR_RT(wstr)
#define WHASH_RTL(wstr) FNV1A_WSTR_RT(wstr, true)
#define WHASH(wstr)    FNV1A_WSTR_CT(wstr)

#define HASH_RT64(str)   FNV1A_STR_RT_64(str)
#define HASH_RTL64(str)  FNV1A_STR_RT_64(str, true)
#define HASH64(str)      FNV1A_STR_CT_64(str)
#define WHASH_RT64(wstr) FNV1A_WSTR_RT_64(wstr)
#define WHASH_RTL64(wstr) FNV1A_WSTR_RT_64(wstr, true)
#define WHASH64(wstr)    FNV1A_WSTR_CT_64(wstr)

// Macro to handle the lowercase
#define FNV1A_HANDLE_LOWER_CASE(c) (lowercase ? ((c >= 'A' && c <= 'Z') ? (c | (1 << 5)) : c) : c)

// FNV1A_STR_RT - Runtime (loop-based)
ALWAYSINLINE inline u32 FNV1A_STR_RT(const i8* buf, bool lowercase = false, u32 ret = 0x811C9DC5) {
  while (*buf) {
    i8 c = *buf++;
    ret ^= FNV1A_HANDLE_LOWER_CASE(c);
    ret *= 0x01000193;
  }
  return ret;
}

// FNV1A_STR_CT - Immediate Compile time (recursive)
consteval u32 FNV1A_STR_CT(i8* buf, bool lowercase = false, u32 ret = 0x811C9DC5) {
  return buf[0] == 0 ? ret : FNV1A_STR_CT(&buf[1], lowercase, (ret ^ FNV1A_HANDLE_LOWER_CASE(buf[0])) * 0x01000193);
}

// FNV1A_WSTR_RT - Runtime (loop-based)
ALWAYSINLINE inline u32 FNV1A_WSTR_RT(const wchar_t* buf, bool lowercase = false, u32 ret = 0x811C9DC5) {
  while (*buf) {
    wchar_t c = *buf++;
    ret ^= FNV1A_HANDLE_LOWER_CASE(c);
    ret *= 0x01000193;
  }
  return ret;
}

// FNV1A_WSTR_CT - Immediate Compile time (recursive)
consteval u32 FNV1A_WSTR_CT(wchar_t* buf, bool lowercase = false, u32 ret = 0x811C9DC5) {
  return buf[0] == 0 ? ret : FNV1A_WSTR_CT(&buf[1], lowercase, (ret ^ FNV1A_HANDLE_LOWER_CASE(buf[0])) * 0x01000193);
}

// FNV1A_RT - Runtime with length (loop-based)
ALWAYSINLINE inline u32 FNV1A_RT(const i8* buf, uptr len, bool lowercase = false, u32 ret = 0x811C9DC5) {
  while (len--) {
    i8 c = *buf++;
    ret ^= FNV1A_HANDLE_LOWER_CASE(c);
    ret *= 0x01000193;
  }
  return ret;
}

ALWAYSINLINE inline u32 FNV1A_BINARY(const u8* buf, u32 len, bool lowercase = false, u32 ret = 0x811C9DC5) {
   while (len--) {
    ret ^= *buf++;
    ret *= 0x01000193;
  }
  return ret;
}



// FNV1A_CT - Immediate Compile time with length (recursive)
consteval u32 FNV1A_CT(i8* buf, uptr len, bool lowercase = false, u32 ret = 0x811C9DC5) {
  return len <= 0 ? ret : FNV1A_CT(&buf[1], --len, lowercase, (ret ^ FNV1A_HANDLE_LOWER_CASE(buf[0])) * 0x01000193);
}

// FNV1A_STR_RT_64 - Runtime (loop-based)
ALWAYSINLINE inline u64 FNV1A_STR_RT_64(const i8* buf, bool lowercase = false, u64 ret = FNV1A_BASIS_64) {
  while (*buf) {
    i8 c = *buf++;
    ret ^= FNV1A_HANDLE_LOWER_CASE(c);
    ret *= FNV1A_PRIME_64;
  }
  return ret;
}

// FNV1A_STR_CT_64 - Immediate Compile time (recursive)
consteval u64 FNV1A_STR_CT_64(i8* buf, bool lowercase = false, u64 ret = FNV1A_BASIS_64) {
  return buf[0] == 0 ? ret : FNV1A_STR_CT_64(&buf[1], lowercase, (ret ^ FNV1A_HANDLE_LOWER_CASE(buf[0])) * FNV1A_PRIME_64);
}

// FNV1A_WSTR_RT_64 - Runtime (loop-based)
ALWAYSINLINE inline u64 FNV1A_WSTR_RT_64(const wchar_t* buf, bool lowercase = false, u64 ret = FNV1A_BASIS_64) {
  while (*buf) {
    wchar_t c = *buf++;
    ret ^= FNV1A_HANDLE_LOWER_CASE(c);
    ret *= FNV1A_PRIME_64;
  }
  return ret;
}

// FNV1A_WSTR_CT_64 - Immediate Compile time (recursive)
consteval u64 FNV1A_WSTR_CT_64(wchar_t* buf, bool lowercase = false, u64 ret = FNV1A_BASIS_64) {
  return buf[0] == 0 ? ret : FNV1A_WSTR_CT_64(&buf[1], lowercase, (ret ^ FNV1A_HANDLE_LOWER_CASE(buf[0])) * FNV1A_PRIME_64);
}

// FNV1A_RT_64 - Runtime with length (loop-based)
ALWAYSINLINE inline u64 FNV1A_RT_64(const i8* buf, uptr len, bool lowercase = false, u64 ret = FNV1A_BASIS_64) {
  while (len--) {
    i8 c = *buf++;
    ret ^= FNV1A_HANDLE_LOWER_CASE(c);
    ret *= FNV1A_PRIME_64;
  }
  return ret;
}

// FNV1A_CT_64 - Immediate Compile time with length (recursive)
consteval u64 FNV1A_CT_64(i8* buf, uptr len, bool lowercase = false, u64 ret = FNV1A_BASIS_64) {
  return len <= 0 ? ret : FNV1A_CT_64(&buf[1], --len, lowercase, (ret ^ FNV1A_HANDLE_LOWER_CASE(buf[0])) * FNV1A_PRIME_64);
}

// Windows has its own "EPOCH time". So we'll fetch it from here and then convert into it a unix-timestamp.
ALWAYSINLINE inline u32 get_kernel_time() {
  u32 high1, low, high2;
  do{
    u32 addr_high1 = XOR32_IMPORTANT_ONLY(0x7FFE0018);
    u32 addr_low   = XOR32_IMPORTANT_ONLY(0x7FFE0014);
    u32 addr_high2 = XOR32_IMPORTANT_ONLY(0x7FFE001C);

    high1 = *(volatile u32*)addr_high1;
    low   = *(volatile u32*)addr_low;
    high2 = *(volatile u32*)addr_high2;
  }while (high1 != high2);

  u64 filetime = ((u64)high1 << 32) | low;
  return (u32)((filetime - XOR32_IMPORTANT_ONLY(116444736000000000ULL)) / XOR32_IMPORTANT_ONLY(10000000));
}

//INLINED_PROTECTION
#if defined(INTERNAL_CHEAT)
// Yea this fucking sucks.
constexpr u32 fetch_cookie_hash(u32 line) {
  switch (line % 128) {
    case 1: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE1");
    case 2: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE2");
    case 3: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE3");
    case 4: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE4");
    case 5: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE5");
    case 6: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE6");
    case 7: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE7");
    case 8: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE8");
    case 9: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE9");
    case 10: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE10");
    case 11: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE11");
    case 12: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE12");
    case 13: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE13");
    case 14: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE14");
    case 15: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE15");
    case 16: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE16");
    case 17: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE17");
    case 18: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE18");
    case 19: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE19");
    case 20: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE20");
    case 21: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE21");
    case 22: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE22");
    case 23: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE23");
    case 24: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE24");
    case 25: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE25");
    case 26: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE26");
    case 27: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE27");
    case 28: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE28");
    case 29: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE29");
    case 30: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE30");
    case 31: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE31");
    case 32: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE32");
    case 33: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE33");
    case 34: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE34");
    case 35: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE35");
    case 36: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE36");
    case 37: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE37");
    case 38: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE38");
    case 39: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE39");
    case 40: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE40");
    case 41: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE41");
    case 42: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE42");
    case 43: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE43");
    case 44: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE44");
    case 45: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE45");
    case 46: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE46");
    case 47: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE47");
    case 48: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE48");
    case 49: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE49");
    case 50: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE50");
    case 51: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE51");
    case 52: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE52");
    case 53: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE53");
    case 54: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE54");
    case 55: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE55");
    case 56: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE56");
    case 57: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE57");
    case 58: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE58");
    case 59: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE59");
    case 60: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE60");
    case 61: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE61");
    case 62: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE62");
    case 63: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE63");
    case 64: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE64");
    case 65: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE65");
    case 66: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE66");
    case 67: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE67");
    case 68: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE68");
    case 69: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE69");
    case 70: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE70");
    case 71: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE71");
    case 72: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE72");
    case 73: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE73");
    case 74: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE74");
    case 75: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE75");
    case 76: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE76");
    case 77: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE77");
    case 78: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE78");
    case 79: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE79");
    case 80: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE80");
    case 81: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE81");
    case 82: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE82");
    case 83: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE83");
    case 84: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE84");
    case 85: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE85");
    case 86: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE86");
    case 87: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE87");
    case 88: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE88");
    case 89: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE89");
    case 90: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE90");
    case 91: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE91");
    case 92: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE92");
    case 93: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE93");
    case 94: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE94");
    case 95: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE95");
    case 96: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE96");
    case 97: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE97");
    case 98: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE98");
    case 99: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE99");
    case 100: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE100");
    case 101: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE101");
    case 102: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE102");
    case 103: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE103");
    case 104: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE104");
    case 105: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE105");
    case 106: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE106");
    case 107: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE107");
    case 108: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE108");
    case 109: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE109");
    case 110: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE110");
    case 111: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE111");
    case 112: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE112");
    case 113: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE113");
    case 114: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE114");
    case 115: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE115");
    case 116: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE116");
    case 117: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE117");
    case 118: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE118");
    case 119: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE119");
    case 120: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE120");
    case 121: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE121");
    case 122: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE122");
    case 123: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE123");
    case 124: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE124");
    case 125: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE125");
    case 126: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE126");
    case 127: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE127");
    case 128: return FNV1A_STR_CT("KERNEL_COOKIE_VALUE128");
    default: return 0;
  }
}

constexpr u32 fetch_time_hash(u32 line) {
  switch (line % 128) {
    case 1: return FNV1A_STR_CT("KERNEL_TIME_VALUE1");
    case 2: return FNV1A_STR_CT("KERNEL_TIME_VALUE2");
    case 3: return FNV1A_STR_CT("KERNEL_TIME_VALUE3");
    case 4: return FNV1A_STR_CT("KERNEL_TIME_VALUE4");
    case 5: return FNV1A_STR_CT("KERNEL_TIME_VALUE5");
    case 6: return FNV1A_STR_CT("KERNEL_TIME_VALUE6");
    case 7: return FNV1A_STR_CT("KERNEL_TIME_VALUE7");
    case 8: return FNV1A_STR_CT("KERNEL_TIME_VALUE8");
    case 9: return FNV1A_STR_CT("KERNEL_TIME_VALUE9");
    case 10: return FNV1A_STR_CT("KERNEL_TIME_VALUE10");
    case 11: return FNV1A_STR_CT("KERNEL_TIME_VALUE11");
    case 12: return FNV1A_STR_CT("KERNEL_TIME_VALUE12");
    case 13: return FNV1A_STR_CT("KERNEL_TIME_VALUE13");
    case 14: return FNV1A_STR_CT("KERNEL_TIME_VALUE14");
    case 15: return FNV1A_STR_CT("KERNEL_TIME_VALUE15");
    case 16: return FNV1A_STR_CT("KERNEL_TIME_VALUE16");
    case 17: return FNV1A_STR_CT("KERNEL_TIME_VALUE17");
    case 18: return FNV1A_STR_CT("KERNEL_TIME_VALUE18");
    case 19: return FNV1A_STR_CT("KERNEL_TIME_VALUE19");
    case 20: return FNV1A_STR_CT("KERNEL_TIME_VALUE20");
    case 21: return FNV1A_STR_CT("KERNEL_TIME_VALUE21");
    case 22: return FNV1A_STR_CT("KERNEL_TIME_VALUE22");
    case 23: return FNV1A_STR_CT("KERNEL_TIME_VALUE23");
    case 24: return FNV1A_STR_CT("KERNEL_TIME_VALUE24");
    case 25: return FNV1A_STR_CT("KERNEL_TIME_VALUE25");
    case 26: return FNV1A_STR_CT("KERNEL_TIME_VALUE26");
    case 27: return FNV1A_STR_CT("KERNEL_TIME_VALUE27");
    case 28: return FNV1A_STR_CT("KERNEL_TIME_VALUE28");
    case 29: return FNV1A_STR_CT("KERNEL_TIME_VALUE29");
    case 30: return FNV1A_STR_CT("KERNEL_TIME_VALUE30");
    case 31: return FNV1A_STR_CT("KERNEL_TIME_VALUE31");
    case 32: return FNV1A_STR_CT("KERNEL_TIME_VALUE32");
    case 33: return FNV1A_STR_CT("KERNEL_TIME_VALUE33");
    case 34: return FNV1A_STR_CT("KERNEL_TIME_VALUE34");
    case 35: return FNV1A_STR_CT("KERNEL_TIME_VALUE35");
    case 36: return FNV1A_STR_CT("KERNEL_TIME_VALUE36");
    case 37: return FNV1A_STR_CT("KERNEL_TIME_VALUE37");
    case 38: return FNV1A_STR_CT("KERNEL_TIME_VALUE38");
    case 39: return FNV1A_STR_CT("KERNEL_TIME_VALUE39");
    case 40: return FNV1A_STR_CT("KERNEL_TIME_VALUE40");
    case 41: return FNV1A_STR_CT("KERNEL_TIME_VALUE41");
    case 42: return FNV1A_STR_CT("KERNEL_TIME_VALUE42");
    case 43: return FNV1A_STR_CT("KERNEL_TIME_VALUE43");
    case 44: return FNV1A_STR_CT("KERNEL_TIME_VALUE44");
    case 45: return FNV1A_STR_CT("KERNEL_TIME_VALUE45");
    case 46: return FNV1A_STR_CT("KERNEL_TIME_VALUE46");
    case 47: return FNV1A_STR_CT("KERNEL_TIME_VALUE47");
    case 48: return FNV1A_STR_CT("KERNEL_TIME_VALUE48");
    case 49: return FNV1A_STR_CT("KERNEL_TIME_VALUE49");
    case 50: return FNV1A_STR_CT("KERNEL_TIME_VALUE50");
    case 51: return FNV1A_STR_CT("KERNEL_TIME_VALUE51");
    case 52: return FNV1A_STR_CT("KERNEL_TIME_VALUE52");
    case 53: return FNV1A_STR_CT("KERNEL_TIME_VALUE53");
    case 54: return FNV1A_STR_CT("KERNEL_TIME_VALUE54");
    case 55: return FNV1A_STR_CT("KERNEL_TIME_VALUE55");
    case 56: return FNV1A_STR_CT("KERNEL_TIME_VALUE56");
    case 57: return FNV1A_STR_CT("KERNEL_TIME_VALUE57");
    case 58: return FNV1A_STR_CT("KERNEL_TIME_VALUE58");
    case 59: return FNV1A_STR_CT("KERNEL_TIME_VALUE59");
    case 60: return FNV1A_STR_CT("KERNEL_TIME_VALUE60");
    case 61: return FNV1A_STR_CT("KERNEL_TIME_VALUE61");
    case 62: return FNV1A_STR_CT("KERNEL_TIME_VALUE62");
    case 63: return FNV1A_STR_CT("KERNEL_TIME_VALUE63");
    case 64: return FNV1A_STR_CT("KERNEL_TIME_VALUE64");
    case 65: return FNV1A_STR_CT("KERNEL_TIME_VALUE65");
    case 66: return FNV1A_STR_CT("KERNEL_TIME_VALUE66");
    case 67: return FNV1A_STR_CT("KERNEL_TIME_VALUE67");
    case 68: return FNV1A_STR_CT("KERNEL_TIME_VALUE68");
    case 69: return FNV1A_STR_CT("KERNEL_TIME_VALUE69");
    case 70: return FNV1A_STR_CT("KERNEL_TIME_VALUE70");
    case 71: return FNV1A_STR_CT("KERNEL_TIME_VALUE71");
    case 72: return FNV1A_STR_CT("KERNEL_TIME_VALUE72");
    case 73: return FNV1A_STR_CT("KERNEL_TIME_VALUE73");
    case 74: return FNV1A_STR_CT("KERNEL_TIME_VALUE74");
    case 75: return FNV1A_STR_CT("KERNEL_TIME_VALUE75");
    case 76: return FNV1A_STR_CT("KERNEL_TIME_VALUE76");
    case 77: return FNV1A_STR_CT("KERNEL_TIME_VALUE77");
    case 78: return FNV1A_STR_CT("KERNEL_TIME_VALUE78");
    case 79: return FNV1A_STR_CT("KERNEL_TIME_VALUE79");
    case 80: return FNV1A_STR_CT("KERNEL_TIME_VALUE80");
    case 81: return FNV1A_STR_CT("KERNEL_TIME_VALUE81");
    case 82: return FNV1A_STR_CT("KERNEL_TIME_VALUE82");
    case 83: return FNV1A_STR_CT("KERNEL_TIME_VALUE83");
    case 84: return FNV1A_STR_CT("KERNEL_TIME_VALUE84");
    case 85: return FNV1A_STR_CT("KERNEL_TIME_VALUE85");
    case 86: return FNV1A_STR_CT("KERNEL_TIME_VALUE86");
    case 87: return FNV1A_STR_CT("KERNEL_TIME_VALUE87");
    case 88: return FNV1A_STR_CT("KERNEL_TIME_VALUE88");
    case 89: return FNV1A_STR_CT("KERNEL_TIME_VALUE89");
    case 90: return FNV1A_STR_CT("KERNEL_TIME_VALUE90");
    case 91: return FNV1A_STR_CT("KERNEL_TIME_VALUE91");
    case 92: return FNV1A_STR_CT("KERNEL_TIME_VALUE92");
    case 93: return FNV1A_STR_CT("KERNEL_TIME_VALUE93");
    case 94: return FNV1A_STR_CT("KERNEL_TIME_VALUE94");
    case 95: return FNV1A_STR_CT("KERNEL_TIME_VALUE95");
    case 96: return FNV1A_STR_CT("KERNEL_TIME_VALUE96");
    case 97: return FNV1A_STR_CT("KERNEL_TIME_VALUE97");
    case 98: return FNV1A_STR_CT("KERNEL_TIME_VALUE98");
    case 99: return FNV1A_STR_CT("KERNEL_TIME_VALUE99");
    case 100: return FNV1A_STR_CT("KERNEL_TIME_VALUE100");
    case 101: return FNV1A_STR_CT("KERNEL_TIME_VALUE101");
    case 102: return FNV1A_STR_CT("KERNEL_TIME_VALUE102");
    case 103: return FNV1A_STR_CT("KERNEL_TIME_VALUE103");
    case 104: return FNV1A_STR_CT("KERNEL_TIME_VALUE104");
    case 105: return FNV1A_STR_CT("KERNEL_TIME_VALUE105");
    case 106: return FNV1A_STR_CT("KERNEL_TIME_VALUE106");
    case 107: return FNV1A_STR_CT("KERNEL_TIME_VALUE107");
    case 108: return FNV1A_STR_CT("KERNEL_TIME_VALUE108");
    case 109: return FNV1A_STR_CT("KERNEL_TIME_VALUE109");
    case 110: return FNV1A_STR_CT("KERNEL_TIME_VALUE110");
    case 111: return FNV1A_STR_CT("KERNEL_TIME_VALUE111");
    case 112: return FNV1A_STR_CT("KERNEL_TIME_VALUE112");
    case 113: return FNV1A_STR_CT("KERNEL_TIME_VALUE113");
    case 114: return FNV1A_STR_CT("KERNEL_TIME_VALUE114");
    case 115: return FNV1A_STR_CT("KERNEL_TIME_VALUE115");
    case 116: return FNV1A_STR_CT("KERNEL_TIME_VALUE116");
    case 117: return FNV1A_STR_CT("KERNEL_TIME_VALUE117");
    case 118: return FNV1A_STR_CT("KERNEL_TIME_VALUE118");
    case 119: return FNV1A_STR_CT("KERNEL_TIME_VALUE119");
    case 120: return FNV1A_STR_CT("KERNEL_TIME_VALUE120");
    case 121: return FNV1A_STR_CT("KERNEL_TIME_VALUE121");
    case 122: return FNV1A_STR_CT("KERNEL_TIME_VALUE122");
    case 123: return FNV1A_STR_CT("KERNEL_TIME_VALUE123");
    case 124: return FNV1A_STR_CT("KERNEL_TIME_VALUE124");
    case 125: return FNV1A_STR_CT("KERNEL_TIME_VALUE125");
    case 126: return FNV1A_STR_CT("KERNEL_TIME_VALUE126");
    case 127: return FNV1A_STR_CT("KERNEL_TIME_VALUE127");
    case 128: return FNV1A_STR_CT("KERNEL_TIME_VALUE128");
    default: return 0;
  }
}

#define cookie_rotator ((__LINE__ + __COUNTER__) % 128) + 1

#define kernel_time_24h_close(){\
  volatile const u32 expire_time  = HASH("KERNEL_TIME_VALUE24H"); \
  if(get_kernel_time() > expire_time){ \
    assert_private(false && "protection error tnd 111"); \
    I(ExitProcess)(0); \
  } \
}

#define kernel_time_48h_close(){\
  volatile const u32 expire_time  = HASH("KERNEL_TIME_VALUE48H"); \
  if(get_kernel_time() > expire_time){ \
    assert_private(false && "protection error tnd 222"); \
    I(ExitProcess)(0); \
    return; \
  } \
}

#define kernel_time_1min_close(){\
  volatile const u32 expire_time  = HASH("KERNEL_TIME_VALUE1MIN"); \
  if(get_kernel_time() > expire_time){ \
    assert_private(false && "protection error tnd 333"); \
    I(ExitProcess)(0); \
    return; \
  } \
}

#define kernel_time_expired_val(){\
  constexpr volatile const u32 security_x_key   = make_xor_key(cookie_rotator); \
  constexpr volatile const u32 security_x_cookie = fetch_time_hash(cookie_rotator - 1); \
  constexpr volatile const u32 security_cur_val  = (security_x_cookie ^ security_x_key); \
  if(get_kernel_time() > security_cur_val){ \
    assert_private(false && "protection error tnd 11"); \
    return 0; \
  } \
}

#define kernel_time_expired_val(x){\
  constexpr volatile const u32 security_x_key   = make_xor_key(cookie_rotator); \
  constexpr volatile const u32 security_x_cookie = fetch_time_hash(cookie_rotator - 1); \
  constexpr volatile const u32 security_cur_val  = (security_x_cookie ^ security_x_key); \
  if(get_kernel_time() > security_cur_val){ \
    assert_private(false && "protection error tnd 22"); \
    return x; \
  } \
}

#define kernel_time_expired_return(){\
  constexpr volatile const u32 security_x_key   = make_xor_key(cookie_rotator); \
  constexpr volatile const u32 security_x_cookie = fetch_time_hash(cookie_rotator - 1); \
  constexpr volatile const u32 security_cur_val  = (security_x_cookie ^ security_x_key); \
  if(get_kernel_time() > security_cur_val){ \
    assert_private(false && "protection error tnd 33"); \
    return; \
  } \
}

#define kernel_time_expired_close(){\
  constexpr volatile const u32 security_x_key   = make_xor_key(cookie_rotator); \
  constexpr volatile const u32 security_x_cookie = fetch_time_hash(cookie_rotator - 1); \
  constexpr volatile const u32 security_cur_val  = (security_x_cookie ^ security_x_key); \
  if(get_kernel_time() > security_cur_val){ \
    assert_private(false && "protection error tnd 44"); \
    I(ExitProcess)(0); \
    return; \
  } \
}

// Is not equal.
#define cookie_block_check_return_val(){\
  constexpr volatile const u32 security_x_key   = make_xor_key(cookie_rotator); \
  constexpr volatile const u32 security_x_cookie = fetch_cookie_hash(cookie_rotator - 1); \
  constexpr volatile const u32 security_cur_val  = (security_x_cookie ^ security_x_key); \
  if(*(volatile u32*)(XOR32_IMPORTANT_ONLY(0x7FFE0330)) != security_cur_val){ \
    assert_private(false && "protection error tnd 1"); \
    return 0; \
  } \
}

#define cookie_block_check_return(){\
  constexpr volatile const u32 security_x_key   = make_xor_key(cookie_rotator); \
  constexpr volatile const u32 security_x_cookie = fetch_cookie_hash(cookie_rotator - 1); \
  constexpr volatile const u32 security_cur_val  = (security_x_cookie ^ security_x_key); \
  if(*(volatile u32*)(XOR32_IMPORTANT_ONLY(0x7FFE0330)) != security_cur_val){ \
    assert_private(false && "protection error tnd 2"); \
    return; \
  } \
}

#define cookie_block_check_return_val(val){\
  constexpr volatile const u32 security_x_key   = make_xor_key(cookie_rotator); \
  constexpr volatile const u32 security_x_cookie = fetch_cookie_hash(cookie_rotator - 1); \
  constexpr volatile const u32 security_cur_val  = (security_x_cookie ^ security_x_key); \
  if(*(volatile u32*)(XOR32_IMPORTANT_ONLY(0x7FFE0330)) != security_cur_val){ \
    assert_private(false && "protection error tnd 3"); \
    return val; \
  } \
}

// IS EQUAL.
#define cookie_block_check_return_val_ok(){\
  constexpr volatile const u32 security_x_key   = make_xor_key(cookie_rotator); \
  constexpr volatile const u32 security_x_cookie = fetch_cookie_hash(cookie_rotator - 1); \
  constexpr volatile const u32 security_cur_val  = (security_x_cookie ^ security_x_key); \
  if(*(volatile u32*)(XOR32_IMPORTANT_ONLY(0x7FFE0330)) == security_cur_val){ \
    return 0; \
  } else { assert_private(false && "protection tnd 4"); } \
}

#define cookie_block_check_return_ok(){\
  constexpr volatile const u32 security_x_key   = make_xor_key(cookie_rotator); \
  constexpr volatile const u32 security_x_cookie = fetch_cookie_hash(cookie_rotator - 1); \
  constexpr volatile const u32 security_cur_val  = (security_x_cookie ^ security_x_key); \
  if(*(volatile u32*)(XOR32_IMPORTANT_ONLY(0x7FFE0330)) == security_cur_val){ \
    return; \
  } else {assert_private(false && "protection tnd 5"); } \
}

#define cookie_block_check_return_val_ok(val){\
  constexpr volatile const u32 security_x_key   = make_xor_key(cookie_rotator); \
  constexpr volatile const u32 security_x_cookie = fetch_cookie_hash(cookie_rotator - 1); \
  constexpr volatile const u32 security_cur_val  = (security_x_cookie ^ security_x_key); \
  if(*(volatile u32*)(XOR32_IMPORTANT_ONLY(0x7FFE0330)) == security_cur_val){ \
    return val; \
  } else { assert_private(false && "protection tnd 6"); } \
}
#else

#define kernel_time_24h_close()
#define kernel_time_48h_close()
#define kernel_time_1min_close()

#define kernel_time_expired_val()
#define kernel_time_expired_val(x)
#define kernel_time_expired_return()
#define kernel_time_expired_close()

// Is not equal.
#define cookie_block_check_return_val()
#define cookie_block_check_return()

#define cookie_block_check_return_val(val)

// IS EQUAL.
#define cookie_block_check_return_val_ok()
#define cookie_block_check_return_ok()
#define cookie_block_check_return_val_ok(val)
#endif