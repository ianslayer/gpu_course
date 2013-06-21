#ifndef __JOG_VECTOR_INCLUDED__
#define __JOG_VECTOR_INCLUDED__
#include <algorithm>

class Vector2
{
public:
	Vector2()
	{}

	explicit Vector2(float _x, float _y)
	{
		x = _x;
		y = _y;
	}

	float operator []  (int i) const;	
	float& operator [](int i);

	template <class T>
	Vector2& operator=(const T& e);
	
	Vector2& operator+=(const Vector2& rhs);	
	Vector2& operator+=(float s);	
	Vector2& operator-=(const Vector2& rhs);	
	Vector2& operator-=(const float s);
	Vector2& operator*=(const Vector2& rhs);
	Vector2& operator*=(const float s);
	Vector2& operator/=(const Vector2& rhs);
	Vector2& operator/=(const float s);

	bool operator == (const Vector2& rhs);
	
	template<class T>
	void	Evaluate(const T& e);

	float	SquaredLength();
	float	Length();
	void	Normalize();	

	float	x;
	float	y;
};

inline float Vector2::operator []  (int i)const 
{
    return (&x)[i];
}	

inline float& Vector2::operator [](int i) 
{
    return (&x)[i];
}

template<class T>
inline Vector2& Vector2::operator = (const T& e)
{
	Evalute(e);
	return *this;
}

inline Vector2& Vector2::operator+=(const Vector2& rhs)
{
	x += rhs.x;
	y += rhs.y;
	return *this;
}

inline Vector2& Vector2::operator+=(float s)
{
	x += s;
	y += s;
	return *this;
}

inline Vector2& Vector2::operator-=(const Vector2& rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	return *this;
}

inline Vector2& Vector2::operator-=(float s)
{
	x -= s;
	y -= s;
	return *this;
}

inline Vector2& Vector2::operator*=(const Vector2& rhs)
{
	x *= rhs.x;
	y *= rhs.y;
	return *this;
}

inline Vector2& Vector2::operator*=(float s)
{
	x *= s;
	y *= s;
	return *this;
}

inline Vector2& Vector2::operator/=(const Vector2& rhs)
{
	x /= rhs.x;
	y /= rhs.y;
	return *this;
}

inline Vector2& Vector2::operator/=(float s)
{
	x /= s;
	y /= s;
	return *this;
}

inline Vector2 operator-(const Vector2& v)
{
    return Vector2(-v.x, -v.y);
}

template<class T> 
inline void Vector2::Evaluate(const T& e){
		x = e[0];
		y = e[1];		
}

inline Vector2 Max(const Vector2& a, const Vector2& b)
{
	return Vector2(std::max(a.x, b.x), std::max(a.y, b.y) );
}

inline Vector2 Min(const Vector2& a, const Vector2& b)
{
	return Vector2(std::min(a.x, b.x), std::min(a.y, b.y));
}

inline const Vector2 operator + (const Vector2& a, const Vector2& b)
{
	return Vector2(a.x + b.x, a.y + b.y);	
}

inline const Vector2 operator + (float a, const Vector2& b)
{
	return Vector2(a + b.x, a + b.y);	
}	

inline const Vector2 operator + (Vector2& a, float b)
{
	return Vector2(a.x + b, a.y + b);	
}	

inline const Vector2 operator - (const Vector2& a, const Vector2& b)
{
	return Vector2(a.x - b.x, a.y - b.y);	
}

inline const Vector2 operator*(const Vector2& a, const Vector2& b)
{
	return Vector2(a.x * b.x, a.y * b.y);
}

inline const Vector2 operator*(float a, const Vector2& b)
{
	return Vector2(a * b.x, a * b.y);
}

inline const Vector2 operator*(const Vector2& a, float b)
{
	return Vector2(a.x * b, a.y * b);
}		

