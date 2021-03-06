#ifndef GEOMETRY_H
#define GEOMETRY_H
#include "../vector.h"
#include "../matrix.h"
#include "jade_math.h"
#include <algorithm>
#include <float.h>

namespace jade
{
struct AABB
{
	AABB() : center(0, 0, 0), radius(-FLT_MAX, -FLT_MAX, -FLT_MAX) {};
	AABB( const Vector3& _center, const Vector3& _radius ) : center(_center), radius(_radius) {}
    Vector3 center;
    Vector3 radius; //half width
};

inline Vector3 Min(const AABB& bound)
{
	return bound.center - bound.radius;
}
	
inline Vector3 Max(const AABB& bound)
{
	return bound.center + bound.radius;
}
	
struct Sphere
{
	Sphere() : center(0, 0, 0), radius(-FLT_MAX) {};
	Sphere( const Vector3& _center, const float _radius ) : center(_center), radius(_radius) {}
    Vector3 center;
    float   radius;
};

inline Sphere BoundSphere(const AABB& bound)
{
	return Sphere(bound.center, bound.radius.Length());
}

struct Ray
{
	Ray() : origin(0, 0, 0), direction(0, 0, 1) {}
	Ray( const Vector3& _origin, const Vector3& _direction) : origin(_origin), direction(_direction) {}

