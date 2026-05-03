#include "link.h"

void CrossProduct(float* v1, float* v2, float* cross) {
	cross[F_X] = (v1[F_Y] * v2[F_Z]) - (v1[F_Z] * v2[F_Y]);
	cross[F_Y] = (v1[F_Z] * v2[F_X]) - (v1[F_X] * v2[F_Z]);
	cross[F_Z] = (v1[F_X] * v2[F_Y]) - (v1[F_Y] * v2[F_X]);
}

void NormalizeAngle(float* angle) {
	if (*angle > 180.0f)
		*angle -= 360.0f;
	
	if (*angle < -180.0f)
		*angle += 360.0f;
}

void ClampAngle(float* angle) {
	if (*angle < 0.0f)
		*angle += 360.0f;
	
	*angle = fmodf(*angle, 360.0f);
}

float VectorPitch(float* vec) {
	if (vec[F_X] == 0.0f && vec[F_Y] == 0.0f)
		return 0.0f;
	
	float result = RAD2DEG(atan2f(vec[F_Z] * -1.0f, sqrtf((vec[F_X] * vec[F_X]) + (vec[F_Y] * vec[F_Y]))));
	if (result < 0.0f)
		result += 360.0f;
	
	return result;
}

float VectorYaw(float* vec) {
	if (vec[F_Y] == 0.0f && vec[F_X] == 0.0f)
		return 0.0f;
	
	float result = RAD2DEG(atan2f(vec[F_Y], vec[F_X]));
	if (result < 0.0f)
		result += 360.0f;
	
	return result;
}

void VectorAngles(float* vec, float* pitch, float* yaw) {
	*pitch = VectorPitch(vec);
	*yaw = VectorYaw(vec);
}

void AngleForward(float pitch, float yaw, float* vec) {
	float p = DEG2RAD(pitch);
	float y = DEG2RAD(yaw);
	
	float sp = 0.0f;
	float cp = 0.0f;
	sincosf(p, &sp, &cp);
	
	float sy = 0.0f;
	float cy = 0.0f;
	sincosf(y, &sy, &cy);
	
	vec[F_X] = cp * cy;
	vec[F_Y] = cp * sy;
	vec[F_Z] = -sp;
}

void AngleRight(float pitch, float yaw, float roll, float* vec) {
	float p = DEG2RAD(pitch);
	float y = DEG2RAD(yaw);
	float r = DEG2RAD(roll);
	
	float sp = 0.0f;
	float cp = 0.0f;
	sincosf(p, &sp, &cp);

	float sy = 0.0f;
	float cy = 0.0f;
	sincosf(y, &sy, &cy);
	
	float sr = 0.0f;
	float cr = 0.0f;
	sincosf(r, &sr, &cr);
	
	vec[F_X] = -1.0f * sr * sp * cy + -1.0f * cr * -sy;
	vec[F_Y] = -1.0f * sr * sp * sy + -1.0f * cr * cy;
	vec[F_Z] = -1.0f * sr * cr;
}

void AngleUp(float pitch, float yaw, float roll, float* vec) {
	float p = DEG2RAD(pitch);
	float y = DEG2RAD(yaw);
	float r = DEG2RAD(roll);
	
	float sp = 0.0f;
	float cp = 0.0f;
	sincosf(p, &sp, &cp);

	float sy = 0.0f;
	float cy = 0.0f;
	sincosf(y, &sy, &cy);
	
	float sr = 0.0f;
	float cr = 0.0f;
	sincosf(r, &sr, &cr);
	
	vec[F_X] = cr * sp * cy + -sr * -sy;
	vec[F_Y] = cr * sp * sy + -sr * cy;
	vec[F_Z] = cr * cp;
}

