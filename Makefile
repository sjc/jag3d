#############################################################################
# Makefile for new 3D package demo
#############################################################################

include $(JAGSDK)/tools/build/jagdefs.mk

#############################################################################
# location of various directories
#############################################################################

INC=include
LIB=lib
LIBJAG=libjag.a
FONTDIR=fonts
MODELDIR = models
MODELOBJEXT = data.a
JAGSDK_INC=$(JAGSDK)/jaguar/include
JAG3D=jag3d
JAG3D_LIB=$(JAG3D)/lib
JAG3D_REND=$(JAG3D)/renderer

#############################################################################
# names of various commands
#############################################################################

CFLAGS += -mshort -Wall -fno-builtin -nostdinc -I$(INC)
RM = rm -f
AR = ar
ARFLAGS = crs

#############################################################################
# C library objects
# Normally we would put these into an archive (a .a file) and let the linker
# sort out which ones we need; for demo purposes, though, here they all are
# explicitly
#############################################################################
CRT0= $(LIB)/jagrt.o

LIBOBJS = $(LIB)/alloc.o $(LIB)/clock.o $(LIB)/ctype.o $(LIB)/font.o \
	$(LIB)/joyinp.o $(LIB)/joypad.o $(LIB)/memset.o \
	$(LIB)/olist.o $(LIB)/sprintf.o $(LIB)/strcat.o $(LIB)/strcmp.o \
	$(LIB)/strcpy.o $(LIB)/strdup.o $(LIB)/strncmp.o $(LIB)/util.o \
	$(LIB)/video.o

$(LIBJAG): $(LIBOBJS)
	$(AR) $(ARFLAGS) $(LIBJAG) $(LIBOBJS)
	$(RM) $(LIBOBJS)

#
# These are the files unique to the current project
#
SRCOBJS = miscasm.o demo.o vectors.o storage.o

#
# Objects built by the 3D library code
#
JAG3D_OBJS = $(JAG3D)/jag3d.o $(JAG3D_LIB)/utils.o $(JAG3D_LIB)/sincos.o $(JAG3D_LIB)/gpulib.o $(JAG3D_LIB)/3d_sprite_template.o $(JAG3D_REND)/renderer.o 

#
# Building the renderer requires additional parameters to find its .inc files
#
JAG3D_REND_INCS = $(wildcard $(JAG3D_REND)/*.inc)

$(JAG3D_REND)/renderer.o: $(JAG3D_REND)/renderer.s $(JAG3D_REND_INCS)
	$(ASM) $(ASMFLAGS) -i$(JAG3D_REND) -i$(JAGSDK_INC) $<

#
# The model directories are those under $(DIR) which aren't files and aren't
# 	prefixed with _ (which we'll used to temporarily disable a model)
#
MODELDIRS = $(filter-out _% $(MODELDIR)/,$(patsubst $(MODELDIR)/%/,%,$(dir $(wildcard $(MODELDIR)/*/))))

#
# The model object file is named after its directory with $(MODELOBJEXT) suffix appended
#
MODELS = $(foreach D,$(MODELDIRS),$D$(MODELOBJEXT))

#
# We will call a Makefile in $(MODELDIR) to compile each model and its textures
# 	into an object file
#
$(MODELS): %$(MODELOBJEXT) : $(MODELDIR)/%
	@echo "Make model $@ from directory $*"
	$(MAKE) -C $(MODELDIR) OBJ=$@ DIR=$*

PROGS = demo.cof

FIXDATA = -ii $(FONTDIR)/clr6x12.jft _usefnt

OBJS = $(CRT0) $(SRCOBJS) $(MODELS) $(LIBJAG) $(JAG3D_OBJS)

demo.cof: $(OBJS)
	$(LINK) $(LINKFLAGS) -o demo.cof $(OBJS) $(FIXDATA)

include $(JAGSDK)/tools/build/jagrules.mk
