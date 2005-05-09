bin_PROGRAMS	= moth c4

if WITH_GLUT
  bin_PROGRAMS	+= gmoth gc4
endif

COMMON		= common.c common.h

moth_SOURCES 	= $(COMMON) moth.c libmoth.h libmoth.c
gmoth_SOURCES	= $(COMMON) gmoth.c libmoth.h libmoth.c

c4_SOURCES 	= $(COMMON) connect4.c libc4.h libc4.c
gc4_SOURCES	= $(COMMON) gconnect4.c libc4.h libc4.c

