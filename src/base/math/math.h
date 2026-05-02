#pragma once

// Math related libraries

#include "vector.h"
#include "matrix.h"

namespace math{
  inline std::string get_math_extension(){
#if defined(__AVX512BW__)
    return XOR("AVX512BW");
#elif defined(__AVX512CD__)
    return XOR("AVX512CD");
#elif defined(__AVX512QD__)
    return XOR("AVX512QD");
#elif defined(__AVX512F__)
    return XOR("AVX512F");
#elif defined(__AVX512VL__)
    return XOR("AVX512VL");
#elif defined(__AVX2__)
    return XOR("AVX2");
#elif defined(__AVX__)
    return XOR("AVX");
#elif defined(__SSE4_2__)
    return XOR("SSE4.2");
#elif defined(__SSE4_1__)
    return XOR("SSE4.1");
#elif defined(__SSSE3__)
    return XOR("SSSE3");
#elif defined(__SSE3__ )
    return XOR("SSE3");
#elif defined(__SSE2__)
    return XOR("SSE2");
#elif defined(__SSE__)
    return XOR("SSE");
#else
    return XOR("MATHUNK");
#endif
  }

	inline float time( const bool ms = false, const bool very_accurate = false ){
    if(very_accurate){
      static LARGE_INTEGER freq;

      if(freq.LowPart == 0 && freq.HighPart == 0)
        QueryPerformanceFrequency(&freq);

      LARGE_INTEGER count;
      QueryPerformanceCounter(&count);

      if(count.QuadPart == 0 || freq.QuadPart == 0)
        return 0.f;

      float time = ((float)count.QuadPart / (float)freq.QuadPart);

      if(ms)
        time *= 1000.f;

      return time;
    }
    else{
      float tc = (float)GetTickCount();

      if(!ms && tc > 0.f)
        tc /= 1000.f;

      return tc;
    }
  }

  inline vec3i mouse_pos(void* target_window = nullptr){
    POINT pos;
    I(GetCursorPos)(&pos);

    if(target_window != nullptr)
      I(ScreenToClient)((HWND)target_window, &pos);

    return vec3i(pos.x, pos.y);
  }

  // half distance of circle
  consteval float pi(){
    return 3.14159265358979323846f;
  }

  // full distance of circle
  consteval float pi2(){
    return 6.28318530717958647692f;
  }

  // Converts a radian to a degree r * (180/pi)
  inline float rad_2_deg(float r){
    return r * (180.f / pi());
  }

  // Converts a degree to a radian d * (pi/180)
  inline float deg_2_rad(float d){
    return d * (pi() / 180.f);
  }

  inline float abs(float a){
    return a >= 0.f ? a : -a;
  }

  inline double abs(double a){
    return a >= 0.f ? a : -a;
  }

  inline vec3 abs(vec3 a){
    for(u32 i = 0; i < 3; i++)
      a[i] = abs(a[i]);

    return a;
  }

  inline i32 abs(i32 a){
    return a >= 0 ? a : -a;
  }

  inline float biggest(float a, float b){
    return a > b ? a : b;
  }

  inline float smallest(float a, float b){
    return a < b ? a : b;
  }

  inline i32 biggest(i32 a, i32 b){
    return a > b ? a : b;
  }

  inline i32 smallest(i32 a, i32 b){
    return a < b ? a : b;
  }

  inline float clamp(float val, float min, float max){
    return val < min ? min : (val > max ? max : val);
  }

  inline i32 clamp(i32 val, i32 min, i32 max){
    return val < min ? min : (val > max ? max : val);
  }

  inline colour clamp(colour col){
    for(u8 i = 0; i < 4; i++)
      col[i] = clamp(col[i], 0.f, 255.f);

    return col;
  }

  inline float floor(float x){
#if defined(__AVX__) || defined(__AVX2__) || defined(__SSE2__) || defined(__SSE3__) || defined(__SSE4_1__) || defined(__SSE4_2__)
    __m128 xmm0 = _mm_set_ss(x);
    __m128 xmm1 = _mm_set_ss(1.0f);

    __m128 xmm2 = _mm_cvtepi32_ps(_mm_cvttps_epi32(xmm0));
    __m128 xmm3 = _mm_sub_ps(xmm2, _mm_and_ps(_mm_cmplt_ps(xmm0, xmm2), xmm1));

    return _mm_cvtss_f32(xmm3);
#else
    i32 xi = (i32)x;
    return (x >= 0) ? xi : (x == xi ? x : xi - 1);
#endif
  }

