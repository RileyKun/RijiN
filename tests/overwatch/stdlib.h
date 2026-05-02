//#define assert(exp) if ((exp) == false) { _assert(__FUNCTION__); }

int wide_str_len(wchar_t* w_string);
int wide_to_multi(wchar_t* w_string, char* c_string);

float atan2f3(float y, float x);
float __cdecl atanf2(float xx);

#define COMPILE_MATH

#ifdef COMPILE_MATH
#define ABS(x) (x < 0 ? x * -1 : x)

#define	PI 3.14159265358979323846f

#define DEG2RAD(a) ((a) * (PI / 180.0f))
#define RAD2DEG(a) ((a) * (180.0f / PI))

#define FLT_MAX 3.402823466e+38F
#endif