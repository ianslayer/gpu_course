#ifndef RENDERER_H
#define RENDERER_H

class Vector3;
namespace jade
{
    class Scene;
    class Camera;
	class Texture;
	class AABB;

    class Renderer
    {
    public:
        virtual ~Renderer() {}
        virtual void Render(const Camera* camera, const Scene* scene) = 0;
		virtual void ScreenShot(const char* path, const Camera* camera, const Scene* scene) {};
		virtual void ScreenShot(const char* path, const Camera* camera, const Scene* scene, Texture* tex) {};
        virtual void SetRendererOption(void* options) {}

		virtual void DrawBoundingBox(const Camera* camera, const AABB& bound) {} //debug hack
		virtual void DrawLine(const Camera* camera, const Vector3& p0, const Vector3& p1, const Vector3& color) {} //debug hack
    };

}

#endif