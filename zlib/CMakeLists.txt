cmake_minimum_required(VERSION 2.6)

project(zlib)

add_library(zlib
	source/adler32.c
	source/compress.c
	source/crc32.c
	source/crc32.h
	source/deflate.c
	source/deflate.h
	source/infback.c
	source/inffast.c
	source/inffast.h
	source/inffixed.h
	source/inflate.c
	source/inflate.h
	source/inftrees.c
	source/inftrees.h
	source/trees.c
	source/trees.h
	source/uncompr.c
	source/zutil.c
	source/zutil.h
	include/zlib.h
	include/zconf.h
)

