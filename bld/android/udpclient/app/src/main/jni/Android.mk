# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_ARM_MODE	:= arm

LOCAL_MODULE    := udpclient

MY_SRC_PATH		:= ../../../../../../../src

LOCAL_SRC_FILES := \
	udpclient-jni.c \
    $(MY_SRC_PATH)/util/util.cpp \
    $(MY_SRC_PATH)/client/udpclient.cpp

LOCAL_C_INCLUDES := ./ $(MY_SRC_PATH)/util

LOCAL_LDLIBS += -ldl -llog 

LOCAL_CFLAGS := -g -DANDROID -DHAVE_NEON -DARM -DLINUX
LOCAL_CPPFLAGS += -frtti -fexceptions

include $(BUILD_SHARED_LIBRARY)
