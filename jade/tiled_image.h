#ifndef TILED_IMAGE_H
#define TILED_IMAGE_H

namespace jade
{

template <class T, int tile_dim>
class Tile
{
public:
	T& Pixel(int x, int y)
	{
		return p[y * tile_dim + x];
	}
	
	const T& Pixel(int x, int y) const
	{
		return p[y*tile_dim + x];
	}
	
	T p[tile_dim * tile_dim];
};
	
template<class T, int tile_dim>
class TiledImage
{
public:
	TiledImage(int width, int height);
	TiledImage(const T* inputBuf, int width, int height);
	
	~TiledImage();
	
	int XTileCount() const;
	int YTileCount() const;
	int TileCount() const;
	
	Tile<T, tile_dim>& GetTile(int x, int y);
	const Tile<T, tile_dim>& GetTile(int x, int y) const;
	
	Tile<T, tile_dim>& GetTile(int linearIdx);
	const Tile<T, tile_dim>& GetTile(int linearIdx) const;
	
	T& Pixel(int x, int y);
	const T& Pixel(int x, int y) const;
	
	void Linearize(T* outBuf);
	
private:
	Tile<T, tile_dim>* tileList;
	
	int width;
	int height;
};

template<class T, int tile_dim> inline
TiledImage<T, tile_dim>::TiledImage(int _width, int _height) : width(_width), height(_height)
{
	tileList = new Tile<T, tile_dim>[TileCount()];
}

template<class T, int tile_dim> inline
TiledImage<T, tile_dim>::TiledImage(const T* inputBuf, int _width, int _height) : width(_width), height(_height)
{
	tileList = new Tile<T, tile_dim>[TileCount()];
	
	for(int i = 0; i < height; i++)
	{
		for(int j = 0; j < width; j++)
		{
			Pixel(j, i) = inputBuf[i * width + j];
		}
	}
}
	
template<class T, int tile_dim> inline
TiledImage<T, tile_dim>::~TiledImage()
{
	delete[] tileList;
}

template<class T, int tile_dim> inline
int TiledImage<T, tile_dim>::XTileCount() const
{
	return (int) ceilf((float)width / (float)tile_dim);
}
	
template<class T, int tile_dim> inline
int TiledImage<T, tile_dim>::YTileCount() const
{
	return (int) ceilf((float)height / (float)tile_dim);
}
	
template<class T, int tile_dim> inline
int TiledImage<T, tile_dim>::TileCount() const
{
	return XTileCount() * YTileCount();
}
	
template<class T, int tile_dim> inline
Tile<T, tile_dim>& TiledImage<T, tile_dim>::GetTile(int x, int y)
{
	int xIndex = x / tile_dim;
	int yIndex = y / tile_dim;
	
	return tileList[yIndex * XTileCount() + xIndex ];
}

template<class T, int tile_dim> inline
const Tile<T, tile_dim>& TiledImage<T, tile_dim>::GetTile(int x, int y) const
{
	int xIndex = x / tile_dim;
	int yIndex = y / tile_dim;
	
	return tileList[yIndex * XTileCount() + xIndex ];
}
	
template<class T, int tile_dim> inline
Tile<T, tile_dim>& TiledImage<T, tile_dim>::GetTile(int linearIdx)
{
	return tileList[linearIdx];
}
	
template<class T, int tile_dim> inline
const Tile<T, tile_dim>& TiledImage<T, tile_dim>::GetTile(int linearIdx) const
{
	return tileList[linearIdx];
}
	
template<class T, int tile_dim> inline
T& TiledImage<T, tile_dim>::Pixel(int x, int y)
{
	int xOffset = x % tile_dim;
	int yOffset = y % tile_dim;
	
	return GetTile(x, y).Pixel(xOffset, yOffset);
}
	
template<class T, int tile_dim> inline
const T& TiledImage<T, tile_dim>::Pixel(int x, int y) const
{
	int xOffset = x % tile_dim;
	int yOffset = y % tile_dim;
	
	return GetTile(x, y).Pixel(xOffset, yOffset);
}
	
template<class T, int tile_dim> inline
void TiledImage<T, tile_dim>::Linearize(T *outBuf)
{
	for(int i = 0; i < height; i++)
	{
		for(int j = 0; j < width; j++)
		{
			 outBuf[i * width + j] = Pixel(j, i);
		}
	}
}
	
}



#endif
