#include <switch.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <png.h>
#include <jpeglib.h>
#include <time.h>

#include "gfx.h"

tex *frameBuffer;
clr textClr;

#define BILLION 1000000000L

static NWindow *window;
static Framebuffer fb;
static bool framestarted = false;
struct timespec start, end;
char char_arr[200];

static inline uint32_t blend(const clr px, const clr fb)
{
	if(px.a == 0x00)
		return clrGetColor(fb);
	else if(px.a == 0xFF)
		return clrGetColor(px);

	uint8_t subAl = 0xFF - px.a;

	uint8_t fR = (px.r * px.a + fb.r * subAl) / 0xFF;
	uint8_t fG = (px.g * px.a + fb.g * subAl) / 0xFF;
	uint8_t fB = (px.b * px.a + fb.b * subAl) / 0xFF;

	return (0xFF << 24 | fB << 16 | fG << 8 | fR);
}

static inline uint32_t smooth(const clr px1, const clr px2)
{
	uint8_t fR = (px1.r + px2.r) / 2;
	uint8_t fG = (px1.g + px2.g) / 2;
	uint8_t fB = (px1.b + px2.b) / 2;
	uint8_t fA = (px1.a + px2.a) / 2;

	return (fA << 24 | fB << 16 | fG << 8 | fR);
}

bool graphicsInit(int windowWidth, int windowHeight)
{
	window = nwindowGetDefault();
	nwindowSetDimensions(window, windowWidth, windowHeight);

	framebufferCreate(&fb, window, windowWidth, windowHeight, PIXEL_FORMAT_RGBA_8888, 2);
	framebufferMakeLinear(&fb);
	plInitialize();

	//Make a fake tex that points to framebuffer
	frameBuffer = malloc(sizeof(tex));
	frameBuffer->width = windowWidth;
	frameBuffer->height = windowHeight;
	frameBuffer->size = windowWidth * windowHeight;

	return true;
}

bool graphicsExit()
{
	free(frameBuffer);

	plExit();
	framebufferClose(&fb);
	nwindowClose(window);

	return true;
}

void gfxBeginFrame()
{
	clock_gettime(CLOCK_MONOTONIC, &start);
	if(!framestarted)
	{
		frameBuffer->data = (uint32_t *)framebufferBegin(&fb, NULL);
		framestarted = true;
	}
}

void gfxEndFrame(const font *f)
{
/*
	uint64_t diff;
	clock_gettime(CLOCK_MONOTONIC, &end);
	diff = (BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec) / 1000000;
	sprintf(char_arr, "%llu ms\n", (long long unsigned int) diff);
	drawText(char_arr, frameBuffer, f, 1200, 10, 14, clrCreateU32(0xFF464646));
*/
	if(framestarted)
	{
		framebufferEnd(&fb);
		framestarted = false;
	}
}

static void drawGlyphBound(const FT_Bitmap *bmp, tex *target, int _x, int _y, int top, int bottom)
{
	if(bmp->pixel_mode != FT_PIXEL_MODE_GRAY)
		return;

	uint8_t *bmpPtr = bmp->buffer;
	for(int y = _y; y < _y + bmp->rows; y++)
	{
		if(y > target->height || y < 0)
			continue;

		uint32_t *rowPtr = &target->data[y * target->width + _x];
		for(int x = _x; x < _x + bmp->width; x++, bmpPtr++, rowPtr++)
		{
			if(x > target->width || x < 0 || y < top || y > bottom)
				continue;

			if(*bmpPtr > 0)
			{
				clr txClr = clrCreateRGBA(textClr.r, textClr.g, textClr.b, *bmpPtr);
				clr tgtClr = clrCreateU32(*rowPtr);

				*rowPtr = blend(txClr, tgtClr);
			}
		}
	}
}

static void drawGlyph(const FT_Bitmap *bmp, tex *target, int _x, int _y)
{
	drawGlyphBound(bmp, target, _x, _y, 0, target->height);
}

static inline void resizeFont(const font *f, float sz)
{
	if(f->external)
		FT_Set_Char_Size(f->face[0], 0, sz * 64, 90, 90);
	else
	{
		for(int i = 0; i < 6; i++)
			FT_Set_Char_Size(f->face[i], 0, sz * 64, 90, 90);
	}
}

