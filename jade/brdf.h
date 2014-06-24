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

	inline Vector3 BlinnBRDF(const Vector3& wo, const Vector3& wi, const Vector3& n, const Vector3& f0, float roughness)
	{
		Vector3 h = Normalize(wo + wi);
		const Vector3& l = wi;
		float nDotH = std::max(dot(n, h), 0.f );
		float lDotH = std::max( dot(l, h), 0.f );
		
		float hDotWi = dot(h, wi);

		Vector3 fresnel = FSchlick(f0, lDotH);
		float ndf = BlinnNDF(nDotH, roughness);
		float geom = GeomTerm(wi, wo, n);

		return ( fresnel * ndf ) / (4 * M_PI * hDotWi * geom + 0.0001);
	}

	inline Vector3 SphericalDirection(float sintheta,
		float costheta, float phi) {
			return Vector3(sintheta * cosf(phi),
				sintheta * sinf(phi),
				costheta);
	}


	inline Vector3 SphericalDirection(float sintheta, float costheta,
		float phi, const Vector3 &x,
		const Vector3& y, const Vector3& z) {
			return sintheta * cosf(phi) * x +
				sintheta * sinf(phi) * y + costheta * z;
	}

	inline bool SameHemisphere(const Vector3 &w, const Vector3 &wp) {
		return w.z * wp.z > 0.f;
	}

	//inline Vector3 CosineSampleBRDF(const Vector3& wo, const Vector3& n, );
	inline void SampleBlinnNDF(const Vector3& wo, Vector3& wi, float roughness, float u1, float u2, float& pdf)
	{
		float costheta = powf(u1, 1.f / (roughness + 1));
		float sintheta = sqrtf(std::max(0.f, 1.f - costheta*costheta));
		float phi = u2 * 2.f * M_PI;
		Vector3 wh = SphericalDirection(sintheta, costheta, phi);
		if (!SameHemisphere(wo, wh)) wh = -wh;
		wi = -wo + 2.f * dot(wo, wh) * wh;

		float blinn_pdf = ((roughness + 1.f) * powf(costheta, roughness)) /
			(2.f * M_PI * 4.f * dot(wo, wh));
		if (dot(wo, wh) <= 0.f) 
			blinn_pdf = 0.f;

		pdf = blinn_pdf;
	}

}

#endif