  inline void floor(vec3& v){
    v.x = math::floor(v.x);
    v.y = math::floor(v.y);
    v.z = math::floor(v.z);
  }

  inline float ceil(float x){
#if defined(__AVX__) || defined(__AVX2__) || defined(__SSE4_1__) || defined(__SSE4_2__)
    __m128 xmm0 = _mm_set_ss(x);
    __m128 xmm1 = _mm_ceil_ps(xmm0);

    return _mm_cvtss_f32(xmm1);
#else
    i32 xi = (i32)x;
    return (x >= 0) ? ((x == xi) ? xi : xi + 1) : ((x == xi) ? xi + 1 : xi);
#endif
  }

  inline float trunc(float x){
    return floor(x < 0 ? -x : x);
  }

  inline float fmod(float x, float y){
    return x - trunc(x / y) * y;
  }

  inline void ceil(vec3& v){
    if(v.x != 0.f)
      v.x = math::ceil(v.x);

    if(v.y != 0.f)
      v.y = math::ceil(v.y);

    if(v.z != 0.f)
      v.z = math::ceil(v.z);
  }

  inline vec3 transform(vec3 vec, matrix3x4 m){
    return vec3(
      vec.dot( m[0] ) + m[0][3],
      vec.dot( m[1] ) + m[1][3],
      vec.dot( m[2] ) + m[2][3]
    );
  }

  inline float sqrt(float x){
    __m128 xmm0 = _mm_set_ss(x);
    xmm0        = _mm_sqrt_ps(xmm0);

    return _mm_cvtss_f32(xmm0);
  }

  inline float round( float a ) {
    float t;

    if ( a >= 0.0 ) {
      t = floor( a );
      if ( t - a <= -0.5 )
        t += 1.0;
      return t;
    }
    t = floor( -a );
    if ( t + a <= -0.5 )
      t += 1.0;

    return -t;
  }

  inline float powf(float x, float y) {
    union { float d; int x; } u = {x};
    u.x = (int)(y * (u.x - 1064866805) + 1064866805);
    return u.d;
  }

  inline float normalize_angle(float ang){
    if(ang < -180.f || ang > 180.f)
      ang += (360.f * round(math::abs(ang / 360.f))) * (ang < 0.f ? 1.f : -1.f);

    return ang;
  }

  inline vec3 clamp_angles(vec3 ang){
    ang.x = math::clamp(normalize_angle(ang.x), -89.f, 89.f);
    ang.y = math::clamp(normalize_angle(ang.y), -180.f, 180.f);
    ang.z = 0.f;
    return ang;
  }

  inline vec3 round_vector(vec3 v){
    for(u32 i = 0; i <= 2; i++)
        v[i] = round(v[i]);

    return v;
  }

  inline vec3 floor_vector(vec3 v){
    for(u32 i = 0; i <= 2; i++)
      v[i] = floor(v[i]);

    return v;
  }

  inline vec3i lerp(vec3i src, vec3i dst, vec3i frac){
    return src + frac * (dst - src);
  }

  inline float lerp(float src, float dst, float frac){
    return src + frac * (dst - src);
  }

  inline i32 lerp(i32 src, i32 dst, float frac){
    return (i32)lerp((float)src, (float)dst, (float)frac);
  }

  inline colour lerp(colour src, colour dst, float frac){
    return rgba(
      lerp(src.r, dst.r, frac),
      lerp(src.g, dst.g, frac),
      lerp(src.b, dst.b, frac),
      lerp(src.a, dst.a, frac)
    );
  }

  inline colour lerp(colour src, float dst[4], float frac){
    return rgba(
      lerp(src.r, dst[0], frac),
      lerp(src.g, dst[1], frac),
      lerp(src.b, dst[2], frac),
      lerp(src.a, dst[3], frac)
    );
  }

