PROJECT = tapir
MAJVER = 0.3
MINVER = 0
LIBS = ${PROJECT}
PROGS = tapir

include make/conf
include make/fontconfig.mk
include make/gl.mk
include make/libconfig.mk
include make/libdl.mk
include make/libmath.mk
include make/librt.mk
include make/libruby.mk
include make/pthread.mk
include make/sdl2
include make/sdl2_image
include make/sdl2_mixer
include make/sdl2_ttf
include make/zlib.mk

#-lcrypt $(SDL_LIBS) $(GL_LIBS) $(FONTCONFIG_LIBS) $(LIBCONFIG_LIBS)
CFLAGS = `pkg-config --cflags sdl2 SDL2_image SDL2_mixer SDL2_ttf fontconfig gl libconfig`
CFLAGS += -D_VERSION_=\"${VERSION}\" -O2 -Wall -Wextra -Wno-unused-parameter -g -o
DOCS = 
INFOS =
INCLUDES = 
INCLUDES += 

MANS = 
SRC = Audio.c Bitmap.c BitmapArray.c  Color.c Font.c Graphics.c Input.c Plane.c
SRC += RGSSError.c RGSSReset.c Rect.c Sprite.c Table.c Tilemap.c Tone.c
# archive.c
SRC += Viewport.c Win32APIFake.c Window.c font_lookup.c gl_misc.c
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
