#ifndef BRDF_H
#define BRDF_H

#include "../vector.h"
#include "jade_math.h"
#include <math.h>

namespace jade
{
	inline Vector3 FSchlick(const Vector3& f0, float lDotH)
	{
		return f0 + (Vector3(1.0f) - f0) * powf(1.f - lDotH, 5.f);
	}

	inline float BlinnNDF(float nDotH, float roughness)
	{
		return ((roughness + 2.f) / (2 * M_PI) ) * powf(nDotH, roughness);
	}

	inline float GeomTerm(const Vector3& wi, const Vector3& wo, const Vector3& n)
	{
		return std::max(dot(n, wo), dot(n, wi));
	}

	inline Vector3 BlinnBRDF(const Vector3& wo, const Vector3& wi, const Vector3& n, const Vector3& l, const Vector3& f0, float roughness)
	{
		Vector3 h = Normalize(wo + wi);
		float nDotH = std::max(dot(n, h), 0.f );
		float lDotH = std::max( dot(l, h), 0.f );
		
		float hDotWi = dot(h, wi);

		Vector3 fresnel = FSchlick(f0, lDotH);
		float ndf = BlinnNDF(nDotH, roughness);
		float geom = GeomTerm(wi, wo, n);

		return ( fresnel * ndf ) / (4 * M_PI * hDotWi * geom + 0.0001);
	}

	inline Vector3 SampleBlinnNDF(const Vector3& wo, const Vector3& n, float u1, float u2, float& pdf);

}

#endif