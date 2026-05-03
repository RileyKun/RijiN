void CrossProduct(float* v1, float* v2, float* cross);
void NormalizeAngle(float* angle);
void ClampAngle(float* angle);
float VectorPitch(float* vec);
float VectorYaw(float* vec);
void VectorAngles(float* vec, float* pitch, float* yaw);
void AngleForward(float pitch, float yaw, float* vec);
void AngleRight(float pitch, float yaw, float roll, float* vec);
void AngleUp(float pitch, float yaw, float roll, float* vec);
void AngleVectors(float pitch, float yaw, float roll, float* vec0, float* vec1, float* vec2);

enum {
	F_Y = 0,
	F_Z,
	F_X
};

enum {
	VC_W = 0,
	VC_X,
	VC_Y,
	VC_Z
};

class Vector {
public:
	INLINE Vector() {
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}
	INLINE Vector(float x0, float y0, float z0) {
		x = x0;
		y = y0;
		z = z0;
	}
	INLINE Vector(float* a) {
		x = a[F_X];
		y = a[F_Y];
		z = a[F_Z];
	}
	
	// conversion
	INLINE operator float*() {
		return (float*)this;
	}
	
	// logic
	INLINE bool operator==(Vector in) {
		return x == in.x && y == in.y && z == in.z;
	}
	INLINE bool operator!=(Vector in) {
		return x != in.x || y != in.y || z != in.z;
	}
	INLINE bool operator!=(float f) {
		return x != f || y != f || z != f;
	}
	
	// arithmetic
	INLINE Vector operator+(Vector in) {
		return Vector(x + in.x, y + in.y, z + in.z);
	}
	INLINE Vector operator-(Vector in) {
		return Vector(x - in.x, y - in.y, z - in.z);
	}
	INLINE Vector operator*(Vector in) {
		return Vector(x * in.x, y * in.y, z * in.z);
	}
	INLINE Vector operator*(float m) {
		return Vector(x * m, y * m, z * m);
	}
	INLINE Vector operator/(Vector in) {
		return Vector(x / in.x, y / in.y, z / in.z);
	}
	INLINE Vector operator/(float d) {
		return Vector(x / d, y / d, z / d);
	}
	INLINE void operator+=(Vector in) {
		x += in.x;
		y += in.y;
		z += in.z;
	}
	INLINE void operator-=(Vector in) {
		x -= in.x;
		y -= in.y;
		z -= in.z;
	}
	INLINE void operator*=(Vector in) {
		x *= in.x;
		y *= in.y;
		z *= in.z;
	}
	INLINE void operator*=(float m) {
		x *= m;
		y *= m;
		z *= m;
	}
	INLINE void operator/=(Vector in) {
		x /= in.x;
		y /= in.y;
		z /= in.z;
	}
	INLINE void operator/=(float d) {
		x /= d;
		y /= d;
		z /= d;
	}
	INLINE float length_sqr() {
		return (x * x) + (y * y) + (z * z);
	}
	INLINE float length_2d_sqr() {
		return (x * x) + (y * y);
	}
	INLINE float length() {
		return sqrtf(length_sqr());
	}
	INLINE float length_2d() {
		return sqrtf(length_2d_sqr());
	}
	INLINE Vector normal() {
		float len = length();
		
		if (len > 0.0f)
			return Vector(*this) / len;
		else
			return Vector(*this);
	}
	INLINE float dot(Vector in) {
		return (x * in.x) + (y * in.y) + (z * in.z);
	}
	INLINE Vector cross(Vector a) {
		return Vector(y * a.z - z * a.y, z * a.x - x * a.z, x * a.y - y * a.x);
	}
	INLINE float pitch() {
		return VectorPitch((float*)this);
	}
	INLINE float yaw() {
		return VectorYaw((float*)this);
	}
	INLINE Vector forward() {
		Vector result;
		AngleForward((*this)[0], (*this)[1], (float*)(&result));
		
		return result;
	}
	INLINE Vector right() {
		Vector result;
		AngleRight((*this)[0], (*this)[1], (*this)[2], (float*)(&result));
		
		return result;
	}
	INLINE Vector up() {
		Vector result;
		AngleUp((*this)[0], (*this)[1], (*this)[2], (float*)(&result));
		
		return result;
	}
	
	// assignment
	INLINE float& operator[](int i) {
		return ((float*)this)[i];
	}
	INLINE Vector operator=(float f) {
		x = f;
		y = f;
		z = f;
		
		return *this;
	}
	INLINE Vector operator=(float* a) {
		x = a[F_X];
		y = a[F_Y];
		z = a[F_Z];
		
		return *this;
	}
	
	float x;
	float y;
	float z;
};

class Quat {
public:
	float x;
	float y;
	float z;
	float w;
};

/*class matrix3x4 {
public:
	matrix3x4() {
	}
	matrix3x4(matrix3x4& in) {
		for (int i = 0; i < 4; ++i) {
			m[i][0] = in[i][0];
			m[i][1] = in[i][1];
			m[i][2] = in[i][2];
		}
	}
	void init(Vector pos, Vector angles) {
		set_column(angles.forward(), 0);
		set_column(angles.right(), 1);
		set_column(angles.up(), 2);
		set_column(pos, 3);
	}
	inline float* operator[](int i) {
		return m[i];
	}
	inline Vector get_column(int i) {
		return Vector(m[0][i], m[1][i], m[2][i]);
	}
	inline void set_column(Vector in, int i) {
		m[F_X][i] = in[F_X];
		m[F_Y][i] = in[F_Y];
		m[F_Z][i] = in[F_Z];
	}
	inline Vector get_transform() {
		return get_column(3);
	}
	inline Vector rotate(Vector in) {
		Vector v1(m[F_X][0], m[F_X][1], m[F_X][2]);
		Vector v2(m[F_Y][0], m[F_Y][1], m[F_Y][2]);
		Vector v3(m[F_Z][0], m[F_Z][1], m[F_Z][2]);

		return Vector(in.dot(v1), in.dot(v2), in.dot(v3));
	}
	inline Vector transform(Vector in) {
		Vector rot = rotate(in);

		return get_transform() + rot;
	}

	float m[3][4];
};

class __attribute__((aligned(16))) matrix3x4a : public matrix3x4 {
public:
	
};*/

bool IsWithinCylinder(Vector start, Vector end, float length, float radius, Vector test);
bool RayIntersectSphere(Vector org, Vector center, Vector dir, float radius_sqr, float* tmin);
bool RayIntersectCapsule(Vector org, Vector dir, Vector min, Vector max, float radius, Vector* point);
bool RayIntersectBox(Vector origin, Vector dir, Vector min, Vector max, float* length);
void QuaternionNormalize(Quat* quat);
//void MatrixQuaternion(matrix3x4* matrix, Quat* quat);
//void QuaternionMatrix(Quat quat, matrix3x4* matrix);