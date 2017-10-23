#include "png.h"
#include <zlib.h>

#include <stdlib.h>

namespace wts
{
	static const uint32_t crc_table[256]={
        0x00000000,0x77073096,0xee0e612c,0x990951ba
        ,0x076dc419,0x706af48f,0xe963a535,0x9e6495a3
        ,0x0edb8832,0x79dcb8a4,0xe0d5e91e,0x97d2d988
        ,0x09b64c2b,0x7eb17cbd,0xe7b82d07,0x90bf1d91
        ,0x1db71064,0x6ab020f2,0xf3b97148,0x84be41de
        ,0x1adad47d,0x6ddde4eb,0xf4d4b551,0x83d385c7
        ,0x136c9856,0x646ba8c0,0xfd62f97a,0x8a65c9ec
        ,0x14015c4f,0x63066cd9,0xfa0f3d63,0x8d080df5
        ,0x3b6e20c8,0x4c69105e,0xd56041e4,0xa2677172
        ,0x3c03e4d1,0x4b04d447,0xd20d85fd,0xa50ab56b
        ,0x35b5a8fa,0x42b2986c,0xdbbbc9d6,0xacbcf940
        ,0x32d86ce3,0x45df5c75,0xdcd60dcf,0xabd13d59
        ,0x26d930ac,0x51de003a,0xc8d75180,0xbfd06116
        ,0x21b4f4b5,0x56b3c423,0xcfba9599,0xb8bda50f
        ,0x2802b89e,0x5f058808,0xc60cd9b2,0xb10be924
        ,0x2f6f7c87,0x58684c11,0xc1611dab,0xb6662d3d
        ,0x76dc4190,0x01db7106,0x98d220bc,0xefd5102a
        ,0x71b18589,0x06b6b51f,0x9fbfe4a5,0xe8b8d433
        ,0x7807c9a2,0x0f00f934,0x9609a88e,0xe10e9818
        ,0x7f6a0dbb,0x086d3d2d,0x91646c97,0xe6635c01
        ,0x6b6b51f4,0x1c6c6162,0x856530d8,0xf262004e
        ,0x6c0695ed,0x1b01a57b,0x8208f4c1,0xf50fc457
        ,0x65b0d9c6,0x12b7e950,0x8bbeb8ea,0xfcb9887c
        ,0x62dd1ddf,0x15da2d49,0x8cd37cf3,0xfbd44c65
        ,0x4db26158,0x3ab551ce,0xa3bc0074,0xd4bb30e2
        ,0x4adfa541,0x3dd895d7,0xa4d1c46d,0xd3d6f4fb
        ,0x4369e96a,0x346ed9fc,0xad678846,0xda60b8d0
        ,0x44042d73,0x33031de5,0xaa0a4c5f,0xdd0d7cc9
        ,0x5005713c,0x270241aa,0xbe0b1010,0xc90c2086
        ,0x5768b525,0x206f85b3,0xb966d409,0xce61e49f
        ,0x5edef90e,0x29d9c998,0xb0d09822,0xc7d7a8b4
        ,0x59b33d17,0x2eb40d81,0xb7bd5c3b,0xc0ba6cad
        ,0xedb88320,0x9abfb3b6,0x03b6e20c,0x74b1d29a
        ,0xead54739,0x9dd277af,0x04db2615,0x73dc1683
        ,0xe3630b12,0x94643b84,0x0d6d6a3e,0x7a6a5aa8
        ,0xe40ecf0b,0x9309ff9d,0x0a00ae27,0x7d079eb1
        ,0xf00f9344,0x8708a3d2,0x1e01f268,0x6906c2fe
        ,0xf762575d,0x806567cb,0x196c3671,0x6e6b06e7
        ,0xfed41b76,0x89d32be0,0x10da7a5a,0x67dd4acc
        ,0xf9b9df6f,0x8ebeeff9,0x17b7be43,0x60b08ed5
        ,0xd6d6a3e8,0xa1d1937e,0x38d8c2c4,0x4fdff252
        ,0xd1bb67f1,0xa6bc5767,0x3fb506dd,0x48b2364b
        ,0xd80d2bda,0xaf0a1b4c,0x36034af6,0x41047a60
        ,0xdf60efc3,0xa867df55,0x316e8eef,0x4669be79
        ,0xcb61b38c,0xbc66831a,0x256fd2a0,0x5268e236
        ,0xcc0c7795,0xbb0b4703,0x220216b9,0x5505262f
        ,0xc5ba3bbe,0xb2bd0b28,0x2bb45a92,0x5cb36a04
        ,0xc2d7ffa7,0xb5d0cf31,0x2cd99e8b,0x5bdeae1d
        ,0x9b64c2b0,0xec63f226,0x756aa39c,0x026d930a
        ,0x9c0906a9,0xeb0e363f,0x72076785,0x05005713
        ,0x95bf4a82,0xe2b87a14,0x7bb12bae,0x0cb61b38
        ,0x92d28e9b,0xe5d5be0d,0x7cdcefb7,0x0bdbdf21
        ,0x86d3d2d4,0xf1d4e242,0x68ddb3f8,0x1fda836e
        ,0x81be16cd,0xf6b9265b,0x6fb077e1,0x18b74777
        ,0x88085ae6,0xff0f6a70,0x66063bca,0x11010b5c
        ,0x8f659eff,0xf862ae69,0x616bffd3,0x166ccf45
        ,0xa00ae278,0xd70dd2ee,0x4e048354,0x3903b3c2
        ,0xa7672661,0xd06016f7,0x4969474d,0x3e6e77db
        ,0xaed16a4a,0xd9d65adc,0x40df0b66,0x37d83bf0
        ,0xa9bcae53,0xdebb9ec5,0x47b2cf7f,0x30b5ffe9
        ,0xbdbdf21c,0xcabac28a,0x53b39330,0x24b4a3a6
        ,0xbad03605,0xcdd70693,0x54de5729,0x23d967bf
        ,0xb3667a2e,0xc4614ab8,0x5d681b02,0x2a6f2b94
        ,0xb40bbe37,0xc30c8ea1,0x5a05df1b,0x2d02ef8d
    };

