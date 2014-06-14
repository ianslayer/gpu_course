#ifndef COLOR_H
#define COLOR_H

#include "../vector.h"

namespace jade
{
	typedef Vector4 RGBA32F;
	typedef Vector3 RGB32F;
	
	struct RGBA8
	{
		unsigned char c[4];
	};
	
	class RGB8
	{
		unsigned char c[3];
	};
	
}

#endif