  inline vec3 lerp(vec3 src, vec3 dst, float frac){
    return vec3(
      lerp(src.x, dst.x, frac),
      lerp(src.y, dst.y, frac),
      lerp(src.z, dst.z, frac)
    );
  }

  inline float lerp_angle(float src, float dst, float frac){
    return normalize_angle(src + frac * normalize_angle(dst - src));
  }

  inline vec3 lerp_angle(vec3 src, vec3 dst, float frac){
    return vec3(
      lerp_angle(src.x, dst.x, frac),
      lerp_angle(src.y, dst.y, frac),
      lerp_angle(src.z, dst.z, frac)
    );
  }

  inline float cos(float xx) {
    float x, y, z;
    unsigned long j;
    int sign;

    sign = 1;
    x = xx;
    if (xx < 0.0f)
      x = -xx;

    j = 1.27323954473516f * x;
    y = j;

    if (j & 1) {
      j += 1;
      y += 1.0;
    }

    j &= 7;
    if (j > 3) {
      sign = -sign;
      j -= 4;
    }

    if (j > 1)
      sign = -sign;

    if (x > 8192.0f){
      assert(false);
    }
    else
      x = ((x - y * 0.78515625f) - y * 2.4187564849853515625e-4f) - y * 3.77489497744594108e-8f;

    z = x * x;
    if ((j==1) || (j==2)) {
      y = -1.9515295891E-4f;
      y = y * z + 8.3321608736E-3f;
      y = y * z + -1.6666654611E-1f;
      y = y * (z * x);
      y = y + x;
    } else {
      y = ((2.443315711809948E-005f * z
        - 1.388731625493765E-003f) * z
        + 4.166664568298827E-002f);
      y = y * (z * z);
      y = y - (0.5f * z);
      y = y + 1.0f;
    }

    if (sign < 0)
      y = -y;

    return y;
  }

  inline float sin(float xx) {
    float x, y, z;
    unsigned long j;
    int sign;

    sign = 1;
    x = xx;
    if (xx < 0.0f) {
      sign = -1;
      x = -xx;
    }

    j = 1.27323954473516f * x;
    y = j;

    if (j & 1) {
      j += 1;
      y += 1.0;
    }

    j &= 7;
    if (j > 3) {
      sign = -sign;
      j -= 4;
    }

    if (x > 8192.0f){
      assert(false);
    }
    else
      x = ((x - y * 0.78515625f) - y * 2.4187564849853515625e-4f) - y * 3.77489497744594108e-8f;

    z = x * x;
    if ((j==1) || (j==2)) {
      y = ((2.443315711809948E-005f * z
        - 1.388731625493765E-003f) * z
        + 4.166664568298827E-002f);
      y = y * (z * z);
      y = y - (0.5f * z);
      y = y + 1.0f;
    } else {
      y = -1.9515295891E-4f;
      y = y * z + 8.3321608736E-3f;
      y = y * z + -1.6666654611E-1f;
      y = y * (z * x);
      y = y + x;
    }

    if(sign < 0)
      y = -y;

    return y;
  }

  inline void sin_cos(float xx, float* p_sin, float* p_cos) {
    if(p_sin != nullptr)
      *p_sin = sin(xx);

    if(p_cos != nullptr)
      *p_cos = cos(xx);
  }

  inline float tan(float xx) {
    float x, y, z, zz;
    unsigned long j;
    int sign;

    if (xx < 0.0) {
      x = -xx;
      sign = -1;
    } else {
      x = xx;
      sign = 1;
    }

    j = 1.27323954473516f * x;
    y = j;

    if (j & 1) {
      j += 1;
      y += 1.0f;
    }

    z = ((x - y * 0.78515625f) - y * 2.4187564849853515625e-4f) - y * 3.77489497744594108e-8f;
    zz = z * z;

    if (x > 1.0e-4f) {
      y =
      (((((9.38540185543E-3f * zz
      + 3.11992232697E-3f) * zz
      + 2.44301354525E-2f) * zz
      + 5.34112807005E-2f) * zz
      + 1.33387994085E-1f) * zz
      + 3.33331568548E-1f) * zz * z + z;
    } else {
      y = z;
    }

    if (j & 2)
      y = -1.0f / y;

    if (sign < 0)
      y = -y;

    return y;
  }

