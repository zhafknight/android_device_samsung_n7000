# Release name
PRODUCT_RELEASE_NAME := n7000

# Inherit from the common Open Source product configuration
$(call inherit-product, $(SRC_TARGET_DIR)/product/aosp_base_telephony.mk)

# Inherit from our custom product configuration
$(call inherit-product, vendor/twrp/config/common.mk)

# Time Zone data for Recovery
PRODUCT_COPY_FILES += \
    system/timezone/output_data/iana/tzdata:recovery/root/system/usr/share/zoneinfo/tzdata

## Device identifier. This must come after all inclusions
PRODUCT_DEVICE := n7000
PRODUCT_NAME := omni_n7000
PRODUCT_BRAND := samsung
PRODUCT_MANUFACTURER := samsung
TARGET_SCREEN_WIDTH := 800
TARGET_SCREEN_HEIGHT := 1280