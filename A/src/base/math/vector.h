#pragma once

#define XY 2
#define XYZ 3
#define XYZW 4

class c_vec_int{
public:
  union {
    struct {
      i32 x, y, z, w;
    };

    struct {
      i32 r, g, b, a;
    };

    i32 raw[4];
  };
};

// IMM8 MASK LAYOUT EXAMPLE
// Use this for bit to hex: https://www.rapidtables.com/convert/number/binary-to-hex.html?x=01110001
//
//  INPUT|OUTPUT
//  WZYX | WZYX
//  0111 | 0001
//  HIGH    LOW

class c_vec3i{
public:
  union {
    struct {
      i32 x, y, z;
    };

    i32 raw[3];
  };


  ALWAYSINLINE c_vec3i store(i32 _x, i32 _y = 0, i32 _z = 0){
    x = _x;
    y = _y;
    z = _z;

    return *this;
  }

  ALWAYSINLINE c_vec3i(){
    store(0,0,0);
  }

  ALWAYSINLINE explicit c_vec3i(i32* xyz){
    store(xyz[0],xyz[1],xyz[2]);
  }

  ALWAYSINLINE explicit c_vec3i(i32 _x, i32 _y = 0, i32 _z = 0){
    store(_x, _y, _z);
  }

  ALWAYSINLINE i32& operator[](u32 i){
    return ((i32*)this)[i];
  }

  // Copy c_vec3i to xmm register
  ALWAYSINLINE void operator=(c_vec3i vec){
    store(vec.x, vec.y, vec.z);
  }

  // Tests inequality of two xmm registers, returns -1 if the same and 0 if different
  ALWAYSINLINE bool operator!=(c_vec3i vec){
    return x != vec.x || y != vec.y || z != vec.z;
  }

  ALWAYSINLINE bool operator==(c_vec3i vec){
    return x == vec.x && y == vec.y && z == vec.z;
  }

  // Add two xmm registers together
  ALWAYSINLINE c_vec3i operator+(c_vec3i vec){
    return c_vec3i(x + vec.x, y + vec.y, z + vec.z);
  }

  // Add two xmm registers together
  ALWAYSINLINE void operator+=(c_vec3i vec){
    store(x + vec.x, y + vec.y, z + vec.z);
  }

  // Subtract xmm register from current xmm register
  ALWAYSINLINE c_vec3i operator-(c_vec3i vec){
    return c_vec3i(x - vec.x, y - vec.y, z - vec.z);
  }

  // Subtract xmm register from current xmm register
  ALWAYSINLINE void operator-=(c_vec3i vec){
    store(x - vec.x, y - vec.y, z - vec.z);
  }

  // Multiplity xmm register against another xmm register
  ALWAYSINLINE c_vec3i operator*(c_vec3i vec){
    return c_vec3i(x * vec.x, y * vec.y, z * vec.z);
  }

  // Multiplity xmm register against another xmm register
  ALWAYSINLINE void operator*=(c_vec3i vec){
    store(x * vec.x, y * vec.y, z * vec.z);
  }

  // Divide xmm register against another xmm register
  ALWAYSINLINE c_vec3i operator/(c_vec3i vec){
    return c_vec3i(x / vec.x, y / vec.y, z / vec.z);
  }

  // Divide xmm register against another xmm register
  ALWAYSINLINE void operator/=(c_vec3i vec){
    store(x / vec.x, y / vec.y, z / vec.z);
  }

  // Add xmm register against a i32
  ALWAYSINLINE c_vec3i operator+(i32 in){
    return c_vec3i(x + in, y + in, z + in);
  }

  // Add xmm register against a i32
  ALWAYSINLINE void operator+=(i32 in){
    store(x + in, y + in, z + in);
  }

  // Subtract xmm register against a float
  ALWAYSINLINE c_vec3i operator-(i32 in){
    return c_vec3i(x - in, y - in, z - in);
  }

  // Subtract xmm register against a float
  ALWAYSINLINE void operator-=(i32 in){
    store(x - in, y - in, z - in);
  }