  inline float asin(float xx) {
    float a, x, z;
    int sign, flag;

    x = xx;

    if (x > 0) {
      sign = 1;
      a = x;
    } else {
      sign = -1;
      a = -x;
    }

    if (a > 1.0f)
      return 0.0f;

    if (a < 1.0e-4) {
      z = a;

      if (sign < 0)
        z = -z;

      return z;
    }

    if (a > 0.5f) {
      z = 0.5f * (1.0f - a);
      x = sqrt(z);
      flag = 1;
    } else {
      x = a;
      z = x * x;
      flag = 0;
    }

    z =
    ((((4.2163199048E-2f * z
    + 2.4181311049E-2f) * z
    + 4.5470025998E-2f) * z
    + 7.4953002686E-2f) * z
    + 1.6666752422E-1f) * z * x
    + x;

    if (flag != 0) {
      z = z + z;
      z = 1.5707963267948966192f - z;
    }

    if (sign < 0)
      z = -z;

    return z;
  }

  inline float acos(float x) {
    if (x < -0.5f)
      return (3.141592653589793238f - 2.0f * asin(sqrt(0.5f * (1.0f + x))));

    if (x > 1.0f)
      return 0.0;

    if (x > 0.5f)
      return 2.0f * asin(sqrt(0.5f * (1.0f - x)));

    return 1.5707963267948966192f - asin(x);
  }

  inline float atan(float xx) {
    float x, y, z;
    int sign;
    x = xx;

    if (xx < 0.0f) {
      sign = -1;
      x = -xx;
    } else {
      sign = 1;
      x = xx;
    }

    if (x > 2.414213562373095) {
      y = 1.5707963267948966192f;
      x = -(1.0f / x);
    } else if (x > 0.4142135623730950) {
      y = 0.7853981633974483096f;
      x = (x - 1.0f) / (x + 1.0f);
    }
    else {
      y = 0.0;
    }

    z = x * x;
    y += ((( 8.05374449538e-2f * z
      - 1.38776856032E-1f) * z
      + 1.99777106478E-1f) * z
      - 3.33329491539E-1f) * z * x
      + x;

    if (sign < 0)
      y = -y;

    return y;
  }

  inline float atan2(float y, float x) {
    float z, w;
    int code;

    code = 0;

    if( x < 0.0f )
      code = 2;

    if( y < 0.0f )
      code |= 1;

    if( x == 0.0 ) {
      if (code & 1)
        return -1.5707963267948966192f;

      if (y == 0.0f)
        return 0.0f;

      return 1.5707963267948966192f;
    }

    if (y == 0.0f) {
      if( code & 2 )
        return 3.141592653589793238f;

      return 0.0f;
    }

    switch (code) {
      default:
      case 0:
      case 1: {
        w = 0.0;
        break;
      }
      case 2: {
        w = 3.141592653589793238f;
        break;
      }
      case 3: {
        w = -3.141592653589793238f;
        break;
      }
    }

    z = atan(y/x);
    return w + z;
  }

  inline float fract(float x){
    return x - (i32)x;
  }

  inline colour hsv_2_rgb(hsv hsv_col){
    return rgb(
      clamp((hsv_col.v * lerp(1.f, clamp(abs(fract(hsv_col.h + 1.f) * 6.f - 3.f) - 1.f, 0.f, 1.f), hsv_col.s)) * 255.f, 0.f, 255.f),
      clamp((hsv_col.v * lerp(1.f, clamp(abs(fract(hsv_col.h + 0.6666666f) * 6.f - 3.f) - 1.f, 0.f, 1.f), hsv_col.s)) * 255.f, 0.f, 255.f),
      clamp((hsv_col.v * lerp(1.f, clamp(abs(fract(hsv_col.h + 0.3333333f) * 6.f - 3.f) - 1.f, 0.f, 1.f), hsv_col.s)) * 255.f, 0.f, 255.f)
    );
  }