static inline FT_GlyphSlot loadGlyph(const uint32_t c, const font *f, FT_Int32 flags)
{
	if(f->external)
	{
		FT_Load_Glyph(f->face[0], FT_Get_Char_Index(f->face[0], c), flags);
		return f->face[0]->glyph;
	}
	for(int i = 0; i < 6; i++)
	{
		FT_UInt cInd = 0;
		if( (cInd = FT_Get_Char_Index(f->face[i], c)) != 0 && \
				FT_Load_Glyph(f->face[i], cInd, flags) == 0)
		{
			return f->face[i]->glyph;
		}
	}

	return NULL;
}

void drawText(const char *str, tex *target, const font *f, float x, float y, float sz, clr c)
{
	drawTextBound(str, target, f, x, y, sz, c, 0, target->height);
}

void drawTextBound(const char *str, tex *target, const font *f, float x, float y, float sz, clr c, int top, int bottom)
{
	float tmpX = x;
	uint32_t tmpChr = 0;
	ssize_t unitCnt = 0;
	textClr = c;

	resizeFont(f, sz);

	size_t length = strlen(str);
	for(unsigned i = 0; i < length; )
	{
		unitCnt = decode_utf8(&tmpChr, (const uint8_t *)&str[i]);
		if(unitCnt <= 0)
			break;

		i += unitCnt;
		switch(tmpChr)
		{
			case '\n':
				tmpX = x;
				y += sz + 11;
				continue;
				break;

			case '"':
			case '#':
				if(clrGetColor(textClr) == 0xFFEE9900)
					textClr = c;
				else
					textClr = clrCreateU32(0xFFEE9900);
				continue;
				break;

			case '*':
				if(clrGetColor(textClr) == 0xFF0000FF)
					textClr = c;
				else
					textClr = clrCreateU32(0xFF0000FF);
				continue;
				break;
		}

		FT_GlyphSlot slot = loadGlyph(tmpChr, f, FT_LOAD_RENDER);
		if(slot != NULL)
		{
			float drawY = y + (sz - slot->bitmap_top);
			drawGlyphBound(&slot->bitmap, target, tmpX + slot->bitmap_left, drawY, top, bottom);

			tmpX += slot->advance.x >> 6;
		}
	}
}

void drawTextWrap(const char *str, tex *target, const font *f, float x, float y, float sz, clr c, int maxWidth)
{
	char wordBuf[128];
	size_t nextbreak = 0;
	size_t strLength = strlen(str);
	float tmpX = x;
	for(unsigned i = 0; i < strLength; )
	{
		nextbreak = strcspn(&str[i], " /");

		memset(wordBuf, 0, 128);
		memcpy(wordBuf, &str[i], nextbreak + 1);

		size_t width = textGetWidth(wordBuf, f, sz);

		if(tmpX + width >= x + maxWidth)
		{
			tmpX = x;
			y += sz + 17;
		}

		size_t wLength = strlen(wordBuf);
		uint32_t tmpChr = 0;
		for(unsigned j = 0; j < wLength; )
		{
			ssize_t unitCnt = decode_utf8(&tmpChr, (const uint8_t *)&wordBuf[j]);
			if(unitCnt <= 0)
				break;

			j += unitCnt;
			switch(tmpChr)
			{
				case '\n':
					tmpX = x;
					y += sz + 17;
					continue;
					break;
				/*
				case '"':
				case '#':
					if(clrGetColor(textClr) == 0xFFEE9900)
						textClr = c;
					else
						textClr = clrCreateU32(0xFFEE9900);
					continue;
					break;

				case '*':
					if(clrGetColor(textClr) == 0xFF0000FF)
						textClr = c;
					else
						textClr = clrCreateU32(0xFF0000FF);
					continue;
					break;
				*/
			}
			textClr = c;

			FT_GlyphSlot slot = loadGlyph(tmpChr, f, FT_LOAD_RENDER);
			if(slot != NULL)
			{
				int drawY = y + (sz - slot->bitmap_top);
				drawGlyph(&slot->bitmap, target, tmpX + slot->bitmap_left, drawY);

				tmpX += slot->advance.x >> 6;
			}
		}

		i += strlen(wordBuf);
	}
}

size_t textGetWidth(const char *str, const font *f, float sz)
{
	size_t width = 0;
	uint32_t untCnt = 0, tmpChr = 0;
	FT_Error ret = 0;

	resizeFont(f, sz);

	size_t length = strlen(str);
	for(unsigned i = 0; i < length; )
	{
		untCnt = decode_utf8(&tmpChr, (const uint8_t *)&str[i]);

		if(untCnt <= 0)
			break;

		i += untCnt;
		FT_GlyphSlot slot = loadGlyph(tmpChr, f, FT_LOAD_DEFAULT);
		if(ret)
			return 0;

		width += slot->advance.x >> 6;
	}

	return width;
}