  // Multiplity xmm register against a float
  ALWAYSINLINE c_vec3i operator*(i32 in){
    return c_vec3i(x * in, y * in, z * in);
  }

  // Multiplity xmm register against a float
  ALWAYSINLINE void operator*=(i32 in){
    store(x * in, y * in, z * in);
  }

  // Divide xmm register against a float
  ALWAYSINLINE c_vec3i operator/(i32 in){
    return c_vec3i(x / in, y / in, z / in);
  }

  // Divide xmm register against a float
  ALWAYSINLINE void operator/=(i32 in){
    store(x / in, y / in, z / in);
  }
};

class c_vec3{
public:
  union {
    struct {
      float x, y, z;
    };

    float raw[3];
  };

  ALWAYSINLINE __m128 new_xmm(float x = 0.f, float y = 0.f, float z = 0.f){
    return _mm_setr_ps(x, y, z, 0.f);
  }

  ALWAYSINLINE void store(__m128 xmm){
    float* tbl = (float*)&xmm;

    x = tbl[0];
    y = tbl[1];
    z = tbl[2];
  }

  ALWAYSINLINE __m128 xmm(){
    return new_xmm(x, y, z);
  }

  ALWAYSINLINE c_vec3(){
    store(new_xmm());
  }

  ALWAYSINLINE explicit c_vec3(__m128 new_xmm){
    store(new_xmm);
  }

  ALWAYSINLINE explicit c_vec3(float* xyz){
    store(new_xmm(xyz[0], xyz[1], xyz[2]));
  }

  ALWAYSINLINE explicit c_vec3(float x, float y = 0.f, float z = 0.f){
    store(new_xmm(x, y, z));
  }

  ALWAYSINLINE float& operator[](u32 i){
    return ((float*)this)[i];
  }

  // Copy c_vec3 to xmm register
  ALWAYSINLINE void operator=(c_vec3 vec){
    store(new_xmm(vec.x, vec.y, vec.z));
  }

  // Tests inequality of two xmm registers, returns -1 if the same and 0 if different
  ALWAYSINLINE bool operator!=(c_vec3 vec){
    __m128 xmm1 = _mm_cmpeq_ps(xmm(), vec.xmm()); // (xmm == vec.xmm)[4] ? 0xFFFFFFFF : 0

    return ((u32*)&xmm1)[0] == 0 || ((u32*)&xmm1)[1] == 0 || ((u32*)&xmm1)[2] == 0 || ((u32*)&xmm1)[3] == 0;
  }

  // Tests equality of two xmm registers, returns -1 if the same and 0 if different
  ALWAYSINLINE bool operator==(c_vec3 vec){
    __m128 xmm1 = _mm_cmpeq_ps(xmm(), vec.xmm()); // (xmm == vec.xmm)[4] ? 0xFFFFFFFF : 0

    return ((u32*)&xmm1)[0] == 0xFFFFFFFF && ((u32*)&xmm1)[1] == 0xFFFFFFFF && ((u32*)&xmm1)[2] == 0xFFFFFFFF && ((u32*)&xmm1)[3] == 0xFFFFFFFF;
  }

  // Add two xmm registers together
  ALWAYSINLINE c_vec3 operator+(c_vec3 vec){
    return c_vec3(xmm() + vec.xmm());
  }

  // Add two xmm registers together
  ALWAYSINLINE void operator+=(c_vec3 vec){
    store(xmm() + vec.xmm());
  }

  // Subtract xmm register from current xmm register
  ALWAYSINLINE c_vec3 operator-(c_vec3 vec){
    return c_vec3(xmm() - vec.xmm());
  }

  // Subtract xmm register from current xmm register
  ALWAYSINLINE void operator-=(c_vec3 vec){
    store(xmm() - vec.xmm());
  }

  // Multiplity xmm register against another xmm register
  ALWAYSINLINE c_vec3 operator*(c_vec3 vec){
    return c_vec3(xmm() * vec.xmm());
  }

  // Multiplity xmm register against another xmm register
  ALWAYSINLINE void operator*=(c_vec3 vec){
    store(xmm() * vec.xmm());
  }

