/*
	FBInk: FrameBuffer eInker, a library to print text & images to an eInk Linux framebuffer
	Copyright (C) 2018-2022 NiLuJe <ninuje@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later

	----

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef __FBINK_DEVICE_ID_H
#define __FBINK_DEVICE_ID_H

// Mainly to make IDEs happy
#include "fbink.h"
#include "fbink_internal.h"

#include <linux/fs.h>

// Used as a sentinel value during device detection
#define DEVICE_INVALID UINT16_MAX

#ifndef FBINK_FOR_LINUX
#	if defined(FBINK_FOR_KOBO) || defined(FBINK_FOR_CERVANTES)
// NOTE: This is NTX's homegrown hardware tagging, c.f., arch/arm/mach-imx/ntx_hwconfig.h in a Kobo kernel, for instance
#		define HWCONFIG_DEVICE "/dev/mmcblk0"
#		define HWCONFIG_OFFSET (1024 * 512)
#		define HWCONFIG_MAGIC  "HW CONFIG "
#		if defined(FBINK_FOR_CERVANTES)
// Keep it to the minimum on Cervantes
typedef struct __attribute__((__packed__))
{
#			pragma GCC diagnostic push
#			pragma GCC diagnostic ignored "-Wattributes"
	char magic[10] __attribute__((nonstring));     // HWCONFIG_MAGIC (i.e., "HW CONFIG ")
	char version[5] __attribute__((nonstring));    // In Kobo-land, up to "v3.3" on Mk.7+
#			pragma GCC diagnostic pop
	uint8_t len;    // Length (in bytes) of the full payload, header excluded (up to 76 on v3.3)
	// Header stops here, actual data follows
	uint8_t pcb_id;    // First field is the PCB ID, which dictates the device model, the only thing we care about ;)
} NTXHWConfig;
#		elif defined(FBINK_FOR_KOBO)
// On Kobo, we'll need dig a little deeper to pickup the CPU & display resolution, too.
// So we'll first read the header only, because its size is fixed,
// and then trust it to tell us how much data there is to read for the payload.
typedef struct __attribute__((__packed__))
{
#			pragma GCC diagnostic push
#			pragma GCC diagnostic ignored "-Wattributes"
	char    magic[10] __attribute__((nonstring));     // HWCONFIG_MAGIC (i.e., "HW CONFIG ")
	char    version[5] __attribute__((nonstring));    // In Kobo-land, up to "v3.3" on Mk.7+
#			pragma GCC diagnostic pop
	uint8_t len;    // Length (in bytes) of the full payload, header excluded (up to 76 on v3.3)
} NTXHWConfig;
// Index of the few fields we're interested in inside the payload...
#			define KOBO_HWCFG_PCB               0
// NOTE: This one *might* help w/ ntxBootRota, although a direct mapping seems impossible...
#			define KOBO_HWCFG_DisplayPanel      10
// NOTE: Accelerometer
#			define KOBO_HWCFG_RSensor           11
#			define KOBO_HWCFG_CPU               27
// NOTE: This one was added in v1.0, while the original NTX Touch was only on v0.7,
//       which is why we handle this dynamically, instead of relying on a fixed-size struct...
#			define KOBO_HWCFG_DisplayResolution 31
// NOTE: This one tells us a bit about the potential rotation trickeries on some models (ntxRotaQuirk)...
#			define KOBO_HWCFG_DisplayBusWidth   35
#		endif    // FBINK_FOR_CERVANTES
#	endif            // FBINK_FOR_KOBO || FBINK_FOR_CERVANTES

#	if defined(FBINK_FOR_KINDLE)
#		define KINDLE_SERIAL_NO_LENGTH 16
static bool     is_kindle_device(uint32_t);
static bool     is_kindle_device_new(uint32_t);
static uint32_t from_base(const char*, uint8_t);
static char*    to_base(int64_t, uint8_t);
static void     identify_kindle(void);
#        elif defined(FBINK_FOR_CERVANTES)
static void identify_cervantes(void);
#        elif defined(FBINK_FOR_KOBO)
// List of NTX/Kobo PCB IDs... For a given device,
// what we get in the NTXHWConfig payload @ index KOBO_HWCFG_CPU corresponds to an index in this array.
// Can thankfully be populated from /bin/ntx_hwconfig with the help of strings -n2 and a bit of sed, i.e.,
// sed -re 's/(^)(.*?)($)/"\2",/g' Kobo_PCB_IDs.txt
// Double-check w/ ntx_hwconfig -l -s /dev/mmcblk0
// NOTE: Last updated on 09/21/22, from FW 4.34.20097 (NTX HwConfig v3.7.6.33.296-20220803)
//       Last checked on 09/21/22 against 4.34.20097
/*
static const char* kobo_pcbs[] = {
	"E60800", "E60810", "E60820",  "E90800", "E90810", "E60830", "E60850", "E50800", "E50810", "E60860",  "E60MT2",
	"E60M10", "E60610", "E60M00",  "E60M30", "E60620", "E60630", "E60640", "E50600", "E60680", "E60610C", "E60610D",
	"E606A0", "E60670", "E606B0",  "E50620", "Q70Q00", "E50610", "E606C0", "E606D0", "E606E0", "E60Q00",  "E60Q10",
	"E60Q20", "E606F0", "E606F0B", "E60Q30", "E60QB0", "E60QC0", "A13120", "E60Q50", "E606G0", "E60Q60",  "E60Q80",
	"A13130", "E606H2", "E60Q90",  "ED0Q00", "E60QA0", "E60QD0", "E60QF0", "E60QH0", "E60QG0", "H70000",  "ED0Q10",
	"E70Q00", "H40000", "NC",      "E60QJ0", "E60QL0", "E60QM0", "E60QK0", "E70S00", "T60Q00", "C31Q00",  "E60QN0",
	"E60U00", "E70Q10", "E60QP0",  "E60QQ0", "E70Q20", "T05R00", "M31Q00", "E60U10", "E60K00", "E80K00",  "E70Q30",
	"EA0Q00", "E60QR0", "ED0R00",  "E60QU0", "E60U20", "M35QE0", "E60QT0", "E70Q50", "T60U00", "E60QV0",  "E70K00",
	"T60P00", "TA0P00", "MXXQ4X",  "E60P20", "T60P10", "E60K10", "EA0P10", "E60P40", "E70P10", "E70P20",  "E80P00",
	"E70P20", "E60P50", "E70K10",  "E70P50", "E60K20", "E60P60", "EA0P00", "E60P70", "U13Q50", "EA0T00"
};
*/
// And match (more or less accurately, for some devices) that to what we've come to know as a device code,
// because that's what we actually care about...
// c.f., tools/pcb_to_ids.py
static const unsigned short int kobo_ids[] = { 0,   0,   0, 0,   0,   0,     0,   0,   0,   0,   0,   0,   310, 0,   0,
					       0,   0,   0, 0,   0,   310,   320, 0,   0,   330, 0,   0,   340, 350, 0,
					       0,   0,   0, 0,   360, 360,   0,   330, 0,   0,   0,   370, 0,   0,   0,
					       0,   371, 0, 0,   0,   32627, 0,   0,   0,   0,   373, 0,   0,   0,   375,
					       374, 0,   0, 375, 0,   0,     375, 0,   0,   0,   0,   0,   0,   376, 376,
					       377, 0,   0, 0,   0,   0,     382, 0,   0,   0,   0,   0,   384, 0,   0,
					       0,   0,   0, 0,   387, 0,     0,   0,   383, 0,   0,   388, 0,   0,   0 };

