#ifndef GFX_H
#define GFX_H

#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef __cplusplus
extern "C"
{
#endif

//Structs
typedef struct
{
	uint8_t r, g, b, a;
} clr;

typedef struct
{
	FT_Library lib;
	FT_Face face[6];
	FT_Error libRet, faceRet;
	//Loads to buffer for speed for TTF
	uint8_t *fntData;
	bool external;
} font;

typedef struct
{
	size_t size;
	unsigned width, height;
	uint32_t *data;
} tex;

//Inits needed graphics stuff
bool graphicsInit(int windowWidth, int windowHeight);

//Exits needed services
bool graphicsExit();

void gfxBeginFrame();
void gfxEndFrame(const font *f);

//Creates color from uint32_t
inline clr clrCreateU32(uint32_t color)
{
	clr ret;
	ret.a = color >> 24 & 0xFF;
	ret.b = color >> 16 & 0xFF;
	ret.g = color >> 8 & 0xFF;
	ret.r = color & 0xFF;
	return ret;
}

//Sets clr to [r], [g], [b], [a]
inline clr clrCreateRGBA(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a)
{
	clr ret;
	ret.r = _r;
	ret.g = _g;
	ret.b = _b;
	ret.a = _a;
	return ret;
}

//Returns uint32_t color
inline uint32_t clrGetColor(const clr c)
{
	return (c.a << 24 | c.b << 16 | c.g << 8 | c.r);
}

//Draws text using f
void drawText(const char *str, tex *target, const font *f, float x, float y, float sz, clr c);
void drawTextBound(const char *str, tex *target, const font *f, float x, float y, float sz, clr c, int top, int bottom);

//Draws text wrapping lines
void drawTextWrap(const char *str, tex *target, const font *f, float x, float y, float sz, clr c, int maxWidth);

//Returns text width
size_t textGetWidth(const char *str, const font *f, float sz);
size_t textGetHeight(const char *str, const font *f, float sz, int maxWidth);

//Draws rectangle at x, y with w, h
void drawRect(tex *target, int x, int y, int w, int h, const clr c);

//Draws rect with alpha.
void drawRectAlpha(tex *target, int x, int y, int w, int h, const clr c);

/*
TEX BEGIN
*/
//Inits empty tex
tex *texCreate(int w, int h);

//Loads PNG from path
tex *texLoadPNGFile(const char *path);

//Loads JPEG from path
tex *texLoadJPEGFile(const char *path);

//Loads jpeg from memory
tex *texLoadJPEGMem(const uint8_t *jpegData, size_t jpegSize);

//Frees memory used by t
void texDestroy(tex *t);

//Clears tex completely with c
void texClearColor(tex *in, const clr c);

//Draws t at x, y
void texDraw(const tex *t, tex *target, int x, int y);
void texDrawLimit(const tex *t, tex *target, int x, int y);
void texDrawHorizontal(const tex *t, tex *target, int x, int y, int w);
void texDrawVertical(const tex *t, tex *target, int x, int y, int h);

//Draws without alpha blending, faster
void texDrawNoAlpha(const tex *t, tex *target, int x, int y);

//Real resize, slow
void texDrawResize(const tex *t, tex *target, int x, int y, int w, int h);

//Replaces old with newColor
void texSwapColors(tex *t, const clr old, const clr newColor);

//Creates and copies data from another image returns tex
tex *texCreateFromPart(const tex *src, int x, int y, int w, int h);
/*
TEX END
*/

//Loads and returns font with Switch shared font loaded
font *fontLoadSharedFonts();

//Loads and returns TTF font
font *fontLoadTTF(const char *path);

//Frees font
void fontDestroy(font *f);

//returns framebuffer tex pointer
extern tex *frameBuffer;
#ifdef __cplusplus
}
#endif

#endif // GFX_H