  // Divide xmm register against another xmm register
  ALWAYSINLINE c_vec3 operator/(c_vec3 vec){
    return c_vec3(xmm() / vec.xmm());
  }

  // Divide xmm register against another xmm register
  ALWAYSINLINE void operator/=(c_vec3 vec){
    store(xmm() / vec.xmm());
  }

  // Add xmm register against a float
  ALWAYSINLINE c_vec3 operator+(float flt){
    return c_vec3(xmm() + new_xmm(flt, flt, flt));
  }

  // Add xmm register against a float
  ALWAYSINLINE void operator+=(float flt){
    store(xmm() + new_xmm(flt, flt, flt));
  }

  // Subtract xmm register against a float
  ALWAYSINLINE c_vec3 operator-(float flt){
    return c_vec3(xmm() - new_xmm(flt, flt, flt));
  }

  // Subtract xmm register against a float
  ALWAYSINLINE void operator-=(float flt){
    store(xmm() - new_xmm(flt, flt, flt));
  }

  // Multiplity xmm register against a float
  ALWAYSINLINE c_vec3 operator*(float flt){
    return c_vec3(xmm() * new_xmm(flt, flt, flt));
  }

  // Multiplity xmm register against a float
  ALWAYSINLINE void operator*=(float flt){
    store(xmm() * new_xmm(flt, flt, flt));
  }

  // Divide xmm register against a float
  ALWAYSINLINE c_vec3 operator/(float flt){
    return c_vec3(xmm() / new_xmm(flt, flt, flt));
  }

  // Divide xmm register against a float
  ALWAYSINLINE void operator/=(float flt){
    store(xmm() / new_xmm(flt, flt, flt));
  }

  // Peforms a dot product x*x1+y*y1+z*z1
  ALWAYSINLINE float dot(c_vec3 vec){
#if defined(__AVX__) || defined(__AVX2__) || defined(__SSE4_1__) || defined(__SSE4_2__)
    __m128 xmm1 = _mm_dp_ps(xmm(), vec.xmm(), 0x71);  // x*x1+y*y1+z*z1

    return _mm_cvtss_f32(xmm1);
#else
    return x * vec.x + y * vec.y + z * vec.z;
#endif
  }

  // Peforms a dot product x*x1+y*y1
  ALWAYSINLINE float dot_2d(c_vec3 vec){
#if defined(__AVX__) || defined(__AVX2__) || defined(__SSE4_1__) || defined(__SSE4_2__)
    __m128 xmm1 = new_xmm(vec.x, vec.y, 0.f);
    xmm1        = _mm_dp_ps(xmm(), xmm1, 0x31);   // x*x1+y*y1

    return _mm_cvtss_f32(xmm1);
#else
    return x * vec.x + y * vec.y;
#endif
  }

  float dot( float* v ) const {
#if defined(__AVX__) || defined(__AVX2__) || defined(__SSE4_1__) || defined(__SSE4_2__)
    __m128 xmm1 = new_xmm(v[0], v[1], v[2]);
    xmm1        = _mm_dp_ps(xmm(), xmm1, 0x71);   // x*x1+y*y1*z*z1

    return _mm_cvtss_f32(xmm1);
#else
    return x * v[0] + y * v[1] + z * v[2];
#endif
  }

  // Performs rsqrt(x*x+y*y+z*z)*(x*x+y*y+z*z) (1 / rsqrt(x*x + y*y + z*z))
  ALWAYSINLINE float length(bool fast = false){
#if defined(__AVX__) || defined(__AVX2__) || defined(__SSE4_1__) || defined(__SSE4_2__)
    if(fast){
      // Performs inverse square root rsqrt(xmm2) * xmm2
      __m128 xmm0 = xmm();
      __m128 xmm1 = _mm_dp_ps(xmm0, xmm0, 0x71);  // x*x+y*y+z*z+w*w
      __m128 xmm2 = _mm_rsqrt_ps(xmm1);           // rsqrt(xmm1)
      xmm1        = _mm_mul_ps(xmm2, xmm1);       // xmm1*xmm2

      return _mm_cvtss_f32(xmm1);
    }
    else{
      // Peforms square root sqrt(xmm1)
      __m128 xmm0 = xmm();
      __m128 xmm1 = _mm_dp_ps(xmm0, xmm0, 0x71);  // x*x+y*y+z*z+w*w
      xmm1        = _mm_sqrt_ps(xmm1);            // sqrt(xmm1)

      return _mm_cvtss_f32(xmm1);
    }
#else
    __m128 xmm0 = _mm_set_ss(x * x + y * y + z * z);
    xmm0        = _mm_sqrt_ps(xmm0);

    return _mm_cvtss_f32(xmm0);
#endif
  }

