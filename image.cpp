#include "GL/gl3w.h"
#include <stdio.h>
#include "file_utility.h"


GLuint GenerateWhiteTexture()
{
    GLuint defaultTexture;
    glGenTextures(1, &defaultTexture);
    glBindTexture(GL_TEXTURE_2D, defaultTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //IMPORTANT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //IMPORTANT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    unsigned char texel[] = {255, 255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texel);

    return defaultTexture;
}

static void HorizontalFlip( unsigned char *data, int width, int height ) 
{
    int		i, j;
    int		temp;

    for ( i = 0 ; i < height ; i++ ) 
    {
        for ( j = 0 ; j < width / 2 ; j++ ) 
        {
            temp = *( (int *)data + i * width + j );
            *( (int *)data + i * width + j ) = *( (int *)data + i * width + width - 1 - j );
            *( (int *)data + i * width + width - 1 - j ) = temp;
        }
    }
}

static void VerticalFlip( unsigned char *data, int width, int height ) 
{
    int		i, j;
    int		temp;

    for ( i = 0 ; i < width ; i++ ) 
    {
        for ( j = 0 ; j < height / 2 ; j++ ) 
        {
            temp = *( (int *)data + j * width + i );
            *( (int *)data + j * width + i ) = *( (int *)data + ( height - 1 - j ) * width + i );
            *( (int *)data + ( height - 1 - j ) * width + i ) = temp;
        }
    }
}

// TGA types
enum TGAType {
    TGA_TYPE_INDEXED		= 1,
    TGA_TYPE_RGB			= 2,
    TGA_TYPE_GREY			= 3,
    TGA_TYPE_RLE_INDEXED	= 9,
    TGA_TYPE_RLE_RGB		= 10,
    TGA_TYPE_RLE_GREY		= 11
};

#define TGA_intERLEAVE_MASK	0xc0
#define TGA_intERLEAVE_NONE	0x00
#define TGA_intERLEAVE_2WAY	0x40
#define TGA_intERLEAVE_4WAY	0x80

#define TGA_ORIGIN_MASK		0x30
#define TGA_ORIGIN_LEFT		0x00
#define TGA_ORIGIN_RIGHT	0x10
#define TGA_ORIGIN_LOWER	0x00
#define TGA_ORIGIN_UPPER	0x20

#define TGA_HAS_ALPHA		0x0F

#pragma pack(push, 1) //must add this to prevent padding
/// Tga Header.
struct TgaHeader {
    unsigned char id_length, colormap_type, image_type;
    unsigned short colormap_index, colormap_length;
    unsigned char colormap_size;
    unsigned short x_origin, y_origin, width, height;
    unsigned char pixel_size, attributes;

    enum { Size = 18 };		//const static int SIZE = 18;
};	
#pragma pack(pop)

struct Color555 {
    unsigned short  b : 5;
    unsigned short  g : 5;
    unsigned short  r : 5;
};

static void LoadTGA(const unsigned char* _InputBuf, int _InputBufLength, unsigned char ** _OutputBuf, int *_Width, int *_Height)
{
    TgaHeader Header;

    const unsigned char* buf = _InputBuf;

    Header.id_length = *buf++;
    Header.colormap_type = *buf++;
    Header.image_type = *buf++;

    //FIXME: unsigned short part is endian related, TGA is stored in little endian
    Header.colormap_index = ( *(unsigned short*) buf);
    buf+=2;
    Header.colormap_length = ( *(unsigned short*) buf);
    buf+=2;
    Header.colormap_size = *buf++;
        
    Header.x_origin = ( *(unsigned short*) buf);
    buf+=2;
    Header.y_origin = ( *(unsigned short*) buf);
    buf+=2;
    Header.width = ( *(unsigned short*) buf);
    buf+=2;
    Header.height = ( *(unsigned short*) buf);
    buf+=2;

    Header.pixel_size = *buf++;
    Header.attributes = *buf++;

    if(Header.image_type != TGA_TYPE_RGB && Header.image_type != TGA_TYPE_GREY && Header.image_type != TGA_TYPE_RLE_RGB)
    {
        printf("unsupported tga image type\n");
    }

    if(Header.colormap_type != 0)
    {
        printf("tga colormaps not supported\n");
    }

    if( (Header.pixel_size != 32 && Header.pixel_size != 24) && Header.image_type != TGA_TYPE_GREY )
    {
        printf("only 32 or 24 bit images supported\n");
    }

    /*
    if(Header.image_type == TGA_TYPE_INDEXED && (Header.colormap_type != 1 || Header.colormap_size != 24 || Header.colormap_length > 256) )
    {
        printf("unsupported indexed tga image\n");
    }*/

    const int Columns = Header.width;
    const int Rows = Header.height;

    if(_Width)
    {
        *_Width = Columns;
    }
    if(_Height)
    {
        *_Height = Rows;
    }

    const unsigned char* Palette = NULL;
    if(Header.image_type == TGA_TYPE_INDEXED)
    {
        Palette = buf;
        buf += (3 * Header.colormap_length);
    }

    unsigned char* TgaRgbaBuf = new unsigned char[Columns * Rows * 4];
    *_OutputBuf = TgaRgbaBuf;

    if(Header.id_length != 0) 
    {
        buf+=Header.id_length; //skip tga image comment
    }

    if(Header.image_type == TGA_TYPE_RGB || Header.image_type == TGA_TYPE_GREY )
    {
        for(int Row = Rows - 1; Row >= 0; Row--)
        {
            unsigned char* PixBuf = TgaRgbaBuf + Row * Columns *4;
            for(int Column = 0; Column < Columns; Column++)
            {
                unsigned char Red, Green, Blue, Alpha;
                switch(Header.pixel_size)
                {
                case 8:
                    Blue = *buf++;
                    Green = Blue;
                    Red = Blue;
                    *PixBuf++ = Red;
                    *PixBuf++ = Green;
                    *PixBuf++ = Blue;
                    *PixBuf++ = 255;
                    break;
                case 24:
                    Blue = *buf++;
                    Green = *buf++;
                    Red = *buf++;
                    *PixBuf++ = Red;
                    *PixBuf++ = Green;
                    *PixBuf++ = Blue;
                    *PixBuf++ = 255;
                    break;
                case 32:
                    Blue = *buf++;
                    Green = *buf++;
                    Red = *buf++;
                    Alpha = *buf++;
                    *PixBuf++ = Red;
                    *PixBuf++ = Green;
                    *PixBuf++ = Blue;
                    *PixBuf++ = Alpha;
                    break;
                default:
                    printf("unsupported tga pixel size");
                    break;
                }
            }
        }
    }
    else if(Header.image_type == TGA_TYPE_RLE_RGB )
    {
        unsigned char Red, Green, Blue, Alpha, PacketHeader, PacketSize;

        for( int Row = Rows - 1; Row >= 0; Row--) 
        {
                unsigned char* PixBuf = TgaRgbaBuf + Row * Columns * 4;
                for(int Column = 0; Column < Columns; )
                {
                    PacketHeader = *buf++;
                    PacketSize = 1 + (PacketHeader & 0x7f);
                    if(PacketHeader & 0x80)   //run-length packet
                    {
                        switch(Header.pixel_size)
                        {
                            case 24:
                                Blue = *buf++;
                                Green = *buf++;
                                Red = *buf++;
                                Alpha = 255;
                                break;
                            case 32:
                                Blue = *buf++;
                                Green = *buf++;
                                Red = *buf++;
                                Alpha = *buf++;
                                break;
                            default:
                                printf("illegal pixel_size");
                                break;
                        }

                        for( int i = 0; i < PacketSize; i++)
                        {
                            *PixBuf++ = Red;
                            *PixBuf++ = Green;
                            *PixBuf++ = Blue;
                            *PixBuf++ = Alpha;
                            Column++;
                            if(Column == Columns) //run spans across rows. Is this necessary? I read TGA spec said this can not happen
                            {
                                Column = 0;
                                if(Row > 0)
                                {
                                    Row--;
                                }
                                else
                                {
                                    goto breakOut;
                                }
                                PixBuf = TgaRgbaBuf + Row * Column * 4;
                            }
                        }
                    }
                    else // non run length packet
                    {
                        for(int i = 0; i < PacketSize; i++)
                        {
                            switch(Header.pixel_size)
                            {
                            case 24:
                                Blue = *buf++;
                                Green = *buf++;
                                Red = *buf++;
                                *PixBuf++ = Red;
                                *PixBuf++ = Green;
                                *PixBuf++ = Blue;
                                *PixBuf++ = 255;
                                break;
                            case 32:
                                Blue = *buf++;
                                Green = *buf++;
                                Red = *buf++;
                                Alpha = *buf++;
                                *PixBuf++ = Red;
                                *PixBuf++ = Green;
                                *PixBuf++ = Blue;
                                *PixBuf++ = Alpha;
                                break;
                            default:
                                printf("illegal pixel_size");
                                break;
                            }
                            Column++;
                            if(Column == Columns) //run spans across rows. Is this necessary? I read TGA spec said this can not happen
                            {
                                Column = 0;
                                if(Row > 0)
                                {
                                    Row--;
                                }
                                else
                                {
                                    goto breakOut;
                                }
                                PixBuf = TgaRgbaBuf + Row * Column * 4;
                            }
                        }
                    }

                    breakOut: ;
                }
        }   

    }


    if(Header.attributes & TGA_ORIGIN_UPPER )
    {
        VerticalFlip(*_OutputBuf, *_Width, *_Height);
    }
       
}

bool LoadTGA(const char* path, unsigned char** imgBuffer, int* width, int* height)
{
	int fileSize = FileSize(std::string(path));
    unsigned char* buf = new unsigned char[fileSize];
    int readFileLen = ReadFile(std::string(path), buf, fileSize);

    if(readFileLen <= 0)
        return false;

    unsigned char* imgbuf = NULL;

    LoadTGA(buf, fileSize, &imgbuf, width, height);

	delete [] buf;

    if(imgbuf)
    {
        *imgBuffer = imgbuf;
        return true;
    }

    return false;
}


struct BMPHeader
{
    char id[2];
    unsigned int fileSize;
    unsigned int reserved0;
    unsigned int bitmapDataOffset;
    unsigned int bitmapHeaderSize;
    unsigned int width;
    unsigned int height;
    unsigned short planes;
    unsigned short bitsPerPixel;
    unsigned int compression;
    unsigned int bitmapDataSize;
    unsigned int hRes;
    unsigned int vRes;
    unsigned int colors;
    unsigned int importantColors;
    unsigned char palette[256][4];
} ;

static void LoadBMP(const unsigned char* _InputBuf, int _InputBufLength, unsigned char ** _OutputBuf, int *_Width, int *_Height)
{
    BMPHeader bmpHeader;
    const unsigned char* buf = _InputBuf;

    bmpHeader.id[0] = *buf++;
    bmpHeader.id[1] = *buf++;

    //FIXME: endian related
    bmpHeader.fileSize = (*(int*) buf);
    buf+=4;
    bmpHeader.reserved0 = (*(int*) buf);
    buf+=4;
    bmpHeader.bitmapDataOffset = (*(int*)buf);
    buf+=4;
    bmpHeader.bitmapHeaderSize = (*(int*)buf);
    buf+=4;
    bmpHeader.width = (*(int*)buf);
    buf+=4;
    bmpHeader.height = (*(int*)buf);
    buf+=4;
    bmpHeader.planes = (*(unsigned short*)buf);
    buf+=2;
    bmpHeader.bitsPerPixel = (*(unsigned short*)buf);
    buf+=2;
    bmpHeader.compression = (*(unsigned short*)buf);
    buf+=4;
    bmpHeader.bitmapDataSize = (*(unsigned short*)buf);
    buf+=4;
    bmpHeader.hRes = (*(int*)buf);
    buf+=4;
    bmpHeader.vRes = (*(int*)buf);
    buf+=4;
    bmpHeader.colors = (*(int*)buf);
    buf+=4;
    bmpHeader.importantColors = (*(int*)buf);
    buf+=4;

    memcpy(bmpHeader.palette, buf, sizeof(bmpHeader.palette) );

    if(bmpHeader.bitsPerPixel == 8) //palette
        buf+= 1024;

    if(bmpHeader.id[0] != 'B' && bmpHeader.id[1] != 'M')
    {
        printf("LoadBMP: only Windows-style BMP files supported \n");
    }

    if(bmpHeader.fileSize != _InputBufLength)
    {
        printf("LoadBMP: header size does not match file size\n");
    }

    if(bmpHeader.compression != 0)
    {
        printf("LoadBMP: only uncompressed BMP files supported\n");
    }

    if(bmpHeader.bitsPerPixel < 8)
    {
        printf("LoadBMP: monochrome and 4-bit BMP files not supported\n");
    }

    int columns = bmpHeader.width;
    int rows = bmpHeader.height;

    if(rows < 0) //what the...?
        rows = -rows;
    int numPixels = columns * rows;
    if(_Width)
        *_Width = columns;
    if(_Height)
        *_Height = rows;

    unsigned char* bmpRGBA = new unsigned char[numPixels * 4];
    *_OutputBuf = bmpRGBA;

    for(int row = rows - 1; row > 0; row--)
    {
        unsigned char* pixBuf = bmpRGBA + row*columns*4;
        unsigned char red, green, blue, alpha;
        int palIndex;
        unsigned short shortPixel ;
        for(int column = 0; column < columns; column++ )
        {
            switch(bmpHeader.bitsPerPixel)
            {
            case 8:
                palIndex = *buf++;
                *pixBuf++ = bmpHeader.palette[palIndex][2];
                *pixBuf++ = bmpHeader.palette[palIndex][1];
                *pixBuf++ = bmpHeader.palette[palIndex][0];
                break;
            case 16:
                shortPixel = *(unsigned short*) buf;
                buf+=2;
                *pixBuf++ = ( shortPixel & ( 31 << 10 ) ) >> 7;
                *pixBuf++ = ( shortPixel & ( 31 << 5 ) ) >> 2;
                *pixBuf++ = ( shortPixel & ( 31 ) ) << 3;
                *pixBuf++ = 0xff;
                break;

            case 24:
                blue = *buf++;
                green = *buf++;
                red = *buf++;
                *pixBuf++ = red;
                *pixBuf++ = green;
                *pixBuf++ = blue;
                *pixBuf++ = 255;
                break;
            case 32:
                blue = *buf++;
                green = *buf++;
                red = *buf++;
                alpha = *buf++;
                *pixBuf++ = red;
                *pixBuf++ = green;
                *pixBuf++ = blue;
                *pixBuf++ = alpha;
                break;
            default:
                printf("LoadBMP: illegal pixel_size\n");
                break;
            }
        }
    }
}

bool LoadBMP(const char* path, unsigned char** imgBuffer, int* width, int* height)
{
	int fileSize = FileSize(std::string(path));
	unsigned char* buf = new unsigned char[fileSize];
	int readFileLen = ReadFile(std::string(path), buf, fileSize);

	if(readFileLen <= 0)
		return false;

    unsigned char* imgbuf = NULL;

    LoadBMP(buf, fileSize, &imgbuf, width, height);

    if(imgbuf)
    {
        *imgBuffer = imgbuf;
        return true;
    }

    return false;
}


void FreeImageBuffer(const unsigned char* imgBuffer)
{
    delete [] imgBuffer;
}

GLuint CreateGLImage(const unsigned char* imgBuffer, int width, int height)
{
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    
    /*
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //IMPORTANT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //IMPORTANT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    */

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imgBuffer);

    return texID;
}

GLuint CreateMipmapGLImage(const unsigned char* imgBuffer, int width, int height)
{
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    /*
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    */

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imgBuffer);
    glGenerateMipmap(GL_TEXTURE_2D);

    return texID;
}
