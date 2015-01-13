LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

include $(OPENCV_ANDROID_DIR)/sdk/native/jni/OpenCV.mk

LOCAL_MODULE    := dirac
LOCAL_SRC_FILES := dirac.cpp
LOCAL_LDLIBS +=  -llog -ldl

include $(BUILD_SHARED_LIBRARY)
