#ifndef JADE_MATH_H
#define JADE_MATH_H

#include <algorithm>

#define M_PI       3.14159265358979323846f
#define INV_PI     0.31830988618379067154f
#define INV_TWOPI  0.15915494309189533577f
#define INV_FOURPI 0.07957747154594766788f

namespace jade
{
	template<typename T>
	inline T Min(T v1, T v2)
	{
		return std::min(v1, v2);
	}
	
	template<typename T>
	inline T Max(T v1, T v2)
	{
		return std::max(v1, v2);
	}
	
	template<typename T>
	inline T abs(T v)
	{
		return v < 0?-v:v;
	}
	
	template<typename T>
	inline T lerp(T v1, T v2, float t)
	{
		return (1.f - t) * v1 + t * v2;
	}
	
	template<typename T>
	inline T clamp(T v, T vmin, T vmax)
	{
		T temp = Max(v, vmin);
		return Min(temp, vmax);
	}
	
	inline int FloatToInt(float v)
	{
		return (int) v;
	}
}


#endif
