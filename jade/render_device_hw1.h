#ifndef RENDER_DEVICE_HW1
#define RENDER_DEVICE_HW1
#include "../gl_utility.h"

namespace jade
{
    class VertexBufferHW
    {
    public:
        VertexBufferHW();
        ~VertexBufferHW();
        
        GLuint vboID;
    };
    
    class IndexBufferHW
    {
    public:
        IndexBufferHW();
        ~IndexBufferHW();
        
        GLuint iboID;
    };
    
	class Texture2DHW
	{
	public:
		Texture2DHW();
		~Texture2DHW();
        
		GLuint id;
	};
    
	class TextureSamplerStateHW
	{
	public:
        TextureSamplerStateHW();
        ~TextureSamplerStateHW();
        
		GLint minFilter;
		GLint magFilter;
		GLint mipFilter;
        
		GLint uAddress;
		GLint vAddress;
		GLint wAddress;
        
		unsigned int maxAnisotropic;
		GLuint sampler;
	};
    
    typedef VertexBufferHW VertexBufferImpl;
    typedef IndexBufferHW IndexBufferImpl;
	typedef Texture2DHW Texture2DImpl;
	typedef TextureSamplerStateHW TextureSamplerStateImpl;
    
}


#endif 