inline const Vector2 operator/(const Vector2& a, const Vector2& b)
{
	//assert(b.x != 0 && b.y != 0 && b.z != 0);
	return Vector2(a.x / b.x, a.y / b.y);
}

inline const Vector2 operator / (const Vector2& v, float s)
{
	//assert(s != 0);
	return Vector2(v.x / s, v.y / s);
}

class Vector3 {
public:
	Vector3();
	explicit Vector3(float* _pVec);
	explicit Vector3(float _xyz);
	explicit Vector3(float _x, float _y, float _z);
	
	template<class T> 
	explicit Vector3(const T& e);

	void Set(float x, float y, float z);
	void Zero(void);

	float operator []  (int i) const;	
	float& operator [](int i);

	template <class T>
	Vector3& operator=(const T& e);

	Vector3& operator+=(const Vector3& rhs);	
	Vector3& operator+=(float s);	
	Vector3& operator-=(const Vector3& rhs);	
	Vector3& operator-=(const float s);
	Vector3& operator*=(const Vector3& rhs);
	Vector3& operator*=(const float s);
	Vector3& operator/=(const Vector3& rhs);
	Vector3& operator/=(const float s);

	bool operator == (const Vector3& rhs);
	
	float SquaredLength() const;
	float Length() const;
	void Normalize();	

	template<class T>
	void Evaluate(const T& e);

	float x;
	float y;
	float z;
};


inline Vector3::Vector3()
{
	//x = y = z = 0.f;
	//mVec[0] = mVec[1] = mVec[2] = 0.f;
}

inline Vector3::Vector3(float* _pVec)
{
	x = _pVec[0];
	y = _pVec[1];
	z = _pVec[2];
}

inline Vector3::Vector3(float _x, float _y, float _z)
{
	x = _x;
	y = _y;
	z = _z;
}

template<class T>
inline  Vector3:: Vector3(const T& e)
{
	Evaluate(e);
}	

inline void Vector3::Set(float _x, float _y, float _z)
{
	x = _x;
	y = _y;
	z = _z;
}

inline void Vector3::Zero(void)
{
	x = y = z = 0;
}

inline float Vector3::operator []  (int i)const 
{
	return (&x)[i];
}	

inline float& Vector3::operator [](int i) 
{
	return (&x)[i];
}

template <class T> 
inline Vector3& Vector3::operator=(const T& e)
{
	Evaluate(e); return *this;
}	

inline Vector3 operator-(const Vector3& v)
{
	return Vector3(-v.x, -v.y, -v.z);
}

inline Vector3& Vector3::operator+=(const Vector3& rhs)
{
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	return *this;
}

inline Vector3& Vector3::operator+=(float s)
{
	x += s;
	y += s;
	z += s;		
	return *this;
}

inline Vector3& Vector3::operator-=(const Vector3& rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	return *this;
}

inline Vector3& Vector3::operator-=(const float s)
{
	x -= s;
	y -= s;
	z -= s;		
	return *this;
}

inline Vector3& Vector3::operator*=(const Vector3& rhs)
{
	x *= rhs.x;
	y *= rhs.y;
	z *= rhs.z;
	return *this;
}

inline Vector3& Vector3::operator*=(const float s)
{
	x *= s;
	y *= s;
	z *= s;		
	return *this;
}	

inline Vector3& Vector3::operator/=(const Vector3& rhs)
{
	//assert(rhs.x != 0 && rhs.y != 0 && rhs.z != 0);
	x /= rhs.x;
	y /= rhs.y;
	z /= rhs.z;
	return *this;
}

inline Vector3& Vector3::operator/=(const float s)
{
	//assert(s != 0);
	float inv_s = 1.f / s;
	x *= inv_s;
	y *= inv_s;
	z *= inv_s;		
	return *this;
}	

inline bool Vector3::operator == (const Vector3& rhs)
{
	return ( (x == rhs.x) && (y == rhs.y) && (z == rhs.z));
}

inline float Vector3::SquaredLength() const
{
	return x * x + y * y + z * z;
}

