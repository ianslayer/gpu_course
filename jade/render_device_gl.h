#ifndef RENDER_DEVICE_GL_H
#define RENDER_DEVICE_GL_H
#include "../gl_utility.h"

namespace jade
{
    class VertexBufferGL
    {
    public:
        VertexBufferGL();
        ~VertexBufferGL();

        GLuint vboID;
    };

    class IndexBufferGL
    {
    public:
        IndexBufferGL();
        ~IndexBufferGL();

        GLuint iboID;
    };

	class Texture2DGL
	{
	public:
		Texture2DGL();
		~Texture2DGL();

		GLuint id;
	};

	class TextureSamplerStateGL
	{
	public:
		GLint minFilter;
		GLint magFilter;
		GLint mipFilter;

		GLint uAddress;
		GLint vAddress;
		GLint wAddress;

		unsigned int maxAnisotropic;
		GLuint sampler;
	};

    typedef VertexBufferGL VertexBufferImpl;
    typedef IndexBufferGL IndexBufferImpl;
	typedef Texture2DGL Texture2DImpl;
	typedef TextureSamplerStateGL TextureSamplerStateImpl;

}

#endif