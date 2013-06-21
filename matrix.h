#ifndef __JOG_MATRIX_H
#define __JOG_MATRIX_H

#include "vector.h"

class Matrix3x3
{
public: 
	explicit Matrix3x3(){};
	explicit Matrix3x3(float m00, float m01, float m02,
		float m10, float m11, float m12,
		float m20, float m21, float m22);	
	explicit Matrix3x3(const float* floatPtr);

	Vector3& operator [] (int row);
	const Vector3& operator [] (int row) const;

	const Matrix3x3& operator*= (float a);
	const Matrix3x3& operator*= (const Matrix3x3& mat);

	Matrix3x3 operator * (float a);
	Vector3   operator * (const Vector3& vec);
	Matrix3x3 operator * (const Matrix3x3& mat);

	void MakeIdentity();
	Matrix3x3  Transpose();
	Matrix3x3& TransposeSelf();
	static Matrix3x3 RotateAxis(const Vector3& vec, float rad);

    float* FloatPtr();
    const float* FloatPtr() const;

private:
	Vector3 mat[3];
};

inline Matrix3x3::Matrix3x3(float m00, float m01, float m02,
                              float m10, float m11, float m12,
                              float m20, float m21, float m22)
{
	mat[0].x = m00; mat[0].y = m01; mat[0].z = m02;
	mat[1].x = m10; mat[1].y = m11; mat[1].z = m12; 
	mat[2].x = m20; mat[2].y = m21; mat[2].z = m22;
}

inline Matrix3x3::Matrix3x3(const float *floatPtr)
{
	float* mPtr = reinterpret_cast<float*> (this);
	memcpy(mPtr, floatPtr, sizeof(float) * 9);
}

inline Vector3& Matrix3x3::operator [] (int row)
{
	return mat[row];
}

inline const Vector3& Matrix3x3::operator [] (int row) const
{
	return mat[row];
}

inline const Matrix3x3& Matrix3x3::operator *= (float a) 
{
	mat[0].x *= a;	mat[0].y *= a; mat[0].z *= a;
	mat[1].x *= a;	mat[1].y *= a; mat[1].z *= a;
	mat[2].x *= a;	mat[2].y *= a; mat[2].z *= a;

	return *this;
}

inline const Matrix3x3& Matrix3x3::operator *= (const Matrix3x3& mat)
{
	int i, j;
	const float *m2Ptr;
	float *m1Ptr, dst[3];

	m1Ptr = reinterpret_cast<float *>(this);
	m2Ptr = reinterpret_cast<const float *>(&mat);

	for ( i = 0; i < 3; i++ ) {
		for ( j = 0; j < 3; j++ ) {
			dst[j]  = m1Ptr[0] * m2Ptr[ 0 * 3 + j ]
					+ m1Ptr[1] * m2Ptr[ 1 * 3 + j ]
					+ m1Ptr[2] * m2Ptr[ 2 * 3 + j ];
		}
		m1Ptr[0] = dst[0]; m1Ptr[1] = dst[1]; m1Ptr[2] = dst[2];
		m1Ptr += 3;
	}	

	return *this;
}

inline Matrix3x3 Matrix3x3::operator * (float a)
{
	return Matrix3x3(
			mat[0].x * a, mat[0].y * a, mat[0].z * a,
			mat[1].x * a, mat[1].y * a, mat[1].z * a,
			mat[2].x * a, mat[2].y * a, mat[2].z * a
			);
}

inline Vector3 Matrix3x3::operator * (const Vector3& _vec)
{
	return Vector3(
		mat[0].x * _vec.x + mat[0].y * _vec.y + mat[0].z * _vec.z,
		mat[1].x * _vec.x + mat[1].y * _vec.y + mat[1].z * _vec.z,
		mat[2].x * _vec.x + mat[2].y * _vec.y + mat[2].z * _vec.z
		);
}

inline Matrix3x3 Matrix3x3::operator *(const Matrix3x3 &mat)
{
	Matrix3x3 dst = *this;
	dst *= mat;
	return dst;
}

inline void Matrix3x3::MakeIdentity()
{
	mat[0].x = 1.f; mat[0].y = 0.f; mat[0].z = 0.f;
	mat[1].x = 0.f; mat[1].y = 1.f; mat[1].z = 0.f;
	mat[2].x = 0.f; mat[2].y = 0.f; mat[2].z = 1.f;
}