inline float Vector3::Length() const
{
	return sqrt(x * x + y * y + z * z);
}

inline void Vector3::Normalize()
{
	float length = Length();
	x /= length;
	y /= length;
	z /= length;
}

template<class T> inline
	void Vector3::Evaluate(const T& e){
		x = e[0];
		y = e[1];
		z = e[2];		
}

inline const Vector3 operator + (const Vector3& a, const Vector3& b)
{
	return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);	
}

inline const Vector3 operator + (float a, const Vector3& b)
{
	return Vector3(a + b.x, a + b.y, a + b.z);	
}	

inline const Vector3 operator + (Vector3& a, float b)
{
	return Vector3(a.x + b, a.y + b, a.z + b);	
}	

inline const Vector3 operator - (const Vector3& a, const Vector3& b)
{
	return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);	
}

inline const Vector3 operator*(const Vector3& a, const Vector3& b)
{
	return Vector3(a.x * b.x, a.y * b.y, a.z * b.z);
}

inline const Vector3 operator*(float a, const Vector3& b)
{
	return Vector3(a * b.x, a * b.y, a * b.z);
}

inline const Vector3 operator*(const Vector3& a, float b)
{
	return Vector3(a.x * b, a.y * b, a.z * b);
}		

inline const Vector3 operator/(const Vector3& a, const Vector3& b)
{
	//assert(b.x != 0 && b.y != 0 && b.z != 0);
	return Vector3(a.x / b.x, a.y / b.y, a.z / b.z);
}

inline const Vector3 operator / (const Vector3& v, float s)
{
	//assert(s != 0);
	return Vector3(v.x / s, v.y / s, v.z / s);
}

