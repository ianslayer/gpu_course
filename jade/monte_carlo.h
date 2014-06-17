#ifndef MONTE_CARLO_H
#define MONTE_CARLO_H

#include "../vector.h"
#include <vector>

namespace jade
{
	Vector3 UniformSampleHemisphere(float u1, float u2);
	float  UniformHemispherePdf();
	
	Vector3 UniformSampleSphere(float u1, float u2);
	float  UniformSpherePdf();
	
	Vector3 UniformSampleCone(float u1, float u2, float costhetamax);
	Vector3 UniformSampleCone(float u1, float u2, float costhetamax, const Vector3& x, const Vector3& y, const Vector3& z);
	float UniformConePdf(float costhetamax);
	
	Vector2 UniformSampleDisk(float u1, float u2);
	Vector2 ConcentricSampleDisk(float u1, float u2);
	
	inline Vector3 CosineSampleHemisphere(float u1, float u2) {
		Vector3 ret;
		Vector2 xy = ConcentricSampleDisk(u1, u2);
		ret.x = xy.x;
		ret.y = xy.y;
		ret.z = sqrtf(std::max(0.f, 1.f - ret.x*ret.x - ret.y*ret.y));
		return ret;
	}

	Vector2 UniformSampleTriangle(float u1, float u2);
	
	struct Distribution1D {
		// Distribution1D Public Methods
		Distribution1D(const float *f, int n) {
			count = n;
			func = new float[n];
			memcpy(func, f, n*sizeof(float));
			cdf = new float[n+1];
			// Compute integral of step function at $x_i$
			cdf[0] = 0.;
			for (int i = 1; i < count+1; ++i)
				cdf[i] = cdf[i-1] + func[i-1] / n;
			
			// Transform step function integral into CDF
			funcInt = cdf[count];
			if (funcInt == 0.f) {
				for (int i = 1; i < n+1; ++i)
					cdf[i] = float(i) / float(n);
			}
			else {
				for (int i = 1; i < n+1; ++i)
					cdf[i] /= funcInt;
			}
		}
		~Distribution1D() {
			delete[] func;
			delete[] cdf;
		}
		float SampleContinuous(float u, float *pdf, int *off = NULL) const ;
		int SampleDiscrete(float u, float *pdf) const ;
		
	private:
		friend struct Distribution2D;
		// Distribution1D Private Data
		float *func, *cdf;
		float funcInt;
		int count;
	};
	
	
	struct Distribution2D {
		// Distribution2D Public Methods
		Distribution2D(const float *data, int nu, int nv);
		~Distribution2D();
		void SampleContinuous(float u0, float u1, float uv[2],
							  float *pdf) const {
			float pdfs[2];
			int v;
			uv[1] = pMarginal->SampleContinuous(u1, &pdfs[1], &v);
			uv[0] = pConditionalV[v]->SampleContinuous(u0, &pdfs[0]);
			*pdf = pdfs[0] * pdfs[1];
		}
		float Pdf(float u, float v) const;
	private:
		// Distribution2D Private Data
		std::vector<Distribution1D *> pConditionalV;
		Distribution1D *pMarginal;
	};
}


#endif
