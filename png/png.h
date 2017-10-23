
#ifndef WTS_CORE_PNG_H_
#define WTS_CORE_PNG_H_

#include <stdint.h>

namespace wts
{
	static const uint8_t COLOR_TYPE_GRAY=0;
	static const uint8_t COLOR_TYPE_RGB=2;
	static const uint8_t COLOR_TYPE_INDEX=3;
	static const uint8_t COLOR_TYPE_GRAY_ALPHA=4;
	static const uint8_t COLOR_TYPE_RGB_ALPHA=6;

	static const uint8_t COMPRESS_TYPE_LZ=0;

	static const uint8_t FILTER_TYPE_NONE=0;

	static const uint8_t INTERLACE_TYPE_NONE=0;
	static const uint8_t INTERLACE_TYPE_ADAM7=1;

	static const uint8_t LINEFILTER_NONE=0;
	static const uint8_t LINEFILTER_SUB=1;
	static const uint8_t LINEFILTER_UP=2;
	static const uint8_t LINEFILTER_AVG=3;
	static const uint8_t LINEFILTER_PAETH=4;

	struct ImageHeader
	{
		uint32_t width;
		uint32_t height;
		uint8_t color_depth;
		uint8_t color_type;
		uint8_t compress_type;
		uint8_t filter_type;
		uint8_t interlace_type;
        uint8_t pltd[256][3];
        uint8_t trns[256];
	};

	struct Png
	{
		ImageHeader image_header;
		uint8_t *image_data;
		uint32_t GetGrayPixel(int x,int y);
	};

	struct Raw
	{
		uint8_t *data;
		uint32_t size;
	};

	void SetAllocateFunction(void*(*)(uint32_t),void(*)(void*));

	bool ReadFromRaw(Png *out,const Raw *in);

	void FreePng(Png *png);

	bool WriteFromPng(Raw *out,const Png *in);

	bool WriteFromR8G8B8(Raw *out,uint32_t width,uint32_t height,const uint8_t *rgb);
	bool WriteFromR8G8B8A8(Raw *out,uint32_t width,uint32_t height,const uint8_t *rgba);
	bool WriteFromB8G8R8A8(Raw *out,uint32_t width,uint32_t height,const uint8_t *rgba);

	void FreeRaw(Raw *raw);
}

#endif