    Vector3 origin;
    Vector3 direction;
};

inline Ray Transform(const Matrix4x4& m, const Ray& ray)
{
	Ray r;
	r.origin = TransformPoint(m, ray.origin);
	r.direction = Normalize(TransformVector(m, ray.direction));
	return r;
}

inline Vector3 GetPoint(const Ray& r, float t)
{
	return r.origin + t * r.direction;
}

inline Vector3 TransformToTangentSpace(const Vector3& normal, const Vector4& tangent, const Vector3& worldVec)
{
	Vector3 t = DiscardW(tangent);
	Vector3 bitangent = cross(normal, t) * tangent[3];
	return Vector3(dot(t, worldVec), dot(bitangent, worldVec), dot(normal, worldVec));
}

inline Vector3 TransformToWorldSpace(const Vector3& normal, const Vector4& tangent, const Vector3& tangentVec)
{
	Vector3 t = DiscardW(tangent);
	Vector3 bitangent = cross(normal, t) * tangent[3];
	return Vector3(t * tangentVec[0] + bitangent * tangentVec[1] + normal * tangentVec[2]);
}

struct Range
{
	Range() : tmin(0.f), tmax(FLT_MAX){}
	float tmin;
	float tmax;
};
	
struct Plane
{
    Vector3 normal; //plane normal. Points x on the plane satisfy Dot(n, x) = d
    float d; // d = dot(n,p) for a given point p on the plane
};

inline AABB Merge(const AABB& b1, const AABB& b2)
{
    Vector3 maxBound;
    Vector3 minBound;

    Vector3 maxB1 = b1.center + b1.radius;
    Vector3 minB1 = b1.center - b1.radius;
    Vector3 maxB2 = b2.center + b2.radius;
    Vector3 minB2 = b2.center - b2.radius;

    maxBound.x = std::max(maxB1.x, maxB2.x);
    maxBound.y = std::max(maxB1.y, maxB2.y);
    maxBound.z = std::max(maxB1.z, maxB2.z);

    minBound.x = std::min(minB1.x, minB2.x);
    minBound.y = std::min(minB1.y, minB2.y);
    minBound.z = std::min(minB1.z, minB2.z);

    AABB bound;
    bound.radius = (maxBound - minBound) / 2.0f;
    bound.center = (maxBound + minBound) / 2.0f;

    return bound;    
}

inline void GetVertices(const AABB& bound, Vector3 vert[8] )
{
    Vector3 offset[8];
    offset[0] = Vector3(bound.radius.x, bound.radius.y, bound.radius.z);
    offset[1] = Vector3(bound.radius.x, bound.radius.y, -bound.radius.z);
    offset[2] = Vector3(bound.radius.x, -bound.radius.y, bound.radius.z);
    offset[3] = Vector3(bound.radius.x, -bound.radius.y, -bound.radius.z);
    offset[4] = Vector3(-bound.radius.x, bound.radius.y, bound.radius.z);
    offset[5] = Vector3(-bound.radius.x, bound.radius.y, -bound.radius.z);
    offset[6] = Vector3(-bound.radius.x, -bound.radius.y, bound.radius.z);
    offset[7] = Vector3(-bound.radius.x, -bound.radius.y, -bound.radius.z);

    for(int i = 0; i < 8; i++)
    {
        vert[i] = bound.center + offset[i];
    }
}

inline AABB Transform(Matrix4x4& m, const AABB& bound)
{
	Vector3 vert[8];
	GetVertices(bound, vert);
	for(int i = 0; i < 8 ; i++)
	{
		vert[i] = DiscardW(m * Vector4(vert[i], 1.f));
	}

	float xmax, xmin, ymax, ymin, zmax, zmin;
	xmax = xmin = vert[0].x;
	ymax = ymin = vert[0].y;
	zmax = zmin = vert[0].z;

	for(int i = 1; i < 8; i++)
	{
		xmax = std::max(xmax, vert[i].x);
		xmin = std::min(xmin, vert[i].x);

		ymax = std::max(ymax, vert[i].y);
		ymin = std::min(ymin, vert[i].y);

		zmax = std::max(zmax, vert[i].z);
		zmin = std::min(zmin, vert[i].z);
	}

	Vector3 _max = Vector3(xmax, ymax, zmax);
	Vector3 _min = Vector3(xmin, ymin, zmin);
	AABB transformedBound;
	transformedBound.center = (_max + _min) / 2.f;
	transformedBound.radius = (_max - _min) / 2.f;
	

	return transformedBound;
}

inline Plane ComputePlane(Vector3 a, Vector3 b, Vector3 c)
{
    Plane p;
    p.normal = Normalize(cross(b - a, c - a));
    p.d = dot(p.normal, a);
    return p;
}

inline Vector3 ClosestPtPointPlane(Vector3 q, Plane p)
{
    float t = (dot(p.normal, q) - p.d) / dot(p.normal, p.normal);
    return q - t * p.normal;
}

inline float DistPointPlane(Vector3 q, Plane p)
{
    return (dot(p.normal, q) - p.d) / dot(p.normal, p.normal);
}


//Given segment ab and point c, computes closest point d on ab.
//Also return s t for the position of d, d(t) = a + t * (b-a)  
inline void ClosestPtPointSegment(Vector3 c, Vector3 a, Vector3 b, float* t, Vector3* d)
{
    Vector3 ab = b-a;
    //project c onto ab, computing parameterized position d(t) = a + t * (b - a)
    *t = dot(c - a, ab) / dot(ab, ab);
    // if outside  segment, clamp t (and therefore d) to the closest endpoint
    if(*t < 0.0f) *t = 0.0f;
    if(*t > 1.0f) *t = 1.0f;
    //compute projected position from the clamped t
    *d = a + *t * ab;

}

//compute barycentric coordinates (u, v, w) for point p with respect to triangle (a, b, c)
inline void Barycentric(Vector3 a, Vector3 b, Vector3 c, Vector3 p, float* u, float* v, float* w)
{
    Vector3 v0 = b - a, v1 = c - a, v2 = p - a;
    float d00 = dot(v0, v0);
    float d01 = dot(v0, v1);
    float d11 = dot(v1, v1);
    float d20 = dot(v2, v0);
    float d21 = dot(v2, v1);
    float denom = d00 * d11 - d01 * d01;
    *v = (d11 * d20 - d01 * d21) / denom;
    *w = (d00 * d21 - d01 * d20) / denom;
    *u = 1.f - *v - *w;
}

inline float TriangleArea(const Vector3& v0, const Vector3& v1, const Vector3 &v2)
{
    Vector3 v = v1 - v0;
    Vector3 w = v2 - v0;

    return 0.5f * ::sqrt(cross(v, w).SquaredLength() ); //don't use jog::sqrt because of the degenerate triangle
}

inline AABB ComputeBound(const Vector3* vertices, int numVert)
{
	Vector3 minBound = Vector3(FLT_MAX);
	Vector3 maxBound = Vector3(-FLT_MAX);
	Vector3 temp;
	AABB bound;

	for(int i = 0; i < numVert; i++)
	{
		temp = Min(minBound, vertices[i]);
		minBound = temp;
		temp = Max(maxBound, vertices[i]);
		maxBound = temp;
	}

	bound.center = (minBound + maxBound) / 2.f;
	bound.radius = (maxBound - minBound) / 2.f;

	return bound;
}

inline int IntersectRayAABB(const Ray& ray, const AABB& bound, Range& range, float epsilon)
{
	float tmin = 0.f;
	float tmax = FLT_MAX;
	
	Vector3 minBound = Min(bound);
	Vector3 maxBound = Max(bound);
	for(int i = 0; i < 3; i++)
	{
		if(abs(ray.direction[i]) < epsilon)
		{
			if(ray.origin[i] < minBound[i] || ray.origin[i] > maxBound[i])
				return 0;
		}
		else
		{
			float ood = 1.0f / ray.direction[i];
			float t1 = (minBound[i] - ray.origin[i]) * ood;
			float t2 = (maxBound[i] - ray.origin[i]) * ood;
			
			if(t1 > t2)
				std::swap(t1, t2);
			
			tmin = Max(tmin, t1);
			tmax = Min(tmax, t2);
			
			if(tmin > tmax) return 0;
		}
	}
	
	range.tmin = tmin;
	range.tmax = tmax;
	
	return 1;
}

struct Vertex
{
	Vector3 position;
};
	
struct VertexP3C3
{
	Vector3 position;
	Vector3 color;
};

struct VertexP3T2
{
	Vector3 position;
	Vector2 texcoord;
};
	
struct VertexP3N3T2
{
    Vector3 position;
    Vector3 normal;
    Vector2 texcoord;
};
    
struct VertexP3N3T4T2
{
    Vector3 position;
    Vector3 normal;
    Vector4 tangent;
    Vector2 texcoord;
};
	