inline Matrix3x3 Matrix3x3::Transpose()
{
	return Matrix3x3(
		mat[0].x, mat[1].x, mat[2].x,
		mat[0].y, mat[1].y, mat[2].y,
		mat[0].z, mat[1].z, mat[2].z
	);
}

inline Matrix3x3& Matrix3x3::TransposeSelf()
{
	float temp0, temp1, temp2;
	temp0 = mat[0][1];
	mat[0][1] = mat[1][0];
	mat[1][0] = temp0;

	temp1 = mat[0][2];
	mat[0][2] = mat[2][0];
	mat[2][0] = temp1;

	temp2 = mat[1][2];
	mat[1][2] = mat[2][1];
	mat[2][1] = temp2;

	return *this;
}


inline Matrix3x3 Matrix3x3::RotateAxis(const Vector3& r, float rad)
{
	float cosPhi = cos(rad);
	float one_cosPhi = (1.f - cosPhi);
	float sinPhi = sin(rad);
	
	return Matrix3x3(
		cosPhi + one_cosPhi * r.x * r.x,       one_cosPhi * r.x * r.y - r.z * sinPhi, one_cosPhi  * r.x * r.z + r.y * sinPhi,
		one_cosPhi * r.x * r.y + r.z * sinPhi, cosPhi + one_cosPhi * r.y * r.y,       one_cosPhi * r.y * r.z - r.x * sinPhi,
		one_cosPhi * r.x * r.z - r.y * sinPhi, one_cosPhi * r.y * r.z + r.x * sinPhi, cosPhi + one_cosPhi * r.z * r.z
	);
}

inline float* Matrix3x3::FloatPtr()
{
    return reinterpret_cast<float*> (this);
}

inline const float* Matrix3x3::FloatPtr() const
{
    return reinterpret_cast<const float*> (this);
}

class Matrix4x4
{
public:
   Matrix4x4(){}
   explicit Matrix4x4(float m00, float m01, float m02, float m03,
                      float m10, float m11, float m12, float m13,
                      float m20, float m21, float m22, float m23,
                      float m30, float m31, float m32, float m33);

   explicit Matrix4x4(const float* floatPtr);

   void Set(float m00, float m01, float m02, float m03,
            float m10, float m11, float m12, float m13,
            float m20, float m21, float m22, float m23,
            float m30, float m31, float m32, float m33);

   Vector4& operator [] (int _row);
   const Vector4& operator [] (int _row) const;

   Matrix4x4 operator* (float a);
   Vector4 operator*(const Vector4& vec);
   Matrix4x4 operator* (const Matrix4x4& mat);

   const Matrix4x4& operator*=(float a);
   const Matrix4x4& operator*=(const Matrix4x4& mat);

   friend Matrix4x4 operator*(float a, const Matrix4x4& mat);
   friend Vector4 operator*(const Vector4& vec, const Matrix4x4& mat);

   const float* FloatPtr() const;
   float* FloatPtr();

   Matrix4x4  Transpose();
   Matrix4x4& TransposeSelf();

private:
   //store in row major...,
   Vector4 mat[4];
};

inline Matrix4x4::Matrix4x4(float m00, float m01, float m02, float m03, 
                              float m10, float m11, float m12, float m13, 
                              float m20, float m21, float m22, float m23, 
                              float m30, float m31, float m32, float m33)
{
	mat[0][0] = m00;
	mat[0][1] = m01;
	mat[0][2] = m02;
	mat[0][3] = m03;

	mat[1][0] = m10;
	mat[1][1] = m11;
	mat[1][2] = m12;
	mat[1][3] = m13;

	mat[2][0] = m20;
	mat[2][1] = m21;
	mat[2][2] = m22;
	mat[2][3] = m23;

	mat[3][0] = m30;
	mat[3][1] = m31;
	mat[3][2] = m32;
	mat[3][3] = m33;

}

inline Matrix4x4::Matrix4x4(const float* _mIn)
{
	
	memcpy(FloatPtr(), _mIn, sizeof(float) * 16 );
}

inline Vector4& Matrix4x4::operator[](int _row)
{	
	return mat[_row];
}
	