size_t textGetHeight(const char *str, const font *f, float sz, int maxWidth)
{
	char wordBuf[128];
	size_t nextbreak = 0;
	size_t strLength = strlen(str);
	float tmpX = 0, height = sz;
	for(unsigned i = 0; i < strLength; )
	{
		nextbreak = strcspn(&str[i], " /");

		memset(wordBuf, 0, 128);
		memcpy(wordBuf, &str[i], nextbreak + 1);

		size_t width = textGetWidth(wordBuf, f, sz);

		if(tmpX + width >= 0 + maxWidth)
		{
			tmpX = 0;
			height += sz + 17;
		}

		size_t wLength = strlen(wordBuf);
		uint32_t tmpChr = 0;
		for(unsigned j = 0; j < wLength; )
		{
			ssize_t unitCnt = decode_utf8(&tmpChr, (const uint8_t *)&wordBuf[j]);
			if(unitCnt <= 0)
				break;

			j += unitCnt;
			switch(tmpChr)
			{
				case '\n':
					tmpX = 0;
					height += sz + 8;
					continue;
					break;
			}

			FT_GlyphSlot slot = loadGlyph(tmpChr, f, FT_LOAD_RENDER);
			if(slot != NULL)
			{
				tmpX += slot->advance.x >> 6;
			}
		}

		i += strlen(wordBuf);
	}

	return height;
}

void drawRect(tex *target, int x, int y, int w, int h, const clr c)
{
	uint32_t clr = clrGetColor(c);

	for(int tY = y; tY < y + h; tY++)
	{
		uint32_t *rowPtr = &target->data[tY * target->width + x];
		for(int tX = x; tX < x + w; tX++, rowPtr++)
			if(x >= 0 && y >= 0 && x <= 1280 && y <= 720)
				*rowPtr = clr;
	}
}

void drawRectAlpha(tex *target, int x, int y, int w, int h, const clr c)
{
	for(int tY = y; tY < y + h; tY++)
	{
		uint32_t *rowPtr = &target->data[tY * target->width + x];
		for(int tX = x; tX < x + w; tX++, rowPtr++)
			if(x >= 0 && y >= 0 && x <= 1280 && y <= 720)
				*rowPtr = blend(c, clrCreateU32(*rowPtr));
	}
}

tex *texCreate(int w, int h)
{
	tex *ret = malloc(sizeof(tex));

	ret->width = w;
	ret->height = h;

	ret->data = (uint32_t *)malloc(w * h * sizeof(uint32_t));
	memset(ret->data, 0, w * h * sizeof(uint32_t));
	ret->size = ret->width * ret->height;

	return ret;
}

tex *texLoadPNGFile(const char *path)
{
	FILE *pngIn = fopen(path, "rb");
	if(pngIn != NULL)
	{
		png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if(png == 0)
			return NULL;

		png_infop pngInfo = png_create_info_struct(png);
		if(pngInfo == 0)
			return NULL;

		int jmp = setjmp(png_jmpbuf(png));
		if(jmp)
			return NULL;

		png_init_io(png, pngIn);
		png_read_info(png, pngInfo);

		if(png_get_color_type(png, pngInfo) != PNG_COLOR_TYPE_RGBA)
		{
			png_destroy_read_struct(&png, &pngInfo, NULL);
			return NULL;
		}

		tex *ret = malloc(sizeof(tex));
		ret->width = png_get_image_width(png, pngInfo);
		ret->height = png_get_image_height(png, pngInfo);

		ret->data = (uint32_t *)malloc((ret->width * ret->height) * sizeof(uint32_t));
		ret->size = ret->width * ret->height;

		png_bytep *rows = malloc(sizeof(png_bytep) * ret->height);
		for(int i = 0; i < ret->height; i++)
			rows[i] = malloc(png_get_rowbytes(png, pngInfo));

		png_read_image(png, rows);

		uint32_t *dataPtr = &ret->data[0];
		for(int y = 0; y < ret->height; y++)
		{
			uint32_t *rowPtr = (uint32_t *)rows[y];
			for(int x = 0; x < ret->width; x++)
				*dataPtr++ = *rowPtr++;
		}

		for(int i = 0; i < ret->height; i++)
			free(rows[i]);

		free(rows);

		png_destroy_read_struct(&png, &pngInfo, NULL);
		fclose(pngIn);

		return ret;
	}
	return NULL;
}