// Same idea, but for the various NTX/Kobo Display Panels...
/*
static const char* kobo_disp_panel[] = { "6\" Left EPD",     "6\" Right EPD",     "9\" Right EPD",     "5\" Left EPD",
					 "5\" Right EPD",    "6\" Top EPD",       "6\" Bottom EPD",    "5\" Top EPD",
					 "5\" Bottom EPD",   "6.8\" Top EPD",     "6.8\" Bottom EPD",  "NC",
					 "13.3\" Left EPD",  "13.3\" Right EPD",  "13.3\" Bottom EPD", "13.3\" Top EPD",
					 "7.8\" Bottom EPD", "7.8\" Top EPD",     "7.8\" Left EPD",    "7.8\" Right EPD",
					 "7.3\" Bottom EPD", "7.3\" Top EPD",     "7.3\" Left EPD",    "7.3\" Right EPD",
					 "4.7\" Bottom EPD", "4.7\" Top EPD",     "4.7\" Left EPD",    "4.7\" Right EPD",
					 "10.3\" Top EPD",   "10.3\" Bottom EPD", "10.3\" Left EPD",   "10.3\" Right EPD",
					 "8\" Bottom EPD",   "8\" Top EPD",       "8\" Left EPD",      "8\" Right EPD",
					 "7\" Bottom EPD",   "7\" Top EPD",       "7\" Left EPD",      "7\" Right EPD" };
*/
// And the accelerometers...
/*
static const char* kobo_gyros[] = { "No", "Rotary Encoder", "G Sensor", "KL25",   "MMA8X5X",
				    "NC", "KX122",          "KXTJ3",    "STK8331" };
*/
// And for the various NTX/Kobo CPUs...
/*
static const char* kobo_cpus[] = { "mx35",   "m166e",  "mx50",  "x86",    "mx6",    "mx6sl", "it8951", "i386",   "mx7d",
				   "mx6ull", "mx6sll", "mx6dl", "rk3368", "rk3288", "b300",  "b810",   "mt8113t" };
*/
// And for the various NTX/Kobo Display Resolutions...
/*
static const char* kobo_disp_res[] = { "800x600",    "1024x758",    "1024x768",  "1440x1080", "1366x768", "1448x1072",
				       "1600x1200",  "400x375x2",   "1872x1404", "960x540",   "NC",       "2200x1650",
				       "1440x640x4", "1600x1200x4", "1920x1440", "1264x1680", "1680x1264" };
*/
// And for the various NTX/Kobo Display Bus Widths...
/*
static const char* kobo_disp_busw[] = { "8Bits", "16Bits", "8Bits_mirror", "16Bits_mirror", "NC" };
*/

// And for mainline kernels, this is where we poke for device identification
#                define MAINLINE_DEVICE_ID_SYSFS "/sys/firmware/devicetree/base/compatible"

static void set_kobo_quirks(unsigned short int);
static void identify_kobo(void);
static void identify_mainline(void);
#        elif defined(FBINK_FOR_REMARKABLE)
static void identify_remarkable(void);
#        elif defined(FBINK_FOR_POCKETBOOK)
static void identify_pocketbook(void);
#        endif    // FBINK_FOR_KINDLE

static void identify_device(void);
#endif    // !FBINK_FOR_LINUX

#endif
