#
# Copyright (C) 2012 The Android Open-Source Project
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

# Include common makefile
$(call inherit-product, device/samsung/galaxys2-common/common.mk)

LOCAL_PATH := device/samsung/n7000

# Overlay
DEVICE_PACKAGE_OVERLAYS += $(LOCAL_PATH)/overlay

# Rootdir
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/rootdir/init.target.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.target.rc

# This device is xhdpi.
PRODUCT_AAPT_CONFIG := normal
PRODUCT_AAPT_PREF_CONFIG := xhdpi

PRODUCT_PROPERTY_OVERRIDES += \
    ro.sf.lcd_density=320

# Proprietary blobs dependency on libstlport
PRODUCT_PACKAGES += libstlport

# Sensors
PRODUCT_PACKAGES += \
    android.hardware.sensors@1.0-impl \
    sensors.exynos4

# GPS
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/gps/gps.xml:$(TARGET_COPY_OUT_VENDOR)/etc/gps.xml

PRODUCT_PACKAGES += \
    gps.exynos4 \
    libdmitry \
    libgps_symbols
    

# Keylayout
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/input/keylayout/gpio-keys.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/gpio-keys.kl \
    $(LOCAL_PATH)/configs/input/keylayout/max8997-muic.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/max8997-muic.kl \
    $(LOCAL_PATH)/configs/input/keylayout/melfas-touchkey.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/melfas-touchkey.kl \
    $(LOCAL_PATH)/configs/input/keylayout/samsung-keypad.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/samsung-keypad.kl \
    $(LOCAL_PATH)/configs/input/keylayout/sec_key.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/sec_key.kl \
    $(LOCAL_PATH)/configs/input/keylayout/sec_touchkey.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/sec_touchkey.kl

# Idc
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/input/idc/melfas_ts.idc:$(TARGET_COPY_OUT_VENDOR)/usr/idc/melfas_ts.idc \
    $(LOCAL_PATH)/configs/input/idc/mxt224_ts_input.idc:$(TARGET_COPY_OUT_VENDOR)/usr/idc/mxt224_ts_input.idc \
    $(LOCAL_PATH)/configs/input/idc/sec_e-pen.idc:$(TARGET_COPY_OUT_VENDOR)/usr/idc/sec_e-pen.idc \
    $(LOCAL_PATH)/configs/input/idc/sec_touchscreen.idc:$(TARGET_COPY_OUT_VENDOR)/usr/idc/sec_touchscreen.idc

$(call inherit-product-if-exists, vendor/samsung/n7000/n7000-vendor.mk)