    // Performs x*x+y*y+z*z
  ALWAYSINLINE float length_sqr(){
#if defined(__AVX__) || defined(__AVX2__) || defined(__SSE4_1__) || defined(__SSE4_2__)
    __m128 xmm0 = xmm();
    __m128 xmm1 = _mm_dp_ps(xmm0, xmm0, 0x71);  // x*x+y*y+z*z

    return _mm_cvtss_f32(xmm1);
#else
    return x * x + y * y + z * z;
#endif
  }

  // Performs sqrt x*x+y*y
  ALWAYSINLINE float length_2d(bool fast = false){
#if defined(__AVX__) || defined(__AVX2__) || defined(__SSE4_1__) || defined(__SSE4_2__)
    if(fast){
      // Performs inverse square root rsqrt(xmm2) * xmm2
      __m128 xmm1 = new_xmm(x, y, 0.f);             // [3]=x[2]=y
      __m128 xmm2 = _mm_dp_ps(xmm1, xmm1, 0x31);    // x*x+y*y
      __m128 xmm3 = _mm_rsqrt_ps(xmm2);             // rsqrt(xmm2)
      xmm2        = _mm_mul_ps(xmm3, xmm2);         // xmm3*xmm2

      return _mm_cvtss_f32(xmm2);
    }
    else{
      // Peforms square root sqrt(xmm1)
      __m128 xmm1 = new_xmm(x, y, 0.f);             // [3]=x[2]=y
      xmm1        = _mm_dp_ps(xmm1, xmm1, 0x31);    // x*x+y*y
      xmm1        = _mm_sqrt_ps(xmm1);              // sqrt(xmm1)

      return _mm_cvtss_f32(xmm1);
    }
#else
    __m128 xmm0 = _mm_set_ss(x * x + y * y);
    xmm0        = _mm_sqrt_ps(xmm0);

    return _mm_cvtss_f32(xmm0);
#endif
  }

  // Performs x*x+y*y
  ALWAYSINLINE float length_sqr_2d(){
#if defined(__AVX__) || defined(__AVX2__) || defined(__SSE4_1__) || defined(__SSE4_2__)
    __m128 xmm1 = new_xmm(x, y, 0.f);               // [3]=x[2]=y
    __m128 xmm2 = _mm_dp_ps(xmm1, xmm1, 0x31);      // x*x+y*y

    return _mm_cvtss_f32(xmm2);
#else
    return x * x + y * y;
#endif
  }

  ALWAYSINLINE float normalize(bool fast = false){
#if defined(__AVX__) || defined(__AVX2__) || defined(__SSE4_1__) || defined(__SSE4_2__)
    float len   = length(fast);
    __m128 xmm0 = xmm();
    __m128 xmm1 = _mm_dp_ps(xmm0, xmm0, 0x77);      // x*x+y*y+z*z

    if(fast){
      // Performs inverse square root rsqrt(xmm2) * xmm2
      __m128 xmm2 = _mm_rsqrt_ps(xmm1);             // rsqrt(xmm1)
      xmm1        = _mm_mul_ps(xmm2, xmm1);         // xmm1*xmm2
    }
    else{
      // Peforms square root sqrt(xmm1)
      xmm1        = _mm_sqrt_ps(xmm1);              // sqrt(xmm1)
    }

    store(_mm_div_ps(xmm0, xmm1));                  // xmm1/xmm

    return len;
#else
    float len = length(fast);

    if(len != 0.f)
      *this /= len;

    return len;
#endif
  }

  float distance(c_vec3 v, bool fast = false);
  float distance2d(c_vec3 v, bool fast = false);
  i32 distance_meter(c_vec3 v, bool fast = false);
  void normalize_angle();
};