	/*
template <typename VertexType>
inline int IntersectSegmentTriangle(const Ray& ray, const Range& range, const VertexType* vertices, const int* indices, float& u, float& v, float& w, float& t)
{
	const Vector3& a = vertices[indices[0]].position;
	const Vector3& b = vertices[indices[1]].position;
	const Vector3& c = vertices[indices[2]].position;
	
	const Vector3 p = GetPoint(ray, range.tmin);
	const Vector3 q = GetPoint(ray, range.tmax);
	
	Vector3 ab = b - a;
	Vector3 ac = c - a;
	Vector3 qp = p - q;
	
	Vector3 n = cross(ab, ac);
	
	float d = dot(qp, n);
	
	if(d <= 0.f) return 0;
	
	Vector3 ap = p - a;
	t = dot(ap, n);
	if(t < 0.f) return 0;
	if(t > d) return 0;
	
	Vector3 e = cross(qp, ap);
	v = dot(ac, e);
	if(v < 0.f || v > d) return 0;
	w = -dot(ab, e);
	if(w < 0.f || v + w > d) return 0;
	
	float ood = 1.f / d;
	t*=ood;
	v*=ood;
	w*=ood;
	u = 1.f - v- w;
	
	t = lerp(range.tmin, range.tmax, t);

	return 1;
}
	*/
	
template <typename VertexType>
inline int IntersectSegmentTriangle(const Ray& ray, const Range& range, const VertexType* vertices, const int* indices, float& u, float& v, float& w, float& t)
{
	const Vector3& p1 = vertices[indices[0]].position;
	const Vector3& p2 = vertices[indices[1]].position;
	const Vector3& p3 = vertices[indices[2]].position;

	Vector3 e1 = p2 - p1;
	Vector3 e2 = p3 - p1;
	Vector3 s1 = cross(ray.direction, e2);
	float divisor = dot(s1, e1);
	
	if (divisor == 0.)
		return 0;
	float invDivisor = 1.f / divisor;

	// Compute first barycentric coordinate
	Vector3 s = ray.origin - p1;
	float b1 = dot(s, s1) * invDivisor;
	if (b1 < 0. || b1 > 1.)
		return 0;
	
	// Compute second barycentric coordinate
	Vector3 s2 = cross(s, e1);
	float b2 = dot(ray.direction, s2) * invDivisor;
	if (b2 < 0. || b1 + b2 > 1.)
		return 0;
	
	// Compute _t_ to intersection point
	t = dot(e2, s2) * invDivisor;
	if (t < range.tmin || t > range.tmax)
		return 0;
	
	v = b1;
	w = b2;
	u = 1 - (b1 + b2);
	
	return 1;
}
	