inline Vector3 cross(const Vector3& a, const Vector3& b)
{
	return Vector3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

inline float dot(const Vector3& a, const Vector3& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vector3 Max(const Vector3& a, const Vector3& b)
{
	return Vector3(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z) );
}

inline Vector3 Min(const Vector3& a, const Vector3& b)
{
	return Vector3(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
}

inline Vector3 Normalize(const Vector3& a)
{
	Vector3 n(a);
	n.Normalize();
	return n;
}

class Vector4 {
public:
	Vector4();
	explicit Vector4(float* _pVec);
	explicit Vector4(float _xyzw);
	explicit Vector4(float _x, float _y, float _z, float _w);
template<class T> 
	explicit Vector4(const T& e);

	float operator []  (int index) const;	
	float& operator [](int index);

template <class T>
	const Vector4& operator=(const T& e);
	const Vector4& operator+=(const Vector4& rhs);	
	const Vector4& operator+=(float s);	
	const Vector4& operator-=(const Vector4& rhs);	
	const Vector4& operator-=(const float s);
	const Vector4& operator*=(const Vector4& rhs);
	const Vector4& operator*=(const float s);
	const Vector4& operator/=(const Vector4& rhs);
	const Vector4& operator/=(const float s);
	bool operator == (const Vector4& rhs);

	bool  Compare( const Vector4 &a ) const;							// exact compare, no epsilon
	bool  Compare( const Vector4 &a, const float epsilon ) const;		// compare with epsilon

	float SquaredLength() const;
    float Length() const;
	void  Normalize();	

	template<class T>
	void  Evaluate(const T& e);

	float x;
	float y;
	float z;
	float w;
};

inline Vector4::Vector4()
{
	//x = y = z = w = 0.f;
}

inline Vector4::Vector4(float* _pVec)
{
	x = _pVec[0];
	y = _pVec[1];
	z = _pVec[2];
	w = _pVec[3];
}

inline Vector4::Vector4(float _xyzw)
{
	x = y = z = w = _xyzw;
}

inline Vector4::Vector4(float _x, float _y, float _z, float _w)
{
	x = _x;
	y = _y;
	z = _z;
	w = _w;
}

template<class T> inline
Vector4::Vector4(const T &e)
{
	Evaluate(e);
}

template<class T> inline
void Vector4::Evaluate(const T&e)
{
	x = e[0];
	y = e[1];
	z = e[2];
	w = e[3];
}

inline float Vector4::operator [](int index) const
{
	return (&x)[index];
}

inline float& Vector4::operator [](int index)
{
	return (&x)[index];
}

template<class T> inline const Vector4& Vector4::operator =(const T &e)
{
		Evaluate(e); return *this;
}

inline Vector4 operator-(const Vector4& v)
{
    return Vector4(-v.x, -v.y, -v.z, -v.w);
}

inline const Vector4& Vector4::operator+=(const Vector4& rhs)
{
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	w += rhs.w;
	return *this;
}

inline const Vector4& Vector4::operator+=(float c)
{
	x += c;
	y += c;
	z += c;
	w += c;
	return *this;
}

inline const Vector4& Vector4::operator-=(const Vector4& rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	w -= rhs.w;
	return *this;
}

inline const Vector4& Vector4::operator -=(float c)
{
	x -= c;
	y -= c;
	z -= c;
	w -= c;
	return *this;
}

inline const Vector4& Vector4::operator*=(const Vector4& rhs)
{
	x *= rhs.x;
	y *= rhs.y;
	z *= rhs.z;
	w *= rhs.w;
	return *this;
}

inline const Vector4& Vector4::operator *=(float c)
{
	x *= c;
	y *= c;
	z *= c;
	w *= c;
	return *this;
}

inline const Vector4& Vector4::operator /=(const Vector4& rhs)
{
	//assert(rhs.x != 0 && rhs.y != 0 && rhs.z != 0 && rhs.w != 0);
	x /= rhs.x;
	y /= rhs.y;
	z /= rhs.z;
	w /= rhs.w;
	return *this;
}

inline const Vector4& Vector4::operator /= (float c)
{
	//assert(c!= 0);
	x /= c;
	y /= c;
	z /= c;
	w /= c;
	return *this;
}

inline float Vector4::SquaredLength() const
{
    return x * x + y * y + z * z + w * w;
}

inline float Vector4::Length() const
{
    return sqrt(SquaredLength());
}

inline void Vector4::Normalize()
{
	float length = Length();
	x /= length;
	y /= length;
	z /= length;
	w /= length;
}

inline const Vector4 operator + (const Vector4& a, const Vector4& b)
{
	return Vector4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);	
}

inline const Vector4 operator + (float a, const Vector4& b)
{
	return Vector4(a + b.x, a + b.y, a + b.z, a + b.w);	
}	

inline const Vector4 operator + (Vector4& a, float b)
{
	return Vector4(a.x + b, a.y + b, a.z + b, a.w + b);	
}	

inline const Vector4 operator - (const Vector4& a, const Vector4& b)
{
	return Vector4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);	
}

inline const Vector4 operator*(const Vector4& a, const Vector4& b)
{
	return Vector4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

inline const Vector4 operator*(float a, const Vector4& b)
{
	return Vector4(a * b.x, a * b.y, a * b.z, a * b.w);
}

inline const Vector4 operator*(const Vector4& a, float b)
{
	return Vector4(a.x * b, a.y * b, a.z * b, a.w * b);
}		

inline const Vector4 operator/(const Vector4& a, const Vector4& b)
{
	//assert(b.x != 0 && b.y != 0 && b.z != 0);
	return Vector4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}

inline const Vector4 operator / (const Vector4& v, float s)
{
	//assert(s != 0);
	return Vector4(v.x / s, v.y / s, v.z / s, v.w / s);
}

inline Vector4 Max(const Vector4& a, const Vector4& b)
{
	return Vector4(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z), std::max(a.w, b.w) );
}

inline Vector4 Min(const Vector4& a, const Vector4& b)
{
	return Vector4(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z), std::min(a.w, b.w));
}

#endif