class c_vec4{
public:
  union {
    struct {
      float x, y, z, w;
    };

    struct {
      float r, g, b, a;
    };

    struct {
      float h, s, v, hsv_unused;
    };

    float raw[4];
  };

  ALWAYSINLINE __m128 new_xmm(float x = 0.f, float y = 0.f, float z = 0.f, float w = 0.f){
    return _mm_setr_ps(x, y, z, w);
  }

  ALWAYSINLINE void store(__m128 xmm){
    float* tbl = (float*)&xmm;

    x = tbl[0];
    y = tbl[1];
    z = tbl[2];
    w = tbl[3];
  }

  ALWAYSINLINE __m128 xmm(){
    return new_xmm(x, y, z, w);
  }

  ALWAYSINLINE c_vec4(){
    store(new_xmm());
  }

  ALWAYSINLINE explicit c_vec4(__m128 new_xmm){
    store(new_xmm);
  }

  ALWAYSINLINE explicit c_vec4(float* new_vec){
    store(new_xmm(new_vec[0], new_vec[1], new_vec[2], new_vec[3]));
  }

  ALWAYSINLINE explicit c_vec4(float x, float y = 0.f, float z = 0.f, float w = 0.f){
    store(new_xmm(x, y, z, w));
  }

  ALWAYSINLINE u32 tohex(){
    return (i32)w << 24 | (i32)x << 16 | (i32)y << 8 | (i32)z;
  }

  ALWAYSINLINE u32 tohex_rgba(){
    return ((u32)x << 24) | ((u32)y << 16) | ((u32)z << 8) | (u32)w;
  }

  ALWAYSINLINE float& operator[](u32 i){
    return ((float*)this)[i];
  }

  // Copy c_vec4 to xmm register
  ALWAYSINLINE void operator=(c_vec4 vec){
    store(new_xmm(vec.x, vec.y, vec.z, vec.w));
  }

  // Tests inequality of two xmm registers, returns -1 if the same and 0 if different
  ALWAYSINLINE bool operator!=(c_vec4 vec){
    __m128 xmm1 = _mm_cmpeq_ps(xmm(), vec.xmm()); // (xmm == vec.xmm)[4] ? 0xFFFFFFFF : 0

    return ((u32*)&xmm1)[0] == 0 || ((u32*)&xmm1)[1] == 0 || ((u32*)&xmm1)[2] == 0 || ((u32*)&xmm1)[3] == 0;
  }

  // Tests equality of two xmm registers, returns -1 if the same and 0 if different
  ALWAYSINLINE bool operator==(c_vec4 vec){
    __m128 xmm1 = _mm_cmpeq_ps(xmm(), vec.xmm()); // (xmm == vec.xmm)[4] ? 0xFFFFFFFF : 0

    return ((u32*)&xmm1)[0] == -1 && ((u32*)&xmm1)[1] == -1 && ((u32*)&xmm1)[2] == -1 && ((u32*)&xmm1)[3] == -1;
  }

  // Add two xmm registers together
  ALWAYSINLINE c_vec4 operator+(c_vec4 vec){
    return c_vec4(xmm() + vec.xmm());
  }

  // Add two xmm registers together
  ALWAYSINLINE void operator+=(c_vec4 vec){
    store(xmm() + vec.xmm());
  }

  // Subtract xmm register from current xmm register
  ALWAYSINLINE c_vec4 operator-(c_vec4 vec){
    return c_vec4(xmm() - vec.xmm());
  }

  // Subtract xmm register from current xmm register
  ALWAYSINLINE void operator-=(c_vec4 vec){
    store(xmm() - vec.xmm());
  }

  // Multiplity xmm register against another xmm register
  ALWAYSINLINE c_vec4 operator*(c_vec4 vec){
    return c_vec4(xmm() * vec.xmm());
  }

  // Multiplity xmm register against another xmm register
  ALWAYSINLINE void operator*=(c_vec4 vec){
    store(xmm() * vec.xmm());
  }

