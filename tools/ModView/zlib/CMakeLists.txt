set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(zlib
	STATIC
	deflate.h
	infblock.h
	infcodes.h
	inffast.h
	inffixed.h
	inftrees.h
	infutil.h
	trees.h
	zconf.h
	zlib.h
	zutil.h
	adler32.c
	deflate.c
	infblock.c
	infcodes.c
	inffast.c
	inflate.c
	inftrees.c
	infutil.c
	trees.c
	zutil.c
	crc32.cpp)