	/* Update a running CRC with the bytes buf[0..len-1]--the CRC
	should be initialized to all 1's, and the transmitted value
	is the 1's complement of the final running CRC (see the
	crc() routine below)). */

	unsigned long update_crc(unsigned long crc, unsigned char *buf,
		int len)
	{
		unsigned long c = crc;
		int n;
		for (n = 0; n < len; n++) {
			c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
		}
		return c;
	}

	/* Return the CRC of the bytes buf[0..len-1]. */
	unsigned long calc_crc(unsigned char *buf, int len)
	{
		return update_crc(0xffffffffL, buf, len) ^ 0xffffffffL;
	}


	const uint8_t signature[8]={0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a};

	void *(*alloc)(uint32_t)=::malloc;
	void (*free)(void*)=::free;

	void SetAllocateFunction(void*(*a)(uint32_t),void(*f)(void*))
	{
		alloc=a;
		free=f;
	}

	bool memmem(const void *a,const void *b,uint32_t size)
	{
		while(size--)
			if(((uint8_t*)a)[size]!=((uint8_t*)b)[size])
				return false;
		return true;
	}

	void memcpy(void *dst,const void *src,uint32_t size)
	{
		while(size--)
			((uint8_t*)dst)[size]=((uint8_t*)src)[size];
	}

	uint32_t read32(uint8_t *p)
	{
		return ((uint32_t)p[0]<<24)|
			((uint32_t)p[1]<<16)|
			((uint32_t)p[2]<<8)|p[3];
	}

	void write32(uint8_t *p,uint32_t i)
	{
		p[0]=(uint8_t)(i>>24);
		p[1]=(uint8_t)(i>>16);
		p[2]=(uint8_t)(i>>8);
		p[3]=(uint8_t)(i);
	}

	uint16_t read16(uint8_t *p)
	{
		return ((uint16_t)p[0]<<8)|p[1];
	}

	void write16(uint8_t *p,uint16_t i)
	{
		p[0]=(uint8_t)(i>>8);
		p[1]=(uint8_t)(i);
	}

	inline uint16_t swap(uint16_t in)
	{
		return (in<<8)|(in>>8);
	}