void AngleVectors(float pitch, float yaw, float roll, float* vec0, float* vec1, float* vec2) {
	float p = DEG2RAD(pitch);
	float y = DEG2RAD(yaw);
	float r = DEG2RAD(roll);
	
	float sp = 0.0f;
	float cp = 0.0f;
	sincosf(p, &sp, &cp);

	float sy = 0.0f;
	float cy = 0.0f;
	sincosf(y, &sy, &cy);
	
	float sr = 0.0f;
	float cr = 0.0f;
	sincosf(r, &sr, &cr);
	
	vec0[F_X] = cp * cy;
	vec0[F_Y] = cp * sy;
	vec0[F_Z] = -sp;
	
	vec1[F_X] = -1.0f * sr * sp * cy + -1.0f * cr * -sy;
	vec1[F_Y] = -1.0f * sr * sp * sy + -1.0f * cr * cy;
	vec1[F_Z] = -1.0f * sr * cr;
	
	vec2[F_X] = cr * sp * cy + -sr * -sy;
	vec2[F_Y] = cr * sp * sy + -sr * cy;
	vec2[F_Z] = cr * cp;
}

// source: some google code
bool IsWithinCylinder(Vector start, Vector end, float length, float radius, Vector test) {
	float length_sq = length * length;
	float radius_sq = radius * radius;
	
	float dx = end.x - start.x;
	float dy = end.y - start.y;
	float dz = end.z - start.z;
	
	float pdx = test.x - start.x;
	float pdy = test.y - start.y;
	float pdz = test.z - start.z;
	
	float dot = (pdx * dx) + (pdy * dy) + (pdz * dz);
	
	if (dot < 0.0f || dot > length_sq)
		return false;
	
	float dsq = ((pdx * pdx) + (pdy * pdy) + (pdz * pdz)) - ((dot * dot) / length_sq);
	
	if (dsq > radius_sq)
		return false;
	
	return true;
}

bool RayIntersectSphere(Vector org, Vector center, Vector dir, float radius_sqr, float* tmin) {
	Vector delta = org - center;
	
	float b = 2.0f * delta.dot(dir);
	float c = delta.dot(delta) - radius_sqr;
	
	float d = (b * b) - (4.0f * c);
	
	if (d < 1.e-6f)
		return false;
	
	float d2 = sqrtf(d);
	
	if (tmin)
		*tmin = (-b - d2) * 0.5f;
	
	return true;
}

bool RayIntersectCapsule(Vector org, Vector dir, Vector min, Vector max, float radius, Vector* point) {
	const float radius_sqr = radius * radius;
	
	Vector va = max - min;
	Vector vb = org - min;
	Vector vc = (min + (va)) - org;
	
	float va_dot_d = va.dot(dir);
	float va_dot_vb = va.dot(vb);
	float va_dot_va = va.dot(va);
	float vc_dot_d = vc.dot(dir);
	
	if (vc_dot_d < 0.0f)
		return false;
	
	float m = va_dot_d / va_dot_va;
	float n = va_dot_vb / va_dot_va;
	
	Vector q = dir - (va * m);
	Vector r = vb - (va * n);
	
	float a = q.dot(q);
	
	if (a == 0.0f)
		return false;
	
	float b = 2.0f * q.dot(r);
	float c = r.dot(r) - radius_sqr;
	
	float d = (b * b) - (4.0f * a * c);
	
	if (d < 0.0f)
		return false;
	
	float div = 2.0f * a;
	
	float d2 = sqrtf(d);
	
	float tmin = (-b - d2) / div;
	float tmax = (-b + d2) / div;
	if (tmin > tmax) {
		float temp = tmin;
		tmin = tmax;
		tmax = temp;
	}
	
	float t_k1 = tmin * m + n;
	
	if (t_k1 < 0.0f) {
		float len = 0.0f;
		
		if (!RayIntersectSphere(org, min, dir, radius_sqr, &len))
			return false;
		
		if (point)
			*point = org + (dir * len);
	} else if (t_k1 > 1.0f) {
		float len = 0.0f;
		
		if (!RayIntersectSphere(org, max, dir, radius_sqr, &len))
			return false;
		
		if (point)
			*point = org + (dir * len);
	} else {
		if (point)
			*point = org + (dir * tmin);
	}
	
	return true;
}

