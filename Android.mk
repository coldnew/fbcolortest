LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := fbcolortest
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SRC_FILES:= fbcolortest.c
LOCAL_CFLAGS += -std=c99
LOCAL_STATIC_LIBRARIES := libc

include $(BUILD_EXECUTABLE)
