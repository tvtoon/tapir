PROJECT = tapir
MAJVER = 0.1
MINVER = 0
LIBS = ${PROJECT}
PROGS = tapir-x

include make/conf
include make/libdl.mk
include make/libmath.mk
include make/librt.mk
include make/pthread.mk
include make/zlib.mk

#-lcrypt $(SDL_LIBS) $(GL_LIBS) $(FONTCONFIG_LIBS) $(LIBCONFIG_LIBS)
ELIBFLAGS += -lruby181 `pkg-config --libs --static sdl2 SDL2_image SDL2_mixer SDL2_ttf fontconfig gl libconfig`
CFLAGS = `pkg-config --cflags sdl2 SDL2_image SDL2_mixer SDL2_ttf fontconfig gl libconfig`
CFLAGS += -DRGSS=1 -D_VERSION_=\"${VERSION}\" -O2 -Wall -Wextra -Wno-unused-parameter -g -o
DOCS = 
INFOS =
INCLUDES = 
INCLUDES += 

MANS = 
SRC = Audio.c Bitmap.c BitmapArray.c  Color.c Font.c Graphics.c Input.c Plane.c
SRC += RGSSError.c RGSSReset.c Rect.c Sprite.c Table.c Tilemap.c Tone.c
SRC += Viewport.c Win32APIFake.c Window.c archive.c font_lookup.c gl_misc.c
SRC += ini.c main_rb.c misc.c openres.c sdl_misc.c surface_misc.c tapir_config.c

LIBSRC := ${SRC}
PROGSRC = main.c
#SRC += ${PROGSRC}

include make/exconf
include make/build

${PROGS}: main.o

dist-clean: clean

include make/pack
include make/rules
include make/thedep
