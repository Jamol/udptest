APP_MODULES := udpclient
APP_ABI := armeabi-v7a
APP_STL := c++_shared
APP_PLATFORM := android-9

ifeq ($(NDK_DEBUG), 1)
	APP_OPTIM := debug
else
	APP_OPTIM := release
endif