	/*======================== X-tests ========================*/
#define AXISTEST_X01(a, b, fa, fb)			   \
p0 = a*v0[Y] - b*v0[Z];			       	   \
p2 = a*v2[Y] - b*v2[Z];			       	   \
if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];   \
if(min>rad || max<-rad) return 0;
	
#define AXISTEST_X2(a, b, fa, fb)			   \
p0 = a*v0[Y] - b*v0[Z];			           \
p1 = a*v1[Y] - b*v1[Z];			       	   \
if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];   \
if(min>rad || max<-rad) return 0;
	
	/*======================== Y-tests ========================*/
#define AXISTEST_Y02(a, b, fa, fb)			   \
p0 = -a*v0[X] + b*v0[Z];		      	   \
p2 = -a*v2[X] + b*v2[Z];	       	       	   \
if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];   \
if(min>rad || max<-rad) return 0;
	
#define AXISTEST_Y1(a, b, fa, fb)			   \
p0 = -a*v0[X] + b*v0[Z];		      	   \
p1 = -a*v1[X] + b*v1[Z];	     	       	   \
if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];   \
if(min>rad || max<-rad) return 0;
	
	/*======================== Z-tests ========================*/
	
#define AXISTEST_Z12(a, b, fa, fb)			   \
p1 = a*v1[X] - b*v1[Y];			           \
p2 = a*v2[X] - b*v2[Y];			       	   \
if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;} \
rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];   \
if(min>rad || max<-rad) return 0;
	
#define AXISTEST_Z0(a, b, fa, fb)			   \
p0 = a*v0[X] - b*v0[Y];				   \
p1 = a*v1[X] - b*v1[Y];			           \
if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];   \
if(min>rad || max<-rad) return 0;
	
inline void FindMinMax(float x0,float x1,float x2,float& min,float& max)
{
	min = max = x0;
	if(x1<min) min=x1;
	if(x1>max) max=x1;
	if(x2<min) min=x2;
	if(x2>max) max=x2;
}
	
inline int PlaneBoxOverlap(const Vector3& normal,float d, const Vector3& maxbox)
{
	int q;
	Vector3 vmin,vmax;
	for(q=0;q<=2;q++)
	{
		if(normal[q]>0.0f)
		{
			vmin[q]=-maxbox[q];
			vmax[q]=maxbox[q];
		}
		else
		{
			vmin[q]=maxbox[q];
			vmax[q]=-maxbox[q];
		}
	}
	if(dot(normal,vmin)+d>0.0f) return 0;
	if(dot(normal,vmax)+d>=0.0f) return 1;
	
	return 0;
}
	
