ARCH = x86
include Rules.mk

LOCAL_LIB_PATH = libs
INCDIR = Include
OUT_PARENT_DIR = bin

####################################################
####### Target Setting 
####################################################
LOCAL_EXECUTABLE_NAME := ColorDepth
TARGET_EXECUTABLE_DIR := $(OUT_PARENT_DIR)
TARGET_EXECUTABLE_EXE := $(TARGET_EXECUTABLE_DIR)/$(LOCAL_EXECUTABLE_NAME)
LOCAL_SRC_FILES = main.cpp Common/BerxelCommonFunc.cpp Common/BerxelImageRender.cpp
LOCAL_IN_FILES = Common/

LOCAL_SHARED_LIBRARIES = -L$(LOCAL_LIB_PATH) -Wl,-rpath, -lBerxelHawk -lglut -lGL
LOCAL_INCLUDE_FILES = -I$(INCDIR) -I$(LOCAL_IN_FILES)
CXXFLAGS += -I. -I Include $(LOCAL_INCLUDE_FILES)

all: $(TARGET_EXECUTABLE_DIR) $(TARGET_EXECUTABLE_EXE) 

$(TARGET_EXECUTABLE_EXE):$(LOCAL_SRC_FILES)
	$(CPP) $(CFLAGS) $(CXXFLAGS) $(LOCAL_SRC_FILES) $(LDFLAGS) $(LOCAL_SHARED_LIBRARIES) $(LDLIBS) -o $@ 

$(TARGET_EXECUTABLE_DIR):
	$(MKDIR) -p $(TARGET_EXECUTABLE_DIR)

clean:
	$(RM) $(TARGET_EXECUTABLE_EXE)

.PHONY: all clean