bool RayIntersectBox(Vector origin, Vector dir, Vector b_min, Vector b_max, float* length) {
	Vector dirfrac;
	dirfrac.x = 1 / dir.x;
	dirfrac.y = 1 / dir.y;
	dirfrac.z = 1 / dir.z;
	
	float t1 = (b_min.x - origin.x) * dirfrac.x;
	float t2 = (b_max.x - origin.x) * dirfrac.x;
	float t3 = (b_min.y - origin.y) * dirfrac.y;
	float t4 = (b_max.y - origin.y) * dirfrac.y;
	float t5 = (b_min.z - origin.z) * dirfrac.z;
	float t6 = (b_max.z - origin.z) * dirfrac.z;
	
	float tmin = fmaxf(fmaxf(fminf(t1, t2), fminf(t3, t4)), fminf(t5, t6));
	float tmax = fminf(fminf(fmaxf(t1, t2), fmaxf(t3, t4)), fmaxf(t5, t6));
	
	float t = 0.0f;
	
	if (tmin < 0.0f || tmax < 0.0f || tmin > tmax)
		return false;
	
	t = tmin;

	if (length)
		*length = t;
	
	return true;
}

void QuaternionNormalize(Quat* quat) {
	Quat q = *quat;
	
	float radius, iradius;
	
	radius = q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w;
	
	if (radius > 1e-6) {
		radius = sqrtf(radius);
		iradius = 1.0f / radius;
		q.w *= iradius;
		q.z *= iradius;
		q.y *= iradius;
		q.x *= iradius;
	}
	
	*quat = q;
}

/*void MatrixQuaternion(matrix3x4* matrix, Quat* quat) {
	Quat q;
	float trace = (*matrix)[0][0] + (*matrix)[1][1] + (*matrix)[2][2] + 1.0f;
	if (trace > 1.0f + (1e-5)) {
		q.x = ((*matrix)[2][1] - (*matrix)[1][2]);
		q.y = ((*matrix)[0][2] - (*matrix)[2][0]);
		q.z = ((*matrix)[1][0] - (*matrix)[0][1]);
		q.w = trace;
	}  else if ((*matrix)[0][0] > (*matrix)[1][1] && (*matrix)[0][0] > (*matrix)[2][2]) {
		trace = 1.0f + (*matrix)[0][0] - (*matrix)[1][1] - (*matrix)[2][2];
		q.x = trace;
		q.y = ((*matrix)[1][0] + (*matrix)[0][1] );
		q.z = ((*matrix)[0][2] + (*matrix)[2][0] );
		q.w = ((*matrix)[2][1] - (*matrix)[1][2] );
	} else if ((*matrix)[1][1] > (*matrix)[2][2]) {
		trace = 1.0f + (*matrix)[1][1] - (*matrix)[0][0] - (*matrix)[2][2];
		q.x = ((*matrix)[0][1] + (*matrix)[1][0] );
		q.y = trace;
		q.z = ((*matrix)[2][1] + (*matrix)[1][2] );
		q.w = ((*matrix)[0][2] - (*matrix)[2][0] );
	} else {
		trace = 1.0f + (*matrix)[2][2] - (*matrix)[0][0] - (*matrix)[1][1];
		q.x = ((*matrix)[0][2] + (*matrix)[2][0] );
		q.y = ((*matrix)[2][1] + (*matrix)[1][2] );
		q.z = trace;
		q.w = ((*matrix)[1][0] - (*matrix)[0][1] );
	}
	
	QuaternionNormalize(&q);
	
	//if (quat)
	//	memcpy(quat, &q, sizeof(Quat));
}

void QuaternionMatrix(Quat q, matrix3x4* matrix) {
	(*matrix)[0][0] = 1.0 - 2.0 * q.y * q.y - 2.0 * q.z * q.z;
	(*matrix)[1][0] = 2.0 * q.x * q.y + 2.0 * q.w * q.z;
	(*matrix)[2][0] = 2.0 * q.x * q.z - 2.0 * q.w * q.y;

	(*matrix)[0][1] = 2.0f * q.x * q.y - 2.0f * q.w * q.z;
	(*matrix)[1][1] = 1.0f - 2.0f * q.x * q.x - 2.0f * q.z * q.z;
	(*matrix)[2][1] = 2.0f * q.y * q.z + 2.0f * q.w * q.x;

	(*matrix)[0][2] = 2.0f * q.x * q.z + 2.0f * q.w * q.y;
	(*matrix)[1][2] = 2.0f * q.y * q.z - 2.0f * q.w * q.x;
	(*matrix)[2][2] = 1.0f - 2.0f * q.x * q.x - 2.0f * q.y * q.y;
}*/