	inline void swap(uint8_t *dst,const uint8_t *src,int size)
	{
		for(int i=0;i<size;i++)
		{
			dst[i]=src[size-i-1];
		}
	}

	bool ReadFromRaw(Png *png,const Raw *in)
	{
		uint8_t *p=in->data,*end=in->data+in->size;
		if(p+8<end&&!memmem(signature,p,8))
			return false;
		p+=8;
		::z_stream zs;
		zs.zalloc=Z_NULL;
		zs.zfree=Z_NULL;
		zs.opaque=Z_NULL;
		inflateInit(&zs);
		uint8_t *temp=0;
		uint32_t temp_size=0;
		int stride=0;
        for(int i=0;i<256;i++)
            png->image_header.trns[i]=255;
		for(;;)
		{
			if(p+8>end)
				return false;
			uint32_t content_length=read32(p);
			uint8_t *next_chunk=p+content_length+12;
			uint32_t crc=read32(next_chunk-4);
			uint8_t *content=p+8;

			if(crc!=calc_crc(p+4,content_length+4))
				return false;


			if(memmem("PLTE",p+4,4))
			{
                memcpy(png->image_header.pltd,content,content_length);
            }
			else if(memmem("tRNS",p+4,4))
            {
                memcpy(png->image_header.trns,content,content_length);
            }
			else if(memmem("IHDR",p+4,4))
			{
				png->image_header.width=read32(p+8);
				png->image_header.height=read32(p+12);
				png->image_header.color_depth=p[16];
				png->image_header.color_type=p[17];
				png->image_header.compress_type=p[18];
				png->image_header.filter_type=p[19];
				png->image_header.interlace_type=p[20];
				switch(png->image_header.color_type)
				{
				case COLOR_TYPE_GRAY:
				case COLOR_TYPE_INDEX:
					stride=1;
					break;
				case COLOR_TYPE_GRAY_ALPHA:
					stride=2;
					break;
				case COLOR_TYPE_RGB:
					stride=3;
					break;
				case COLOR_TYPE_RGB_ALPHA:
					stride=4;
					break;
				default:
					return false;
				}

				uint32_t idat_size=png->image_header.width*png->image_header.height*stride*png->image_header.color_depth/8;
				temp_size=idat_size+png->image_header.width;
				png->image_data=(uint8_t*)alloc(idat_size);
				temp=(uint8_t*)alloc(temp_size);

				zs.next_out=temp;
				zs.avail_out=temp_size;
			}
			else if(memmem("IDAT",p+4,4))
			{
				zs.next_in=content;
				zs.avail_in=content_length;

				int ret=inflate(&zs,Z_NO_FLUSH);
				if(Z_OK!=ret&&Z_STREAM_END!=ret)
				{
					free(png->image_data);
					return false;
				}
			}
			else if(memmem("IEND",p+4,4))
			{
				inflateEnd(&zs);
				for(uint32_t y=0;y<png->image_header.height;y++)
				{
					uint32_t pitch=stride*png->image_header.color_depth*png->image_header.width/8;
					uint32_t bpp=png->image_header.color_depth/8*stride;
                    if(bpp<1)
                        bpp=1;
					uint8_t filter=temp[y*(pitch+1)];
					uint8_t *src=(uint8_t*)(temp+y*(pitch+1)+1);
					uint8_t *dst=(uint8_t*)(png->image_data+y*pitch);
					uint32_t len=png->image_header.color_depth*png->image_header.width/8;
					for(uint32_t x=0;x<len;x++)
					{
						swap(dst+x*bpp,src+x*bpp,bpp);
					}
					uint8_t *predst=(uint8_t*)(png->image_data+(y-1)*pitch);
					if(LINEFILTER_NONE==filter)
					{
					}
					else if(LINEFILTER_SUB==filter)
					{
						uint8_t *dstm=dst;
						dst+=bpp;
						for(uint32_t x=bpp;x<len*bpp;x++)
						{
							*dst+++=*dstm++;
						}
					}
					else if(LINEFILTER_UP==filter)
					{
						for(uint32_t x=0;x<len*bpp;x++)
						{
							*dst+++=*predst++;
						}
					}
					else if(LINEFILTER_AVG==filter)
					{
						uint8_t *dstm=dst;
						uint32_t x=0;
						for(;x<bpp;x++)
						{
							*dst+++=*predst++/2;
						}
						for(;x<len*bpp;x++)
						{
							*dst+++=(*dstm+++*predst++)/2;
						}
					}
					else if(LINEFILTER_PAETH==filter)
					{
						uint8_t *dstm=dst;
						uint8_t *predstm=predst;
						uint32_t x=0;
						for(;x<bpp;x++)
						{
							*dst+++=*predst++;
						}
						for(;x<len*bpp;x++)
						{
							int a, b, c, pa, pb, pc, p;

							a = *dstm++;
							b = *predst++;
							c = *predstm++;

							p = b - c;
							pc = a - c;

							pa = abs(p);
							pb = abs(pc);
							pc = abs(p + pc);

							p = (pa <= pb && pa <= pc) ? a : (pb <= pc) ? b : c;
							*dst+++=(uint8_t)p;
						}
					}
				}
				free(temp);
				return true;
			}
			p=next_chunk;
		}
	}