inline int triBoxOverlap(const Vector3& boxcenter,const Vector3& boxhalfsize,const Vector3 triverts[3])
{
	
	const int X = 0;
	const int Y = 1;
	const int Z = 2;
	
	/*    use separating axis theorem to test overlap between triangle and box */
	/*    need to test for overlap in these directions: */
	/*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
	/*       we do not even need to test these) */
	/*    2) normal of the triangle */
	/*    3) crossproduct(edge from tri, {x,y,z}-directin) */
	/*       this gives 3x3=9 more tests */
	Vector3 v0,v1,v2;
	float axis[3];
	float min,max,d,p0,p1,p2,rad,fex,fey,fez;
	Vector3 normal,e0,e1,e2;
	
	/* 1) first test overlap in the {x,y,z}-directions */
	/*    find min, max of the triangle each direction, and test for overlap in */
	/*    that direction -- this is equivalent to testing a minimal AABB around */
	/*    the triangle against the AABB */
#if 1
	/* This is the fastest branch on Sun */
	/* move everything so that the boxcenter is in (0,0,0) */
	v0 = triverts[0] - boxcenter;
	v1 = triverts[1] - boxcenter;
	v2 = triverts[2] - boxcenter;
	
	/* test in X-direction */
	FindMinMax(v0[X],v1[X],v2[X],min,max);
	if(min>boxhalfsize[X] || max<-boxhalfsize[X]) return 0;
	
	/* test in Y-direction */
	FindMinMax(v0[Y],v1[Y],v2[Y],min,max);
	if(min>boxhalfsize[Y] || max<-boxhalfsize[Y]) return 0;
	
	/* test in Z-direction */
	FindMinMax(v0[Z],v1[Z],v2[Z],min,max);
	if(min>boxhalfsize[Z] || max<-boxhalfsize[Z]) return 0;
#else
	/*    another implementation */
	/*    test in X */
	v0[X]=triverts[0][X]-boxcenter[X];
	v1[X]=triverts[1][X]-boxcenter[X];
	v2[X]=triverts[2][X]-boxcenter[X];
	FindMinMax(v0[X],v1[X],v2[X],min,max);
	if(min>boxhalfsize[X] || max<-boxhalfsize[X]) return 0;
	
	/*    test in Y */
	v0[Y]=triverts[0][Y]-boxcenter[Y];
	v1[Y]=triverts[1][Y]-boxcenter[Y];
	v2[Y]=triverts[2][Y]-boxcenter[Y];
	FindMinMax(v0[Y],v1[Y],v2[Y],min,max);
	if(min>boxhalfsize[Y] || max<-boxhalfsize[Y]) return 0;
	
	/*    test in Z */
	v0[Z]=triverts[0][Z]-boxcenter[Z];
	v1[Z]=triverts[1][Z]-boxcenter[Z];
	v2[Z]=triverts[2][Z]-boxcenter[Z];
	FindMinMax(v0[Z],v1[Z],v2[Z],min,max);
	if(min>boxhalfsize[Z] || max<-boxhalfsize[Z]) return 0;
#endif
	
	/*    2) */
	/*    test if the box intersects the plane of the triangle */
	/*    compute plane equation of triangle: normal*x+d=0 */
	e0 = v1 - v0;      /* tri edge 0 */
	e1 = v2 - v1;      /* tri edge 1 */
	normal = cross(e0,e1);
	d=-dot(normal,v0);  /* plane eq: normal.x+d=0 */
	
	if(!PlaneBoxOverlap(normal,d,boxhalfsize)) return 0;
	
	/*    compute the last triangle edge */
	e2 = v0 -v2;
	
	/*    3) */
	fex = fabs(e0[X]);
	fey = fabs(e0[Y]);
	fez = fabs(e0[Z]);
	AXISTEST_X01(e0[Z], e0[Y], fez, fey);
	AXISTEST_Y02(e0[Z], e0[X], fez, fex);
	AXISTEST_Z12(e0[Y], e0[X], fey, fex);
	
	fex = fabs(e1[X]);
	fey = fabs(e1[Y]);
	fez = fabs(e1[Z]);
	AXISTEST_X01(e1[Z], e1[Y], fez, fey);
	AXISTEST_Y02(e1[Z], e1[X], fez, fex);
	AXISTEST_Z0(e1[Y], e1[X], fey, fex);
	
	
	fex = fabs(e2[X]);
	fey = fabs(e2[Y]);
	fez = fabs(e2[Z]);
	AXISTEST_X2(e2[Z], e2[Y], fez, fey);
	AXISTEST_Y1(e2[Z], e2[X], fez, fex);
	AXISTEST_Z12(e2[Y], e2[X], fey, fex);
	
	return 1;
}
	
	
}

#endif