tex *texLoadJPEGFile(const char *path)
{
	FILE *jpegIn = fopen(path, "rb");
	if(jpegIn != NULL)
	{
		struct jpeg_decompress_struct jpegInfo;
		struct jpeg_error_mgr error;

		jpegInfo.err = jpeg_std_error(&error);

		jpeg_create_decompress(&jpegInfo);
		jpeg_stdio_src(&jpegInfo, jpegIn);
		jpeg_read_header(&jpegInfo, true);

		if(jpegInfo.jpeg_color_space == JCS_YCbCr)
			jpegInfo.out_color_space = JCS_RGB;

		tex *ret = malloc(sizeof(tex));

		ret->width = jpegInfo.image_width;
		ret->height = jpegInfo.image_height;

		ret->data = (uint32_t *)malloc((ret->width * ret->height) * sizeof(uint32_t));
		ret->size = ret->width * ret->height;

		jpeg_start_decompress(&jpegInfo);

		JSAMPARRAY row = malloc(sizeof(JSAMPROW));
		row[0] = malloc(sizeof(JSAMPLE) * ret->width * 3);

		uint32_t *dataPtr = &ret->data[0];
		for(int y = 0; y < ret->height; y++)
		{
			jpeg_read_scanlines(&jpegInfo, row, 1);
			uint8_t *jpegPtr = row[0];
			for(int x = 0; x < ret->width; x++, jpegPtr += 3)
				*dataPtr++ = (0xFF << 24 | jpegPtr[2] << 16 | jpegPtr[1] << 8 | jpegPtr[0]);
		}

		jpeg_finish_decompress(&jpegInfo);
		jpeg_destroy_decompress(&jpegInfo);

		free(row[0]);
		free(row);

		fclose(jpegIn);

		return ret;
	}
	return NULL;
}

tex *texLoadJPEGMem(const uint8_t *jpegData, size_t jpegSize)
{
	struct jpeg_decompress_struct jpegInfo;
	struct jpeg_error_mgr error;

	jpegInfo.err = jpeg_std_error(&error);

	jpeg_create_decompress(&jpegInfo);
	jpeg_mem_src(&jpegInfo, jpegData, jpegSize);
	jpeg_read_header(&jpegInfo, true);

	if(jpegInfo.jpeg_color_space == JCS_YCbCr)
		jpegInfo.out_color_space = JCS_RGB;

	tex *ret = malloc(sizeof(tex));
	ret->width = jpegInfo.image_width;
	ret->height = jpegInfo.image_height;

	ret->data = (uint32_t *)malloc((ret->width * ret->height) * sizeof(uint32_t));
	ret->size = ret->width * ret->height;

	jpeg_start_decompress(&jpegInfo);

	JSAMPARRAY row = malloc(sizeof(JSAMPARRAY));
	row[0] = malloc(sizeof(JSAMPLE) * ret->width * 3);

	uint32_t *dataPtr = &ret->data[0];
	for(int y = 0; y < ret->height; y++)
	{
		jpeg_read_scanlines(&jpegInfo, row, 1);
		uint8_t *jpegPtr = row[0];
		for(int x = 0; x < ret->width; x++, jpegPtr += 3)
			*dataPtr++ = (0xFF << 24 | jpegPtr[2] << 16 | jpegPtr[1] << 8 | jpegPtr[0]);
	}

	jpeg_finish_decompress(&jpegInfo);
	jpeg_destroy_decompress(&jpegInfo);

	free(row[0]);
	free(row);

	return ret;
}

void texDestroy(tex *t)
{
	if(t->data != NULL)
		free(t->data);

	if(t != NULL)
		free(t);
}

void texClearColor(tex *in, const clr c)
{
	uint32_t *dataPtr = &in->data[0];
	uint32_t color = clrGetColor(c);
	for(int i = 0; i < in->size; i++)
		*dataPtr++ = color;
}