	void FreePng(Png *png)
	{
		free(png->image_data);
	}

	bool WriteFromPng(Raw *out,const Png *in)
	{
		int elements;
		switch(in->image_header.color_type)
		{
		case COLOR_TYPE_GRAY:
		case COLOR_TYPE_INDEX:
			elements=1;
			break;
		case COLOR_TYPE_GRAY_ALPHA:
			elements=2;
			break;
		case COLOR_TYPE_RGB:
			elements=3;
			break;
		case COLOR_TYPE_RGB_ALPHA:
			elements=4;
			break;
		default:
			return false;
		}

		uint32_t bw=elements*in->image_header.color_depth;

		uint8_t *data=(uint8_t*)alloc(in->image_header.height*in->image_header.width*bw/8+in->image_header.height);

		uint8_t *dst=data;
		uint8_t *src=in->image_data;

		for(uint32_t y=0;y<in->image_header.height;y++)
		{
			uint32_t delta=bw/8;
			if(delta<1)
				delta=1;

			*dst++=LINEFILTER_SUB;
			uint8_t *left=dst;
			uint32_t x=0;
			for(;x<delta;x++)
			{
				*dst++=*src++;
			}
			for(;x<in->image_header.width*bw/8;x++)
			{
				*dst++=*src-*(src-delta);
				src++;
			}
			if(in->image_header.color_depth==16)
			{
				for(uint32_t x=0;x<in->image_header.width*elements;x++)
				{
					uint8_t tmp=left[0];
					left[0]=left[1];
					left[1]=tmp;
					left+=2;
				}
			}
		}

		uint32_t reserv=8+25+12+in->image_header.width*in->image_header.height*bw/8+12+1024;
		uint8_t *temp=(uint8_t*)alloc(reserv);
		uint8_t *p=temp;
		
		memcpy(p,signature,8);
		p+=8;

		//IHDR
		write32(p,13);
		memcpy(p+4,"IHDR",4);
		write32(p+8,in->image_header.width);
		write32(p+12,in->image_header.height);
		p[16]=in->image_header.color_depth;
		p[17]=in->image_header.color_type;
		p[18]=in->image_header.compress_type;
		p[19]=in->image_header.filter_type;
		p[20]=in->image_header.interlace_type;
		write32(p+21,calc_crc(p+4,17));

		p+=25;

		//IDAT
		memcpy(p+4,"IDAT",4);
		uint32_t dst_len=in->image_header.width*in->image_header.height*bw/8;
		compress(p+8,(unsigned long*)&dst_len,data,in->image_header.width*in->image_header.height*bw/8+in->image_header.height);
		write32(p,dst_len);
		write32(p+dst_len+8,calc_crc(p+4,dst_len+4));

		p+=dst_len+12;

		//IEND
		write32(p,0);
		memcpy(p+4,"IEND",4);
		write32(p+8,calc_crc(p+4,4));

		p+=12;

		out->size=(uint32_t)(p-temp);
		out->data=(uint8_t*)alloc(out->size);
		memcpy(out->data,temp,out->size);

		free(data);
		free(temp);
		return true;
	}