inline const  Vector4& Matrix4x4::operator[](int _row) const
{
	return mat[_row];
}

inline float* Matrix4x4::FloatPtr()
{
	return reinterpret_cast<float*> (this);
}

inline const float* Matrix4x4::FloatPtr() const
{
    return reinterpret_cast<const float*> (this);
}

inline void Matrix4x4::Set(float m00, float m01, float m02, float m03, 
							 float m10, float m11, float m12, float m13, 
							 float m20, float m21, float m22, float m23, 
							 float m30, float m31, float m32, float m33)
{
	mat[0][0] = m00; mat[0][1] = m01; mat[0][2] = m02; mat[0][3] = m03;
	mat[1][0] = m10; mat[1][1] = m11; mat[1][2] = m12; mat[1][3] = m13;
	mat[2][0] = m20; mat[2][1] = m21; mat[2][2] = m22; mat[2][3] = m23;
	mat[3][0] = m30; mat[3][1] = m31; mat[3][2] = m32; mat[3][3] = m33;
}

inline Matrix4x4 Matrix4x4::operator *(float a)
{
	return Matrix4x4(
		mat[0][0] * a, mat[0][1] * a, mat[0][2] * a, mat[0][3] * a,
		mat[1][0] * a, mat[1][1] * a, mat[1][2] * a, mat[1][3] * a,
		mat[2][0] * a, mat[2][1] * a, mat[2][2] * a, mat[2][3] * a,
		mat[3][0] * a, mat[3][1] * a, mat[3][2] * a, mat[3][3] * a
	);
}

inline Vector4 Matrix4x4::operator *(const Vector4 &vec)
{
	return Vector4(
		mat[0][0] * vec.x + mat[0][1] * vec.y + mat[0][2] * vec.z + mat[0][3] * vec.w,
		mat[1][0] * vec.x + mat[1][1] * vec.y + mat[1][2] * vec.z + mat[1][3] * vec.w,
		mat[2][0] * vec.x + mat[2][1] * vec.y + mat[2][2] * vec.z + mat[2][3] * vec.w,
		mat[3][0] * vec.x + mat[3][1] * vec.y + mat[3][2] * vec.z + mat[3][3] * vec.w
		);	
}

inline Matrix4x4 Matrix4x4::operator *(const Matrix4x4& mat)
{
	int i, j;
	const float* m1Ptr, *m2Ptr;
	float* dstPtr;
	Matrix4x4 dst;

	m1Ptr = reinterpret_cast<const float*> (this);
	m2Ptr = reinterpret_cast<const float*>(&mat);
	dstPtr = reinterpret_cast<float*>(&dst);

	for ( i = 0; i < 4; i++ ) {
		for ( j = 0; j < 4; j++ ) {
			*dstPtr = m1Ptr[0] * m2Ptr[ 0 * 4 + j ]
					+ m1Ptr[1] * m2Ptr[ 1 * 4 + j ]
					+ m1Ptr[2] * m2Ptr[ 2 * 4 + j ]
					+ m1Ptr[3] * m2Ptr[ 3 * 4 + j ];
			dstPtr++;
		}
		m1Ptr += 4;
	}
	return dst;
}

inline const Matrix4x4& Matrix4x4::operator *=(float a)
{
	mat[0][0] *= a; mat[0][1] *= a; mat[0][2] *= a; mat[0][3] *= a;
	mat[1][0] *= a; mat[1][1] *= a; mat[1][2] *= a; mat[1][3] *= a;
	mat[2][0] *= a; mat[2][1] *= a; mat[2][2] *= a; mat[2][3] *= a;
	mat[3][0] *= a; mat[3][1] *= a; mat[3][2] *= a; mat[3][3] *= a;

	return *this;
}

inline const Matrix4x4& Matrix4x4::operator *= (const Matrix4x4& mat)
{
	int i, j;
	float* m1Ptr;
	const float* m2Ptr;
	float dst[4];

	m1Ptr = reinterpret_cast<float*> (this);
	m2Ptr = reinterpret_cast<const float*>(&mat);

	for ( i = 0; i < 4; i++ ) {
		for ( j = 0; j < 4; j++ ) {
			dst[j] = m1Ptr[0] * m2Ptr[ 0 * 4 + j ]
					+ m1Ptr[1] * m2Ptr[ 1 * 4 + j ]
					+ m1Ptr[2] * m2Ptr[ 2 * 4 + j ]
					+ m1Ptr[3] * m2Ptr[ 3 * 4 + j ];
		}
		m1Ptr[0] = dst[0]; m1Ptr[1] = dst[1]; m1Ptr[2] = dst[2]; m1Ptr[3] = dst[3];
		m1Ptr += 4;
	}
	return *this;
}



