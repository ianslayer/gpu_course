#ifndef GEOMETRY_H
#define GEOMETRY_H
#include "vector.h"
#include <algorithm>

struct AABB
{
    Vector3 center;
    Vector3 radius; //half width
};

struct Sphere
{
    Vector3 center;
    float   radius;
};

struct Ray
{
    Vector3 origin;
    Vector3 direction;
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

inline float TriangleArea(Vector3 v0, Vector3 v1, Vector3 v2)
{
    Vector3 v = v1 - v0;
    Vector3 w = v2 - v0;

    return 0.5f * ::sqrt(cross(v, w).SquaredLength() ); //don't use jog::sqrt because of the degenerate triangle
}

#endif