	bool WriteFromR8G8B8(Raw *out,uint32_t width,uint32_t height,const uint8_t *rgb)
	{
		uint8_t *frgb=(uint8_t*)alloc(width*height*3+height);
		for(uint32_t y=0;y<height;y++)
		{
			frgb[y+y*width*3]=LINEFILTER_SUB;
			uint8_t r=0;
			uint8_t g=0;
			uint8_t b=0;
			for(uint32_t x=0;x<width;x++)
			{
				frgb[y+(y*width+x)*3+1]=rgb[(y*width+x)*3+2]-r;
				frgb[y+(y*width+x)*3+2]=rgb[(y*width+x)*3+1]-g;
				frgb[y+(y*width+x)*3+3]=rgb[(y*width+x)*3+0]-b;
				r=rgb[(y*width+x)*3+2];
				g=rgb[(y*width+x)*3+1];
				b=rgb[(y*width+x)*3+0];
			}
		}

		uint32_t reserv=8+25+12+width*height*3+12+1024;
		uint8_t *temp=(uint8_t*)alloc(reserv);
		uint8_t *p=temp;
		
		memcpy(p,signature,8);
		p+=8;

		//IHDR
		write32(p,13);
		memcpy(p+4,"IHDR",4);
		write32(p+8,width);
		write32(p+12,height);
		p[16]=8;
		p[17]=COLOR_TYPE_RGB;
		p[18]=COMPRESS_TYPE_LZ;
		p[19]=FILTER_TYPE_NONE;
		p[20]=INTERLACE_TYPE_NONE;
		write32(p+21,calc_crc(p+4,17));

		p+=25;

		//IDAT
		memcpy(p+4,"IDAT",4);
		uint32_t dst_len=width*height*3;
		compress(p+8,(unsigned long*)&dst_len,frgb,width*height*3+height);
		write32(p,dst_len);
		write32(p+dst_len+8,calc_crc(p+4,dst_len+4));

		p+=dst_len+12;

		//IEND
		write32(p,0);
		memcpy(p+4,"IEND",4);
		write32(p+8,calc_crc(p+4,4));

		p+=12;

		out->size=(uint32_t)(p-temp);
		out->data=(uint8_t*)alloc(out->size);
		memcpy(out->data,temp,out->size);

		free(frgb);
		free(temp);
		return true;
	}

	bool WriteFromR8G8B8A8(Raw *out,uint32_t width,uint32_t height,const uint8_t *rgba)
	{
		uint8_t *frgba=(uint8_t*)alloc(width*height*4+height);
		for(uint32_t y=0;y<height;y++)
		{
			frgba[y+y*width*4]=LINEFILTER_SUB;
			uint8_t r=0;
			uint8_t g=0;
			uint8_t b=0;
			uint8_t a=0;
			for(uint32_t x=0;x<width;x++)
			{
				frgba[y+(y*width+x)*4+1]=rgba[(y*width+x)*4+0]-r;
				frgba[y+(y*width+x)*4+2]=rgba[(y*width+x)*4+1]-g;
				frgba[y+(y*width+x)*4+3]=rgba[(y*width+x)*4+2]-b;
				frgba[y+(y*width+x)*4+4]=rgba[(y*width+x)*4+3]-a;
				r=rgba[(y*width+x)*4+0];
				g=rgba[(y*width+x)*4+1];
				b=rgba[(y*width+x)*4+2];
				a=rgba[(y*width+x)*4+3];
			}
		}

		uint32_t reserv=8+25+12+width*height*4+12+1024;
		uint8_t *temp=(uint8_t*)alloc(reserv);
		uint8_t *p=temp;
		
		memcpy(p,signature,8);
		p+=8;

		//IHDR
		write32(p,13);
		memcpy(p+4,"IHDR",4);
		write32(p+8,width);
		write32(p+12,height);
		p[16]=8;
		p[17]=COLOR_TYPE_RGB_ALPHA;
		p[18]=COMPRESS_TYPE_LZ;
		p[19]=FILTER_TYPE_NONE;
		p[20]=INTERLACE_TYPE_NONE;
		write32(p+21,calc_crc(p+4,17));

		p+=25;

		//IDAT
		memcpy(p+4,"IDAT",4);
		uint32_t dst_len=width*height*4;
		compress(p+8,(unsigned long*)&dst_len,frgba,width*height*4+height);
		write32(p,dst_len);
		write32(p+dst_len+8,calc_crc(p+4,dst_len+4));

		p+=dst_len+12;

		//IEND
		write32(p,0);
		memcpy(p+4,"IEND",4);
		write32(p+8,calc_crc(p+4,4));

		p+=12;

		out->size=(uint32_t)(p-temp);
		out->data=(uint8_t*)alloc(out->size);
		memcpy(out->data,temp,out->size);

		free(frgba);
		free(temp);
		return true;
	}

