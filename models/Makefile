#############################################################################
# Makefile to build 3D models
# Each model should be exist in its own directory
#############################################################################

include $(JAGSDK)/tools/build/jagdefs.mk

RM = rm -f
AR = ar 
ARFLAGS = crs

#
# Called from the Makefile in the main project directory
# with parameters:
#	OBJ = name of object file to build
#	DIR = name of the model directory to process
#
OBJ = thingdata.a
DIR = ship1

#
# The object will end up in the parent directory
#
OBJPATH = $(abspath ../$(OBJ))

#
# Rules for transforming models and textures
#
.SUFFIXES: .o .3ds
.SUFFIXES: .cry .tga

.3ds.o:
	3dsconv -o $*.s $<
	$(ASM) $(ASMFLAGS) $*.s
	$(RM) $*.s

.tga.cry:
	tga2cry -binary $<

#
# Models can some in any of these format
#
MODELS_3DS = $(wildcard $(DIR)/*.3ds)
MODELS_S = $(wildcard $(DIR)/*.s)
MODELS_C = $(wildcard $(DIR)/*.c)

#
# Models will be compiled to .o (via .s in the case of .3ds)
# This assumes that all files are uniquely named
#
MODELS = $(patsubst %.3ds,%.o,$(MODELS_3DS)) $(patsubst %.s,%.o,$(MODELS_S)) $(patsubst %.c,%.o,$(MODELS_C))

#
# .tga files which we will convert to .cry
#
TGAS = $(wildcard $(DIR)/*.tga)
CRYS = $(patsubst %.tga,%.cry,$(TGAS))

#
# The textures will be assembled into a single .o file
# Each block is:
#		.globl _<filename>
# 	_<filename>:
#		.incbin '<filepath>'
#
INCBIN = \t.globl\t_$F\n_$F:\n\t.incbin '$(DIR)/$F.cry'\n

textures.s: $(CRYS)
	echo "\t.data\n$(foreach F,$(patsubst $(DIR)/%.cry,%,$(CRYS)),$(INCBIN))" > textures.s

textures.o: textures.s

#
# The model and texture object files are packed in an archive
# 	built in the main project directory
#
$(OBJ):	
	$(AR) $(ARFLAGS) $(OBJPATH) $(MODELS) textures.o

all: $(MODELS) $(CRYS) textures.o $(OBJ)
	$(RM) $(MODELS)
	$(RM) $(CRYS)
	$(RM) textures.s 
	$(RM) textures.o

include $(JAGSDK)/tools/build/jagrules.mk
