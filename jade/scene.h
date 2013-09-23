#ifndef SCENE_H
#define SCENE_H
#include <vector>

class Primitive;

class Scene
{
public:

    std::vector<Primitive*> primList;
};

#endif