	bool WriteFromB8G8R8A8(Raw *out,uint32_t width,uint32_t height,const uint8_t *bgra)
	{
		uint8_t *frgba=(uint8_t*)alloc(width*height*4+height);
		for(uint32_t y=0;y<height;y++)
		{
			frgba[y+y*width*4]=LINEFILTER_SUB;
			uint8_t r=0;
			uint8_t g=0;
			uint8_t b=0;
			uint8_t a=0;
			for(uint32_t x=0;x<width;x++)
			{
				frgba[y+(y*width+x)*4+1]=bgra[(y*width+x)*4+2]-r;
				frgba[y+(y*width+x)*4+2]=bgra[(y*width+x)*4+1]-g;
				frgba[y+(y*width+x)*4+3]=bgra[(y*width+x)*4+0]-b;
				frgba[y+(y*width+x)*4+4]=bgra[(y*width+x)*4+3]-a;
				r=bgra[(y*width+x)*4+2];
				g=bgra[(y*width+x)*4+1];
				b=bgra[(y*width+x)*4+0];
				a=bgra[(y*width+x)*4+3];
			}
		}

		uint32_t reserv=8+25+12+width*height*4+12+1024;
		uint8_t *temp=(uint8_t*)alloc(reserv);
		uint8_t *p=temp;
		
		memcpy(p,signature,8);
		p+=8;

		//IHDR
		write32(p,13);
		memcpy(p+4,"IHDR",4);
		write32(p+8,width);
		write32(p+12,height);
		p[16]=8;
		p[17]=COLOR_TYPE_RGB_ALPHA;
		p[18]=COMPRESS_TYPE_LZ;
		p[19]=FILTER_TYPE_NONE;
		p[20]=INTERLACE_TYPE_NONE;
		write32(p+21,calc_crc(p+4,17));

		p+=25;

		//IDAT
		memcpy(p+4,"IDAT",4);
		uint32_t dst_len=width*height*4;
		compress(p+8,(unsigned long*)&dst_len,frgba,width*height*4+height);
		write32(p,dst_len);
		write32(p+dst_len+8,calc_crc(p+4,dst_len+4));

		p+=dst_len+12;

		//IEND
		write32(p,0);
		memcpy(p+4,"IEND",4);
		write32(p+8,calc_crc(p+4,4));

		p+=12;

		out->size=(uint32_t)(p-temp);
		out->data=(uint8_t*)alloc(out->size);
		memcpy(out->data,temp,out->size);

		free(frgba);
		free(temp);
		return true;
	}

	void FreeRaw(Raw *raw)
	{
		free(raw->data);
	}

	typedef uint32_t(*AccessFunc)(Png *png,uint32_t x,uint32_t y);

	uint32_t GetGrayPixel4(Png *png,uint32_t x,uint32_t y)
	{
        uint8_t b=png->image_data[(y*png->image_header.width+x)/2];
        if(x&1)
            return b&0x0f;
        else
            return b>>4;
	}

	uint32_t GetGrayPixel8(Png *png,uint32_t x,uint32_t y)
	{
		return png->image_data[y*png->image_header.width+x];
	}

	uint32_t GetGrayPixel16(Png *png,uint32_t x,uint32_t y)
	{
		return ((uint16_t*)(png->image_data))[y*png->image_header.width+x];
	}

	AccessFunc GetGrayPixel[17]={0,0,0,0,GetGrayPixel4,0,0,0,GetGrayPixel8,0,0,0,0,0,0,0,GetGrayPixel16};

	uint32_t Png::GetGrayPixel(int x, int y)
	{
		return wts::GetGrayPixel[image_header.color_depth](this,x,y);
	}
}