  // Divide xmm register against another xmm register
  ALWAYSINLINE c_vec4 operator/(c_vec4 vec){
    return c_vec4(xmm() / vec.xmm());
  }

  // Divide xmm register against another xmm register
  ALWAYSINLINE void operator/=(c_vec4 vec){
    store(xmm() / vec.xmm());
  }

  // Add xmm register against a float
  ALWAYSINLINE c_vec4 operator+(float flt){
    return c_vec4(xmm() + new_xmm(flt, flt, flt, flt));
  }

  // Add xmm register against a float
  ALWAYSINLINE void operator+=(float flt){
    (xmm() + new_xmm(flt, flt, flt, flt));
  }

  // Subtract xmm register against a float
  ALWAYSINLINE c_vec4 operator-(float flt){
    return c_vec4(xmm() - new_xmm(flt, flt, flt, flt));
  }

  // Subtract xmm register against a float
  ALWAYSINLINE void operator-=(float flt){
    store(xmm() - new_xmm(flt, flt, flt, flt));
  }

  // Multiplity xmm register against a float
  ALWAYSINLINE c_vec4 operator*(float flt){
    return c_vec4(xmm() * new_xmm(flt, flt, flt, flt));
  }

  // Multiplity xmm register against a float
  ALWAYSINLINE void operator*=(float flt){
    store(xmm() * new_xmm(flt, flt, flt, flt));
  }

  // Divide xmm register against a float
  ALWAYSINLINE c_vec4 operator/(float flt){
    return c_vec4(xmm() / new_xmm(flt, flt, flt, flt));
  }

  // Divide xmm register against a float
  ALWAYSINLINE void operator/=(float flt){
    store(xmm() / new_xmm(flt, flt, flt, flt));
  }

  // Peforms a dot product x*x+y*y+z*z+w*w
  ALWAYSINLINE float dot(c_vec4 vec){
#if defined(__AVX__) || defined(__AVX2__) || defined(__SSE4_1__) || defined(__SSE4_2__)
    __m128 xmm1 = _mm_dp_ps(xmm(), vec.xmm(), 0xF1);  // x*x+y*y+z*z+w*w

    return _mm_cvtss_f32(xmm1);
#else
    return x * vec.x + y * vec.y + z * vec.z + w * vec.w;
#endif
  }

  // Peforms a dot product x*x+y*y
  ALWAYSINLINE float dot_2d(c_vec4 vec){
#if defined(__AVX__) || defined(__AVX2__) || defined(__SSE4_1__) || defined(__SSE4_2__)
    __m128 xmm1 = new_xmm(vec.x, vec.y, 0.f);     // [3]=x[2]=y
    xmm1        = _mm_dp_ps(xmm(), xmm1, 0x31);   // x*x+y*y

    return _mm_cvtss_f32(xmm1);
#else
    return x * vec.x + y * vec.y;
#endif
  }

  // Performs rsqrt(x*x+y*y+z*z+w*w)*(x*x+y*y+z*z+w*w) (1 / rsqrt(x*x + y*y + z*z))
  ALWAYSINLINE float length(bool fast = false){
#if defined(__AVX__) || defined(__AVX2__) || defined(__SSE4_1__) || defined(__SSE4_2__)
    if(fast){
      // Performs inverse square root rsqrt(xmm2) * xmm2
      __m128 xmm0 = xmm();
      __m128 xmm1 = _mm_dp_ps(xmm0, xmm0, 0xF1);  // x*x+y*y+z*z+w*w
      __m128 xmm2 = _mm_rsqrt_ps(xmm1);           // rsqrt(xmm1)
      xmm1        = _mm_mul_ps(xmm2, xmm1);       // xmm1*xmm2

      return _mm_cvtss_f32(xmm1);
    }
    else{
      // Peforms square root sqrt(xmm1)
      __m128 xmm0 = xmm();
      __m128 xmm1 = _mm_dp_ps(xmm0, xmm0, 0xF1);  // x*x+y*y+z*z+w*w
      xmm1        = _mm_sqrt_ps(xmm1);            // sqrt(xmm1)

      return _mm_cvtss_f32(xmm1);
    }
#else
    __m128 xmm0 = _mm_set_ss(x * x + y * y + z * z + w * w);
    xmm0        = _mm_sqrt_ps(xmm0);

    return _mm_cvtss_f32(xmm0);
#endif
  }

