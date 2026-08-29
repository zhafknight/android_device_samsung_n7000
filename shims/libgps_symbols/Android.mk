LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := glgps.cpp
LOCAL_SHARED_LIBRARIES := libssl
LOCAL_MODULE := libgps_symbols
LOCAL_VENDOR_MODULE := true

include $(BUILD_SHARED_LIBRARY)