void texDraw(const tex *t, tex *target, int x, int y)
{
	if(t != NULL)
	{
		uint32_t *dataPtr = &t->data[0];
		for(int tY = y; tY < y + t->height; tY++)
		{
			uint32_t *rowPtr = &target->data[tY * target->width + x];
			for(int tX = x; tX < x + t->width; tX++, rowPtr++)
			{
				if(x >= 0 && y >= 0 && x <= 1280 && y <= 720)
				{
					clr dataClr = clrCreateU32(*dataPtr++);
					clr fbClr = clrCreateU32(*rowPtr);

					*rowPtr = blend(dataClr, fbClr);
				}
			}
		}
	}
}

void texDrawNoAlpha(const tex *t, tex *target, int x, int y)
{
	if(t != NULL)
	{
		uint32_t *dataPtr = &t->data[0];
		for(int tY = y; tY < y + t->height; tY++)
		{
			uint32_t *rowPtr = &target->data[tY * target->width + x];
			for(int tX = x; tX < x + t->width; tX++)
			{
				if(x >= 0 && y >= 0 && x <= 1280 && y <= 720)
				{
					*rowPtr++ = *dataPtr++;
				}
			}
		}
	}
}

void texDrawLimit(const tex *t, tex *target, int x, int y)
{
	if(t != NULL)
	{
		int dst = 0, tY = 0;
		uint32_t *dataPtr = &t->data[0];
		for(int ty = 0; ty < t->height; ty++, tY = y + ty)
		{
			if(tY < 0)
				dst = 0;
			else
				dst = tY * target->width + x;

			uint32_t *rowPtr = &target->data[dst];
			for(int x = 0; x < t->width; x++, rowPtr++)
			{
				clr dataClr = clrCreateU32(*dataPtr++);
				clr fbClr = clrCreateU32(*rowPtr);
				if (tY > 87 && tY < 647)
					*rowPtr = blend(dataClr, fbClr);
			}
		}
	}
}

void texDrawHorizontal(const tex *t, tex *target, int x, int y, int w)
{
	if(t != NULL)
	{
		for(int width = 0; width < w; width++) {
			int nX = x + width;
			uint32_t *dataPtr = &t->data[0];
			for(int tY = y; tY < y + t->height; tY++)
			{	
				uint32_t *rowPtr = &target->data[tY * target->width + nX];
				for(int tX = nX; tX < nX + t->width; tX++, rowPtr++)
				{
					if(nX >= 0 && y >= 0 && nX <= 1280 && y <= 720)
					{
						clr dataClr = clrCreateU32(*dataPtr++);
						clr fbClr = clrCreateU32(*rowPtr);

						*rowPtr = blend(dataClr, fbClr);
					}
				}
			}
		}
	}
}

void texDrawVertical(const tex *t, tex *target, int x, int y, int h)
{
	if(t != NULL)
	{
		for(int height = 0; height < h; height++) {
			int nY = y + height;
			uint32_t *dataPtr = &t->data[0];
			for(int tY = nY; tY < nY + t->height; tY++)
			{
				uint32_t *rowPtr = &target->data[tY * target->width + x];
				for(int tX = x; tX < x + t->width; tX++, rowPtr++)
				{
					if(x >= 0 && nY >= 0 && x <= 1280 && nY <= 720)
					{
						clr dataClr = clrCreateU32(*dataPtr++);
						clr fbClr = clrCreateU32(*rowPtr);

						*rowPtr = blend(dataClr, fbClr);
					}
				}
			}
		}
	}
}

