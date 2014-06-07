#include "monte_carlo.h"
#include "jade_math.h"
#include <algorithm>

namespace jade
{
	Vector3 UniformSampleHemisphere(float u1, float u2)
	{
		float z = u1;
		float r = sqrtf(std::max(0.f, 1.f - z*z));
		float phi = 2 * M_PI * u2;
		float x = r * cosf(phi);
		float y = r * sinf(phi);
		return Vector3(x, y, z);
	}
	
	
	float UniformHemispherePdf()
	{
		return INV_TWOPI;
	}
	
	Vector3 UniformSampleSphere(float u1, float u2)
	{
		float z = 1.f - 2.f * u1;
		float r = sqrtf(std::max(0.f, 1.f - z*z));
		float phi = 2.f * M_PI * u2;
		float x = r * cosf(phi);
		float y = r * sinf(phi);
		return Vector3(x, y, z);
	}
	
	float UniformSpherePdf()
	{
		return 1.f / (4.f * M_PI);
	}
	
	Vector3 UniformSampleCone(float u1, float u2, float costhetamax)
	{
		float costheta = (1.f - u1) + u1 * costhetamax;
		float sintheta = sqrtf(1.f - costheta*costheta);
		float phi = u2 * 2.f * M_PI;
		return Vector3(cosf(phi) * sintheta, sinf(phi) * sintheta, costheta);
	}
	
	Vector3 UniformSampleCone(float u1, float u2, float costhetamax, const Vector3 &x, const Vector3 &y, const Vector3 &z)
	{
		float costheta = lerp(costhetamax, 1.f, u1);
		float sintheta = sqrtf(1.f - costheta*costheta);
		float phi = u2 * 2.f * M_PI;
		return cosf(phi) * sintheta * x + sinf(phi) * sintheta * y +
        costheta * z;
	}
	
	float UniformConePdf(float costhetamax)
	{
		return 1.f / (2.f * M_PI * (1.f - costhetamax));
	}
	
	Vector2 UniformSampleDisk(float u1, float u2)
	{
		float r = sqrtf(u1);
		float theta = 2.0f * M_PI * u2;
		return Vector2(r * cosf(theta), r * sinf(theta));
	}

	
	Vector2 ConcentricSampleDisk(float u1, float u2) {
		float r, theta;
		// Map uniform random numbers to $[-1,1]^2$
		float sx = 2 * u1 - 1;
		float sy = 2 * u2 - 1;
		
		// Map square to $(r,\theta)$
		
		// Handle degeneracy at the origin
		if (sx == 0.0 && sy == 0.0) {
			return Vector2(0, 0);
		}
		if (sx >= -sy) {
			if (sx > sy) {
				// Handle first region of disk
				r = sx;
				if (sy > 0.0) theta = sy/r;
				else          theta = 8.0f + sy/r;
			}
			else {
				// Handle second region of disk
				r = sy;
				theta = 2.0f - sx/r;
			}
		}
		else {
			if (sx <= sy) {
				// Handle third region of disk
				r = -sx;
				theta = 4.0f - sy/r;
			}
			else {
				// Handle fourth region of disk
				r = -sy;
				theta = 6.0f + sx/r;
			}
		}
		theta *= M_PI / 4.f;
		return Vector2(r * cosf(theta), r * sinf(theta));
	}
	
	Vector2 UniformSampleTriangle(float u1, float u2)
	{
		float su1 = sqrtf(u1);
		return Vector2(1.f - su1, u2 * su1);
	}
	
	float Distribution1D::SampleContinuous(float u, float *pdf, int *off) const
	{
		// Find surrounding CDF segments and _offset_
		float *ptr = std::upper_bound(cdf, cdf+count+1, u);
		int offset = Max(0, int(ptr-cdf-1));
		if (off) *off = offset;
		//Assert(offset < count);
		//Assert(u >= cdf[offset] && u < cdf[offset+1]);
		
		// Compute offset along CDF segment
		float du = (u - cdf[offset]) / (cdf[offset+1] - cdf[offset]);
		//Assert(!isnan(du));
		
		// Compute PDF for sampled offset
		if (pdf) *pdf = func[offset] / funcInt;
		
		// Return $x\in{}[0,1)$ corresponding to sample
		return (offset + du) / count;
	}
	int Distribution1D::SampleDiscrete(float u, float *pdf) const
	{
		// Find surrounding CDF segments and _offset_
		float *ptr = std::upper_bound(cdf, cdf+count+1, u);
		int offset = Max(0, int(ptr-cdf-1));
		//Assert(offset < count);
		//Assert(u >= cdf[offset] && u < cdf[offset+1]);
		if (pdf) *pdf = func[offset] / (funcInt * count);
		return offset;
	}
	
	float Distribution2D::Pdf(float u, float v) const
	{
		int iu = clamp(FloatToInt(u * pConditionalV[0]->count), 0,
					   pConditionalV[0]->count-1);
		int iv = clamp(FloatToInt(v * pMarginal->count), 0,
					   pMarginal->count-1);
		if (pConditionalV[iv]->funcInt * pMarginal->funcInt == 0.f) return 0.f;
		return (pConditionalV[iv]->func[iu] * pMarginal->func[iv]) /
		(pConditionalV[iv]->funcInt * pMarginal->funcInt);
	}
	
}