    // Performs x*x+y*y+z*z+w*w
  ALWAYSINLINE float length_sqr(){
#if defined(__AVX__) || defined(__AVX2__) || defined(__SSE4_1__) || defined(__SSE4_2__)
    __m128 xmm0 = xmm();
    __m128 xmm1 = _mm_dp_ps(xmm0, xmm0, 0xF1);  // x*x+y*y+z*z+w*w

    return _mm_cvtss_f32(xmm1);
#else
    return x * x + y * y + z * z + w * w;
#endif
  }

  // Performs sqrt x*x+y*y
  ALWAYSINLINE float length_2d(bool fast = false){
#if defined(__AVX__) || defined(__AVX2__) || defined(__SSE4_1__) || defined(__SSE4_2__)
    if(fast){
      // Performs inverse square root rsqrt(xmm2) * xmm2
      __m128 xmm1 = new_xmm(x, y, 0.f);             // [3]=x[2]=y
      __m128 xmm2 = _mm_dp_ps(xmm1, xmm1, 0x31);    // x*x+y*y
      __m128 xmm3 = _mm_rsqrt_ps(xmm2);             // rsqrt(xmm2)
      xmm2        = _mm_mul_ps(xmm3, xmm2);         // xmm3*xmm2

      return _mm_cvtss_f32(xmm2);
    }
    else{
      // Peforms square root sqrt(xmm1)
      __m128 xmm1 = new_xmm(x, y, 0.f);             // [3]=x[2]=y
      xmm1        = _mm_dp_ps(xmm1, xmm1, 0x31);    // x*x+y*y
      xmm1        = _mm_sqrt_ps(xmm1);              // sqrt(xmm1)

      return _mm_cvtss_f32(xmm1);
    }
#else
    __m128 xmm0 = _mm_set_ss(x * x + y * y);
    xmm0        = _mm_sqrt_ps(xmm0);

    return _mm_cvtss_f32(xmm0);
#endif
  }

  // Performs x*x+y*y
  ALWAYSINLINE float length_sqr_2d(){
#if defined(__AVX__) || defined(__AVX2__) || defined(__SSE4_1__) || defined(__SSE4_2__)
    __m128 xmm1 = new_xmm(x, y, 0.f);               // [3]=x[2]=y
    __m128 xmm2 = _mm_dp_ps(xmm1, xmm1, 0x31);      // x*x+y*y

    return _mm_cvtss_f32(xmm2);
#else
    return x * x + y * y;
#endif
  }

  ALWAYSINLINE float normalize(bool fast = false){
#if defined(__AVX__) || defined(__AVX2__) || defined(__SSE4_1__) || defined(__SSE4_2__)
    float len   = length(fast);
    __m128 xmm0 = xmm();
    __m128 xmm1 = _mm_dp_ps(xmm0, xmm0, 0xFF);      // x*x+y*y+z*z+w*w

    if(fast){
      // Performs inverse square root rsqrt(xmm2) * xmm2
      __m128 xmm2 = _mm_rsqrt_ps(xmm1);             // rsqrt(xmm1)
      xmm1        = _mm_mul_ps(xmm2, xmm1);         // xmm1*xmm2
    }
    else{
      // Peforms square root sqrt(xmm1)
      xmm1        = _mm_sqrt_ps(xmm1);              // sqrt(xmm1)
    }

    store(_mm_div_ps(xmm0, xmm1));                  // xmm1/xmm

    return len;
#else
    float len = length(fast);

    if(len != 0.f)
      *this /= len;

    return len;
#endif
  }

  void normalize_angle();
};

#define colour c_vec4
#define hsv c_vec4
#define vec3i c_vec3i
#define vec3 c_vec3
#define vec4 c_vec4

#define flt_array2clr(x) colour(x[0], x[1], x[2], x[3])

#define rgb(r,g,b) colour(r, g, b, 255)
#define rgba(r,g,b,a) colour(r, g, b, a)