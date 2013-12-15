




#include <winsock2.h>	// winsock2 needs to be included before windows.h
#include <windows.h>
#include <stdio.h>
#include "jpeglib.h"

class CTexture
{
public:
	unsigned int tex;
	char filepath[MAX_PATH];
	bool loaded;
	int width, height;

	CTexture()
	{
		loaded = false;
	}
};

#define TEXTURES	2048
extern CTexture g_texture[TEXTURES];

#define TGA_RGB		 2		// This tells us it's a normal RGB (really BGR) file
#define TGA_A		 3		// This tells us it's a ALPHA file
#define TGA_RLE		10		// This tells us that the targa is Run-Length Encoded (RLE)

#define png_infopp_NULL (png_infopp)NULL
#define int_p_NULL (int*)NULL
#define png_voidp_NULL	(png_voidp)NULL

#define JPEG_BUFFER_SIZE (8 << 10)

typedef struct 
{
    struct jpeg_source_mgr  pub;
} JPEGSource;

struct tImage
{
	int channels;			// The channels in the image (3 = RGB : 4 = RGBA)
	int sizeX;				// The width of the image in pixels
	int sizeY;				// The height of the image in pixels
	unsigned char *data;	// The image pixel data
};

struct tTextureToLoad
{
	unsigned int* tex;
	char filepath[MAX_PATH];
	bool clamp;
};

extern int g_texwidth;
extern int g_texheight;
extern int g_texindex;
extern int g_lastLTex;

tImage *LoadBMP(const char *strFileName);
tImage *LoadTGA(const char *strFileName);
void DecodeJPG(jpeg_decompress_struct* cinfo, tImage *pImageData);
tImage *LoadJPG(const char *strFileName);
tImage *LoadPNG(const char *strFileName);
bool FindTexture(unsigned int &texture, const char* filepath);
int NewTexture();
void TextureLoaded(unsigned int texture, const char* filepath, int id=-1);
void FindTextureExtension(char *strFileName);
void FreeTextures();
bool Load1Texture();
void QueueTexture(unsigned int* tex, const char* strFileName, bool clamp);
bool CreateTexture(unsigned int &texture, const char* strFileName, bool clamp, int id=-1);
void ReloadTextures();