//test __restrict
inline void Matrix4x4Mult(const float* __restrict m1Ptr, const float* __restrict m2Ptr, float* __restrict dstPtr)
{
	int i, j;
	for ( i = 0; i < 4; i++ ) {
		for ( j = 0; j < 4; j++ ) {
			*dstPtr = m1Ptr[0] * m2Ptr[ 0 * 4 + j ]
			+ m1Ptr[1] * m2Ptr[ 1 * 4 + j ]
			+ m1Ptr[2] * m2Ptr[ 2 * 4 + j ]
			+ m1Ptr[3] * m2Ptr[ 3 * 4 + j ];
			dstPtr++;
		}
		m1Ptr += 4;
	}
}

inline Matrix4x4 operator*(float a, const Matrix4x4& mat)
{
	return Matrix4x4(
		a * mat[0][0], a * mat[0][1], a * mat[0][2], a * mat[0][3],
		a * mat[1][0], a * mat[1][1], a * mat[1][2], a * mat[1][3],
		a * mat[2][0], a * mat[2][1], a * mat[2][2], a * mat[2][3],
		a * mat[3][0], a * mat[3][1], a * mat[2][2], a * mat[3][3]
	);
}

inline Matrix4x4 Matrix4x4::Transpose()
{
	return Matrix4x4(
			mat[0][0], mat[1][0], mat[2][0], mat[3][0],
			mat[0][1], mat[1][1], mat[2][1], mat[3][1],
			mat[0][2], mat[1][2], mat[2][2], mat[3][2],
			mat[0][3], mat[1][3], mat[2][3], mat[3][3]
	);
}

inline Matrix4x4& Matrix4x4::TransposeSelf()
{
	float temp0, temp1, temp2, temp3, temp4, temp5;
	temp0 = mat[0][1];
	mat[0][1] = mat[1][0];
	mat[1][0] = temp0;

	temp1 = mat[0][2];
	mat[0][2] = mat[2][0];
	mat[2][0] = temp1;

	temp2 = mat[0][3];
	mat[0][3] = mat[3][0];
	mat[3][0] = temp2;

	temp3 = mat[1][2];
	mat[1][2] = mat[2][1];
	mat[2][1] = temp3;

	temp4 = mat[1][3];
	mat[1][3] = mat[3][1];
	mat[3][1] = temp4;

	temp5 = mat[2][3];
	mat[2][3] = mat[3][2];
	mat[3][2] = temp5;

	return *this;

}

inline void MakeIdentity(Matrix4x4& mat)
{
	mat[0][0] = 1.0f;
	mat[0][1] = 0.0f;
	mat[0][2] = 0.0f;
	mat[0][3] = 0.0f;

	mat[1][0] = 0.0f;
	mat[1][1] = 1.0f;
	mat[1][2] = 0.0f;
	mat[1][3] = 0.0f;

	mat[2][0] = 0.0f;
	mat[2][1] = 0.0f;
	mat[2][2] = 1.0f;
	mat[2][3] = 0.0f;

	mat[3][0] = 0.0f;
	mat[3][1] = 0.0f;
	mat[3][2] = 0.0f;
	mat[3][3] = 1.0f;
}

inline Matrix4x4 Identity4x4()
{
    Matrix4x4 identity;
    MakeIdentity(identity);

    return identity;
}

inline Matrix4x4 Translate(const Vector3& _offset)
{
    return Matrix4x4(1, 0, 0, _offset.x,
                     0, 1, 0, _offset.y,
                     0, 0, 1, _offset.z,
                     0, 0, 0, 1);
}

inline Matrix4x4 Scale(const Vector3& _scale)
{
    return Matrix4x4(_scale.x, 0, 0, 0,
                     0, _scale.y, 0, 0,
                     0, 0, _scale.z, 0,
                     0, 0, 0, 1);
}


#endif