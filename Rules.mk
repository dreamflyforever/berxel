
PRODUCT ?= RELEASE
WERROR  ?= 1
GCOV 	?= 0
LINUX_VER ?= x64
REDIST ?= BUILDPACKAGE

# now ok to proceed
SHELL   	  = /bin/sh
MAJOR_VERSION = 0
MINOR_VERSION = 9
PATCH_LEVEL   = 10
BUILD_NUMBER  = $(shell date '+%Y%m%d')


PROJECT_ROOT=$(realpath $(RELPATH_TO_ROOT))

MKDIR	= mkdir
CHMOD   = chmod
COPY    = cp -f
RM      = rm -r
CD      = cd

#########################################################################
# Tool command 
#########################################################################
CC      = $(COMPILE_PREFIX)gcc
CPP     = $(COMPILE_PREFIX)g++
LD      = $(COMPILE_PREFIX)ld
AS      = $(COMPILE_PREFIX)as
AR      = $(COMPILE_PREFIX)ar
OBJCOPY = $(COMPILE_PREFIX)objcopy
OBJDUMP = $(COMPILE_PREFIX)objdump
STRIP   = $(COMPILE_PREFIX)strip
RANLIB  = $(COMPILE_PREFIX)ranlib

#########################################################################
# GCC Version 
#########################################################################
GCC_MAJOR_VER := $(shell export PATH=$(PATH); $(CC) -dumpversion | sed -e "s/\([0-9][0-9]*\)\.\([0-9][0-9]*\)\(\.\)*\([0-9][0-9]*\)*/\1/")
GCC_MINOR_VER := $(shell export PATH=$(PATH); $(CC) -dumpversion | sed -e "s/\([0-9][0-9]*\)\.\([0-9][0-9]*\)\(\.\)*\([0-9][0-9]*\)*/\2/")
GCC_PATCH_VER := $(shell export PATH=$(PATH); $(CC) -dumpversion | sed -e "s/\([0-9][0-9]*\)\.\([0-9][0-9]*\)\(\.\)*\([0-9][0-9]*\)*/\4/")

INCLUDES = -I.

#########################################################################
# Target Path 
#########################################################################
DIST_DIR     = $(RELPATH_TO_ROOT)/x86out/

DISTDIR_BIN  = $(ROOTFS_SYS)/bin
DISTDIR_LIB  = $(ROOTFS_SYS)/lib

######################################
# Directory of output files 
######################################


#########################################################################
# Debug Setting 
#########################################################################
ifeq ($(PRODUCT), DEBUG)

DEFINE      += -D_DEBUG -DDEBUG
CFG_CFLAGS  = -O0 -ggdb -funwind-tables -fno-omit-frame-pointer
CFG_LDFLAGS = -ggdb
OUT_PATHNAME = Debug

######################################
# Debug Target Path 
######################################

ifeq ($(REDIST), BUILDEXAMPLE)
OUTDIR         = $(RELPATH_TO_ROOT)/../out/Debug/x86
INCDIR         = ../../include
LOCAL_LIB_PATH = $(OUTDIR)
OUT_PARENT_DIR = $(RELPATH_TO_ROOT)/out/Debug
else
OUTDIR         = ../../bin
INCDIR         = ../../../include
LOCAL_LIB_PATH = ../../../libs
OUT_PARENT_DIR = ../../bin
endif

######################################
# GCC Option 
######################################
CFG_CFLAGS += -fstack-protector -ftrapv -D_FORTIFY_SOURCE=2
ifeq ($(WERROR), 1)
CFG_CFLAGS += -Werror
endif

# for error: Warning _FORTIFY_SOURCE requires compiling with optimization (-O) [-Werror=cpp]
CFG_CFLAGS += -Wno-cpp


######################################
# GCOV Option 
######################################
ifeq ($(GCOV), 1)
CFG_CFLAGS   += -fprofile-arcs -ftest-coverage
CFG_CXXFLAGS += -fprofile-arcs -ftest-coverage
endif

endif

#########################################################################
# Release Setting 
#########################################################################
ifeq ($(PRODUCT), RELEASE)
DEFINE      += -UDEBUG -U_DEBUG
CFG_CFLAGS  = -O2

CFG_LDFLAGS = -Wl,--strip-all 

OUT_PATHNAME = Release

######################################
# Release Target Path 
######################################

ifeq ($(REDIST), BUILDEXAMPLE)
OUTDIR      = $(RELPATH_TO_ROOT)/../out/Release/x86
INCDIR         = ../../include
LOCAL_LIB_PATH = $(OUTDIR)
OUT_PARENT_DIR = $(RELPATH_TO_ROOT)/out/Release
else
OUTDIR         = ../../bin
INCDIR         = ../../../include
LOCAL_LIB_PATH = ../../../libs
OUT_PARENT_DIR = ../../bin
endif

endif

STATIC_LIB = $(OUTDIR)/static_libs
CFLAGS = $(INCLUDE) $(DEFINE) $(CFG_CFLAGS)

# compiler warnings
CFLAGS += -Wall -Wno-unknown-pragmas

# specify compiler version
CFLAGS += -DGCC_MAJOR_VER=$(GCC_MAJOR_VER) -DGCC_MINOR_VER=$(GCC_MINOR_VER)

# specify current system version
CFLAGS += -DMAJOR_VER=$(MAJOR_VER) -DMINOR_VER=$(MINOR_VER) -DPATCH_LEVEL=$(PATCH_LEVEL) -DBUILD_NUMBER=$(BUILD_NUMBER) 
CXXFLAGS += -fno-exceptions -fcheck-new -pthread

###################################
# All flags are gathered 
###################################
CXXFLAGS += $(CFG_CXXFLAGS) $(INCLUDES)
LDFLAGS += $(CFG_LDFLAGS) -lpthread -Wl,-rpath,$(realpath $(LOCAL_LIB_PATH))

ifeq ($(LINUX_VER), x32)
CFLAGS   += -m32
CXXFLAGS += -m32
LDFLAGS  += -m32
endif


vpath %.o $(OUTDIR)
vpath %.cpp .
vpath %.c .

# set dependency variable, use -include $(INCLUDE_DEPENDENCIES) to include dependence check.
INCLUDE_DEPENDENCIES = $(OBJS:.o=.P)