  inline hsv rgb_2_hsv(colour col){
    hsv hsv_out;

    float min = smallest(smallest(col.r, col.g), col.b);
    float max = biggest(biggest(col.r, col.g), col.b);

    hsv_out.v = max > 0.f ? math::clamp(max / 255.f, 0.f, 1.f) : 0.f;

    float mm_delta = max - min;

    if(mm_delta <= 0.f || max <= 0.f)
      return hsv_out;

    hsv_out.s = math::clamp(mm_delta / max, 0.f, 1.f);

    if(col.r >= max)
      hsv_out.h = ((col.g - col.b) / mm_delta) * 60.f;
    else if(col.g >= max)
      hsv_out.h = (2.f + (col.b - col.r) / mm_delta) * 60.f;
    else
      hsv_out.h = (4.f + (col.r - col.g) / mm_delta) * 60.f;

    if(hsv_out.h < 0.f)
      hsv_out.h += 360.f;

    if(hsv_out.h > 0.f)
      hsv_out.h = math::clamp(hsv_out.h / 360.f, 0.f, 1.f);

    return hsv_out;
  }

  inline colour get_pixel_colour(vec3i pos){
    HDC dc = GetDC(nullptr);
    COLORREF hex = GetPixel(dc, pos.x, pos.y);
    ReleaseDC(nullptr, dc);

    return rgb(GetRValue(hex), GetGValue(hex), GetBValue(hex));
  }

  inline bool get_box_points(vec3 min, vec3 max, matrix3x4 m, float scale, vec3* p){
    if(p == nullptr)
      return false;

    scale = math::clamp(scale - 0.05f, 0.01f, 1.0f);

    p[0] = math::transform(vec3(min.x, min.y, min.z) * scale, m);
    p[1] = math::transform(vec3(min.x, max.y, min.z) * scale, m);
    p[2] = math::transform(vec3(max.x, max.y, min.z) * scale, m);
    p[3] = math::transform(vec3(max.x, min.y, min.z) * scale, m);
    p[4] = math::transform(vec3(max.x, max.y, max.z) * scale, m);
    p[5] = math::transform(vec3(min.x, max.y, max.z) * scale, m);
    p[6] = math::transform(vec3(min.x, min.y, max.z) * scale, m);
    p[7] = math::transform(vec3(max.x, min.y, max.z) * scale, m);

    return true;
  }

  inline void angle_matrix(vec3 angles, matrix3x4& m){
    float sr, sp, sy, cr, cp, cy;

    // Normalize angles before they come here.
    angles.x = normalize_angle(angles.x);
    angles.y = normalize_angle(angles.y);
    angles.z = normalize_angle(angles.z);

    math::sin_cos(math::deg_2_rad(angles.y), &sy, &cy);
    math::sin_cos(math::deg_2_rad(angles.x), &sp, &cp);
    math::sin_cos(math::deg_2_rad(angles.z), &sr, &cr);

    m[0][0] = cp * cy;
    m[1][0] = cp * sy;
    m[2][0] = -sp;

    float crcy = cr * cy;
    float crsy = cr * sy;
    float srcy = sr * cy;
    float srsy = sr * sy;

    m[0][1] = sp * srcy - crsy;
    m[1][1] = sp * srsy + crcy;
    m[2][1] = sr * cp;

    m[0][2] = sp * crcy + srsy;
    m[1][2] = sp * crsy - srcy;
    m[2][2] = cr * cp;

    m[0][3] = 0.f;
    m[1][3] = 0.f;
    m[2][3] = 0.f;
  }

  inline vec3 cross(vec3 src, vec3 other) {
     return vec3(
         src.y * other.z - src.z * other.y,
         src.z * other.x - src.x * other.z,
         src.x * other.y - src.y * other.x
     );
  }

  inline float dot(vec3 src, vec3 other){
    return src.x * other.x + src.y * other.y + src.z * other.z;
  }

  // returns if a number is not divisable by 2 to a whole number
  inline bool odd(i32 num){
    return num == 0 ? 0 : num % 2 == 1;
  }

  // returns if a number is not divisable by 2 to a whole number
  inline bool odd(u32 num){
    return num == 0 ? 0 : num % 2 == 1;
  }
}