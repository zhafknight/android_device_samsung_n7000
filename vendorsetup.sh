#
#	This file is part of the OrangeFox Recovery Project
#
#	Samsung Galaxy Note GT-N7000
#	OrangeFox Recovery Project - fox_12.1
#

#set -o xtrace

FDEVICE="n7000"

fox_get_target_device() {
	if echo "$BASH_SOURCE" | grep -q "/$FDEVICE/"; then
		FOX_BUILD_DEVICE="$FDEVICE"
	elif set | grep BASH_ARGV | grep -w "\"$FDEVICE\""; then
		FOX_BUILD_DEVICE="$FDEVICE"
	elif echo "${BASH_SOURCE[0]}" | grep -q "/$FDEVICE/"; then
		FOX_BUILD_DEVICE="$FDEVICE"
	elif echo "$0" | grep -q "$FDEVICE"; then
		FOX_BUILD_DEVICE="$FDEVICE"
	fi
}

if [ -z "$1" ] && [ -z "$FOX_BUILD_DEVICE" ]; then
	fox_get_target_device
fi

if [ "$1" = "$FDEVICE" ] || [ "$FOX_BUILD_DEVICE" = "$FDEVICE" ]; then

	export TARGET_ARCH="arm"
	export TW_DEFAULT_LANGUAGE="en"
	export LC_ALL="C"
	export ALLOW_MISSING_DEPENDENCIES=true
	export OF_MAINTAINER="zhafknight"
	export FOX_VARIANT="Unofficial"
	export FOX_VANILLA_BUILD=1
	export FOX_RECOVERY_SYSTEM_PARTITION="/dev/block/platform/dw_mmc/by-name/FACTORYFS"
    export FOX_SETTINGS_ROOT_DIRECTORY=/data/recovery
    export FOX_MISCELLANEOUS_ROOT_DIRECTORY=/sdcard
    export OF_DEFAULT_TIMEZONE="MYT-8"
	export OF_ALLOW_DISABLE_NAVBAR=0
    export FOX_DELETE_AROMAFM=1
    export FOX_ENABLE_APP_MANAGER=1
	export FOX_USE_BASH_SHELL=1
	export FOX_ASH_IS_BASH=1
	export FOX_USE_BUSYBOX_BINARY=1
	export FOX_USE_TAR_BINARY=1
	export FOX_USE_XZ_UTILS=1
	export FOX_USE_LZ4_BINARY=1
	export FOX_USE_ZSTD_BINARY=1
	export FOX_USE_DATE_BINARY=1

else

	if [ -z "$FOX_BUILD_DEVICE" ] && [ -z "$BASH_SOURCE" ]; then
		echo "I: Not processing OrangeFox settings for $FDEVICE"
	fi

fi