void texDrawResize(const tex *source, tex *target, int x, int y, int destWidth, int destHeight)
{
	 int pos;
	 float xScale = (float)source->width / (float)destWidth;
	 float yScale = (float)source->height / (float)destHeight;
	 uint8_t r1, r2, r3, r4;
	 uint8_t g1, g2, g3, g4;
	 uint8_t b1, b2, b3, b4;
	 float fx, fy, fx1, fy1;
	 float w1, w2, w3, w4;
	 float sourceX, sourceY;
	 int tY, tX;

	 #define BPP 4

	 uint8_t (*srcPtr)[BPP] = (uint8_t (*)[4])source->data;
	 uint8_t (*tgtPtr)[BPP] = (uint8_t (*)[4])target->data;

	 for (tY = 0; tY < destHeight; tY++)
	 {
		 sourceY = tY * yScale;

		 for (tX = 0; tX < destWidth; tX++)
		 {
			if ((tY + y) > 87 && (tY + y) < 647)
			{
				uint32_t offset = (tY + y) * target->width + (tX + x);

				sourceX = tX * xScale;

				pos = (((int)sourceY + 0) * source->width + (int)sourceX + 0);

				r1 = srcPtr[pos][0];
				g1 = srcPtr[pos][1];
				b1 = srcPtr[pos][2];

				pos = (((int)sourceY + 0) * source->width + (int)sourceX + 1);

				r2 = srcPtr[pos][0];
				g2 = srcPtr[pos][1];
				b2 = srcPtr[pos][2];

				pos = (((int)sourceY + 1) * source->width + (int)sourceX + 0);

				r3 = srcPtr[pos][0];
				g3 = srcPtr[pos][1];
				b3 = srcPtr[pos][2];

				pos = (((int)sourceY + 1) * source->width + (int)sourceX + 1);

				r4 = srcPtr[pos][0];
				g4 = srcPtr[pos][1];
				b4 = srcPtr[pos][2];

				// determine weights
				fx = sourceX - (int)sourceX;
				fy = sourceY - (int)sourceY;
				fx1 = 1.0f - fx;
				fy1 = 1.0f - fy;

				w1 = (fx1*fy1);
				w2 = (fx*fy1);
				w3 = (fx1*fy);
				w4 = (fx*fy);

				tgtPtr[offset][0] = (r1 * w1 + r2 * w2 + r3 * w3 + r4 * w4);
				tgtPtr[offset][1] = (g1 * w1 + g2 * w2 + g3 * w3 + g4 * w4);
				tgtPtr[offset][2] = (b1 * w1 + b2 * w2 + b3 * w3 + b4 * w4);
			}
		}
	}
}

void texSwapColors(tex *t, const clr old, const clr newColor)
{
	uint32_t oldClr = clrGetColor(old), newClr = clrGetColor(newColor);

	uint32_t *dataPtr = &t->data[0];
	for(unsigned i = 0; i < t->size; i++, dataPtr++)
	{
		if((*dataPtr & 0x00FFFFFF) == (oldClr & 0x00FFFFFF))
			*dataPtr = ( newClr & 0x00FFFFFF ) | ( *dataPtr & 0xFF000000 );
	}

}

tex *texCreateFromPart(const tex *src, int x, int y, int w, int h)
{
	tex *ret = texCreate(w, h);

	uint32_t *retPtr = &ret->data[0];
	for(int tY = y; tY < y + h; tY++)
	{
		uint32_t *srcPtr = &src->data[tY * src->width + x];
		for(int tX = x; tX < x + w; tX++)
			*retPtr++ = *srcPtr++;
	}

	return ret;
}

font *fontLoadSharedFonts()
{
	font *ret = malloc(sizeof(font));
	if((ret->libRet = FT_Init_FreeType(&ret->lib)))
	{
		free(ret);
		return NULL;
	}

	for(int i = 0; i < 6; i++)
	{
		PlFontData plFont;
		if(R_FAILED(plGetSharedFontByType(&plFont, i)))
		{
			free(ret);
			return NULL;
		}

		if((ret->faceRet = FT_New_Memory_Face(ret->lib, plFont.address, plFont.size, 0, &ret->face[i])))
		{
			free(ret);
			return NULL;
		}
	}

	ret->external = false;
	ret->fntData = NULL;

	return ret;
}

font *fontLoadTTF(const char *path)
{
	font *ret = malloc(sizeof(font));
	if((ret->libRet = FT_Init_FreeType(&ret->lib)))
	{
		free(ret);
		return NULL;
	}

	FILE *ttf = fopen(path, "rb");
	fseek(ttf, 0, SEEK_END);
	size_t ttfSize = ftell(ttf);
	fseek(ttf, 0, SEEK_SET);

	ret->fntData = malloc(ttfSize);
	fread(ret->fntData, 1, ttfSize, ttf);
	fclose(ttf);

	if((ret->faceRet = FT_New_Memory_Face(ret->lib, ret->fntData, ttfSize, 0, &ret->face[0])))
	{
		free(ret->fntData);
		free(ret);
		return NULL;
	}

	ret->external = true;

	return ret;
}

void fontDestroy(font *f)
{
	if(f->external && f->faceRet == 0)
		FT_Done_Face(f->face[0]);
	else if(!f->external && f->faceRet == 0)
	{
		for(int i = 0; i < 6; i++)
			FT_Done_Face(f->face[i]);
	}
	if(f->libRet == 0)
		FT_Done_FreeType(f->lib);
	if(f->fntData != NULL)
		free(f->fntData);

	free(f);
}
