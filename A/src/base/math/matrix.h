#pragma once

//#define MAKE_MATRIX(a,b) typedef float matrix##a##x##b[a][b]

#define MAKE_MATRIX(a, b) class matrix##a##x##b{ \
public: \
  inline float* operator[]( i32 i ){ \
    return m[ i ]; \
  } \
  \
  inline const float* operator[]( i32 i ) const{ \
    return m[ i ]; \
  } \
  \
  float m[ a ][ b ];\
};

MAKE_MATRIX(1,1);
MAKE_MATRIX(1,2);
MAKE_MATRIX(1,3);
MAKE_MATRIX(1,4);
MAKE_MATRIX(2,1);
MAKE_MATRIX(2,2);
MAKE_MATRIX(2,3);
MAKE_MATRIX(2,4);
MAKE_MATRIX(3,1);
MAKE_MATRIX(3,2);
MAKE_MATRIX(3,3);
MAKE_MATRIX(3,4);
MAKE_MATRIX(4,1);
MAKE_MATRIX(4,2);
MAKE_MATRIX(4,3);
MAKE_MATRIX(4,4);