/*
 * Copyright (C) 2012 The Android Open Source Project
 * Copyright (C) 2012 The CyanogenMod Project <http://www.cyanogenmod.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _BDROID_BUILDCFG_H
#define _BDROID_BUILDCFG_H

#define BTM_DEF_LOCAL_NAME   "GT-N7000"

/* Defined if the kernel does not have support for CLOCK_BOOTTIME_ALARM */
#define KERNEL_MISSING_CLOCK_BOOTTIME_ALARM TRUE

#define BTA_DISABLE_DELAY 100 /* in milliseconds */
#define SC_MODE_INCLUDED FALSE
#define SDP_DEBUG FALSE
#define BTSNOOP_MEM FALSE
#define BLE_PRIVACY_SPT FALSE
#define BTM_SCO_ENHANCED_SYNC_ENABLED FALSE
#define BTM_WBS_INCLUDED TRUE /* Enable WBS */
#define BTIF_HF_WBS_PREFERRED FALSE /* Do not prefer WBS */
#endif
