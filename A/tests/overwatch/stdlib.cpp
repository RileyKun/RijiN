#include "link.h"

typedef NTSTATUS (*NTAPI NtAllocateVirtualMemory_fn)(HANDLE ProcessHandle, PVOID* BaseAddress, ULONG_PTR ZeroBits, PSIZE_T RegionSize, ULONG AllocationType, ULONG Protect);
typedef NTSTATUS (*NTAPI NtFreeVirtualMemory_fn)(HANDLE ProcessHandle, PVOID BaseAddress, PSIZE_T RegionSize, ULONG FreeType);

void _assert(const char* file) {
	//I(MessageBoxA)(NULL, file, "Assertion Failed", MB_ICONERROR);
	
	asm("int $3;");
}

int wide_str_len(wchar_t* w_string) {
	for (int i = 0; true; ++i) {
		if (w_string[i] == 0x0)
			return i;
	}
	
	return 0;
}

int wide_to_multi(wchar_t* w_string, char* c_string) {
	int len = wide_str_len(w_string);
	
	for (int i = 0; i < len; ++i)
		c_string[i] = (char)(w_string[i]);
	
	c_string[len] = '\0';
	
	return len;
}

//#ifdef COMPILE_MATH
// note: function inlining fucks GCC's compiling of sse instructions

float sqrtf(float x) {
	union {
		int i;
		float x;
	} u;
	u.x = x;
	u.i = (1<<29) + (u.i >> 1) - (1<<22);
	
	// Two Babylonian Steps (simplified from:)
	// u.x = 0.5f * (u.x + x/u.x);
	// u.x = 0.5f * (u.x + x/u.x);
	u.x = u.x + x/u.x;
	u.x = 0.25f*u.x + x/u.x;
	
	return u.x;
}

float floorf(float a) {
	return (float)((int)(a + 0.5f));
}

float fabsf(float a) {
	return a * (a < 0.0f ? -1.0f : 1.0f);
}

float fmaxf(float x, float y) {
	return x > y ? x : y;
}

float fminf(float x, float y) {
	return x < y ? x : y;
}

float sinf(float xx) {
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
	y = (int)j;
	
	if (j & 1) {
		j += 1;
		y += 1.0f;
	}
	
	j &= 7;
	if (j > 3) {
		sign = -sign;
		j -= 4;
	}
	
	assert(x < 8192.0f);
	
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

float cosf(float xx) {
	float x, y, z;
	int j;
	int sign;
	
	sign = 1;
	x = xx;
	if (xx < 0.0f)
		x = -xx;
	
	j = 1.27323954473516f * x;
	y = (int)j;
	
	if (j & 1) {
		j += 1;
		y += 1.0f;
	}
	
	j &= 7;
	if (j > 3) {
		sign = -sign;
		j -= 4;
	}
	
	if (j > 1)
		sign = -sign;
	
	assert(x < 8192.0f);
	
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

void sincosf(float xx, float* p_sin, float* p_cos) {
	float r_sin, r_cos;
	float c1, c2;
	float x, y, z;
	int j;
	int sign, sign2;
	
	sign = 1;
	sign2 = 1;
	x = xx;
	if (xx < 0.0f) {
		sign = -1;
		x = -xx;
	}
	
	j = 1.27323954473516f * x;
	y = (int)j;
	
	if (j & 1) {
		j += 1;
		y += 1.0f;
	}
	
	j &= 7;
	if (j > 3) {
		sign = -sign;
		sign2 = -sign2;
		j -= 4;
	}
	
	if (j > 1)
		sign2 = -sign2;
	
	assert(x < 8192.0f);
	
	x = ((x - y * 0.78515625f) - y * 2.4187564849853515625e-4f) - y * 3.77489497744594108e-8f;
	
	z = x * x;
	
	{
		c1 = ((2.443315711809948E-005f * z	
			- 1.388731625493765E-003f) * z	
			+ 4.166664568298827E-002f);
		c1 = c1 * (z * z);
		c1 = c1 - (0.5f * z);
		c1 = c1 + 1.0f;
	}
	
	{
		c2 = -1.9515295891E-4f;
		c2 = c2 * z + 8.3321608736E-3f;
		c2 = c2 * z + -1.6666654611E-1f;
		c2 = c2 * (z * x);
		c2 = c2 + x;
	}
	
	if ((j==1) || (j==2))
		r_sin = c1;
	else
		r_sin = c2;
	
	if ((j==1) || (j==2))
		r_cos = c2;
	else
		r_cos = c1;
	
	if(sign < 0)
		r_sin = -r_sin;
	
	if (j > 1)
		sign = -sign;
	
	if (sign2 < 0)
		r_cos = -r_cos;
	
	if (p_sin)
		*p_sin = r_sin;
	
	if (p_cos)
		*p_cos = r_cos;
}

float tanf(float xx) {
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

float asinf(float xx) {
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
		x = sqrtf(z);
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

float acosf(float x) {
	if (x < -0.5f) 
		return (3.141592653589793238f - 2.0f * asinf(sqrtf(0.5f * (1.0f + x))));
	
	if (x > 1.0f)
		return 0.0;
	
	if (x > 0.5f)
		return 2.0f * asinf(sqrtf(0.5f * (1.0f - x)));
	
	return 1.5707963267948966192f - asinf(x);
}

float atanf(float xx) {
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
	
	if (x > 2.414213562373095f) {
		y = 1.5707963267948966192f;
		x = -(1.0f / x);
	} else if (x > 0.4142135623730950f) {
		y = 0.7853981633974483096f;
		x = (x - 1.0f) / (x + 1.0f);
	}
	else {
		y = 0.0f;
	}
	
	z = x * x;
	y += (((8.05374449538e-2f * z
		- 1.38776856032E-1f) * z
		+ 1.99777106478E-1f) * z
		- 3.33329491539E-1f) * z * x
		+ x;
	
	if (sign < 0)
		y = -y;
	
	return y;
}

float atan2f(float y, float x) {
	float z, w;
	int code;
	
	code = 0;
	
	if (x < 0.0f)
		code = 2;
	
	if (y < 0.0f)
		code |= 1;
	
	if (x == 0.0f) {
		if (code & 1)
			return -1.5707963267948966192f;
		
		if (y == 0.0f)
			return 0.0f;
		
		return 1.5707963267948966192f;
	}
	
	if (y == 0.0f) {
		if (code & 2)
			return 3.141592653589793238f;
		
		return 0.0f;
	}
	
	switch (code) {
		default:
		case 0:
		case 1: {
			w = 0.0f;
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
	
	z = atanf(y/x);
	return w + z;
}

float powf(float x, float y) {
	union { float d; int x; } u = {x};
	u.x = (int)(y * (u.x - 1064866805) + 1064866805);
	return u.d;
}

float fmodf(float x, float y) {
	float a, b;
	const float c = x;
	
    if (0.0f > c) {
        x = -x;
    }
    if (0.0f > y) {
        y = -y;
    }
    if (y != 0 && FLT_MAX >= y && FLT_MAX >= x) {
        while (x >= y) {
            a = x / 2.0f;
            b = y;
            while (a >= b) {
                b *= 2.0f;
            }
			x -= b;
        }
    } else {
        x = 0.0f;
    }
    return 0.0f > c ? -x : x;
}
//#endif