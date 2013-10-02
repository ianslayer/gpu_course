#include "mesh.h"
#include "primitive.h"

namespace jade
{

Primitive::Primitive()
    : mesh(0), translation(0.f)
{
    orientation.MakeIdentity();
}

Primitive::~Primitive()
{
}

Matrix4x4 Primitive::ModelMatrix() const
{
    Matrix4x4 modelMatrix(orientation[0][0], orientation[0][1], orientation[0][2], translation[0], 
                          orientation[1][0], orientation[1][1], orientation[1][2], translation[1], 
                          orientation[2][0], orientation[2][1], orientation[2][2], translation[2],
                          0, 0, 0, 1);

    return modelMatrix;
}

}