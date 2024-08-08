/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Orbita Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Orbita Corporation. This is proprietary information owned by
*    Orbita Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Orbita Corporation.
*
*****************************************************************************/


#ifndef _VIV_DC_CORE_H_
#define _VIV_DC_CORE_H_

#include "viv_dc_type.h"

typedef struct _viv_dc_os viv_dc_os;

typedef struct _dc_framebuffer {
    gctUINT YAddress;
    gctUINT UAddress;
    gctUINT VAddress;
    gctUINT UStride;
    gctUINT VStride;

    gctUINT rotAngle;
    gctUINT alphaMode;
    gctUINT alphaValue;

    gctUINT lut[256];

    /* Original size in pixel before rotation and scale. */
    gctUINT width;
    gctUINT height;
    gctUINT alignedWidth[3];
    gctUINT alignedHeight[3];

    gctUINT tileMode;
    gctUINT scale;
    gctUINT scaleFactorX;
    gctUINT scaleFactorY;
    gctUINT filterTap;
    gctUINT horizontalFilterTap;

    gctUINT horKernel[128];
    gctUINT verKernel[128];

    gctUINT swizzle;
    gctUINT uvSwizzle;

    gctUINT colorKey;
    gctUINT colorKeyHigh;
    gctUINT bgColor;
    gctUINT transparency;
    gctUINT clearFB;
    gctUINT clearValue;

    gctUINT initialOffsetX;
    gctUINT initialOffsetY;

    gctUINT compressed;
    gctUINT tileStatusAddress[3];
}
dc_framebuffer;

typedef struct _dc_overlay {
    gctUINT mode;
    gctUINT srcAlphaValue;
    gctUINT srcAlphaMode;
    gctUINT srcGlobalAlphaValue;
    gctUINT srcGlobalAlphaMode;
    gctUINT enable;
    gctUINT format;

    gctUINT logical[3];
    gctUINT address[3];
    gctUINT tileStatusAddress[3];
    gctUINT stride[3];
    gctUINT tlX;
    gctUINT tlY;
    gctUINT brX;
    gctUINT brY;

    gctUINT rotAngle;
    gctUINT width;
    gctUINT height;
    gctUINT alignedWidth[3];
    gctUINT alignedHeight[3];

    gctUINT swizzle;
    gctUINT uvSwizzle;

    gctUINT colorKey;
    gctUINT colorKeyHigh;
    gctUINT transparency;

    gctUINT dstAlphaMode;
    gctUINT dstAlphaValue;

    gctUINT dstGlobalAlphaMode;
    gctUINT dstGlobalAlphaValue;

    gctUINT clearOverlay;
    gctUINT clearValue;

    gctUINT tileMode;

    gctUINT lut[256];

    gctUINT compressed;

    gctUINT scale;
    gctUINT scaleFactorX;
    gctUINT scaleFactorY;
    gctUINT filterTap;
    gctUINT horizontalFilterTap;
    gctUINT horKernel[128];
    gctUINT verKernel[128];

    gctUINT initialOffsetX;
    gctUINT initialOffsetY;


}
dc_overlay;

typedef struct _dc_dest {
    gctUINT address;
    gctUINT stride;
    gctUINT format;
}
dc_dest;

#define DC_OVERLAY_NUM 1

typedef struct _viv_dc_cursor {
    gctBOOL enable;
    gctUINT phys_addr;
    gctUINT type;
    gctUINT x, y;
    gctUINT hot_x, hot_y;
    gctUINT bg_color, fg_color;
    gctBOOL dirty;
}
viv_dc_cursor;

typedef struct _viv_dc_display {
    /* display */
    gctUINT dpy_hline, dpy_htotal;
    gctUINT dpy_vline, dpy_vtotal;
    gctUINT dpy_hsync_start, dpy_hsync_end;
    gctUINT dpy_vsync_start, dpy_vsync_end;
    gctBOOL dpy_hsync_polarity;
    gctBOOL dpy_vsync_polarity;
    gctBOOL display_dirty;

    /* framebuffer */
    gctUINT fb_phys_addr[3];
    gctUINT fb_format;
    gctUINT fb_tiling;
    gctUINT fb_yuv_type;
    gctUINT fb_stride[3];
    gctUINT fb_astride[3];
    gctBOOL fb_dirty;

    /* panel */
    gctBOOL panel_dep_polarity;
    gctBOOL panel_dap_polarity;
    gctBOOL panel_clockp_polarity;
    gctBOOL panel_dirty;

    /* gamma correction */
#define GAMMA_INDEX_MAX 256
    gctBOOL gamma_enable;
    gctUINT gamma[GAMMA_INDEX_MAX][3];
    gctBOOL gamma_dirty;

    /* dither */
    gctBOOL dither_enable;
    gctUINT dither_red_channel;
    gctUINT dither_green_channel;
    gctUINT dither_blue_channel;
    gctUINT dither_table_low;
    gctUINT dither_table_high;
    gctBOOL dither_dirty;

    /* output */
    gctBOOL output_enable;
    gctUINT output_type;
    gctUINT output_dbi_type;
    gctUINT output_dbi_format;
    gctUINT output_dbi_actime;
    gctUINT output_dbi_period[2];
    gctUINT output_dbi_eor_assert[2];
    gctUINT output_dbi_cs_assert[2];
    gctBOOL output_dbi_polarity;
    gctUINT output_dpi_format;
    gctBOOL output_dirty;

    /**/
    dc_framebuffer framebuffer;

    /* destination. */
    dc_dest dest;
    gctBOOL dest_dirty;

    /* Overlay. */
    dc_overlay overlay[DC_OVERLAY_NUM];
    gctBOOL overlay_dirty;
}
viv_dc_display;

typedef struct _viv_dc_core {
    /* os */
    struct _viv_dc_os *os;

    /* hardware */
    struct _viv_dc_hardware *hardware;

    /* cursor */
    struct _viv_dc_cursor cursor;

    struct _viv_dc_display display;

#if dcENABLE_MMU
    /* MMU */
    struct _dc_mmu *mmu;
#endif
}
viv_dc_core;

/************************************************************************
 * Below is the main interface.
 * It can be called directly after initialiation.
 ************************************************************************/

 /*
  * Set display horizontal configuration.
  *
  *  @core:
  *      Point to core object.
  *
  *  @line_pixels:
  *      Visible number of horizontal pixels per line.
  *
  *  @total_pixels:
  *      Total number of horizontal pixels per line.
  *      Including blank pixels.
  *
  *  @hsync_start:
  *      Start of the horizontal sync pulse.
  *
  *  @hsync_end:
  *      End of the horizontal sync pulse.
  *
  *  @hsync_polarity:
  *      Polarity of the horizontal sync pulse.
  *      Parameters: SET_POSITIVE / SET_NEGATIVE
  */
gctVOID viv_conf_display_set_horizontal(
    viv_dc_core *core,
    gctUINT line_pixels,
    gctUINT total_pixels,
    gctUINT hsync_start,
    gctUINT hsync_end,
    gctBOOL hsync_polarity
    );

 /*
  * Set display vertical configuration.
  *
  *  @core:
  *      Point to core object.
  *
  *  @line_pixels:
  *      Visible number of vertical pixels per line.
  *
  *  @total_pixels:
  *      Total number of vertical pixels per line.
  *      Including blank pixels.
  *
  *  @vsync_start:
  *      Start of the vertical sync pulse.
  *
  *  @vsync_end:
  *      End of the vertical sync pulse.
  *
  *  @vsync_polarity:
  *      Polarity of the vertical sync pulse.
  *      Parameters: SET_POSITIVE / SET_NEGATIVE
  */
gctVOID viv_conf_display_set_vertical(
    viv_dc_core *core,
    gctUINT line_pixels,
    gctUINT total_pixels,
    gctUINT vsync_start,
    gctUINT vsync_end,
    gctBOOL vsync_polarity
    );

 /*
  * Set framebuffer config.
  *
  *  @core:
  *      Point to core object.
  *
  *  @format:
  *      Frambuffer format.
  *      Parameters: vivARGB1555 / vivARGB4444 / RGB565 / ARGB8888
  *
  *  @tiling:
  *      Framebuffer mode. Current tile mode is bound with YUV format.
  *      Parameters: vivLINEAR / vivTILED
  */
gctVOID viv_conf_framebuffer_set_config(
    viv_dc_core *core,
    viv_format_type format,
    viv_tiling_type tiling
    );

 /*
  * Set framebuffer stride.
  *
  *  @core:
  *      Point to core object.
  *
  *  @stride:
  *      Frambuffer strdie.
  *     Parameters must be 128 byte aligned.
  */
gctVOID viv_conf_framebuffer_set_stride(
    viv_dc_core *core,
    gctUINT stride
    );

 /*
  * Set framebuffer address.
  *
  *  @core:
  *      Point to core object.
  *
  *  @addr:
  *      Frambuffer physical address that dc hardware can access.
  */
gctVOID viv_conf_framebuffer_set_address(
    viv_dc_core *core,
    gctUINT addr
    );

gctVOID viv_conf_framebuffer_set_framebuffer(
    viv_dc_core *core,
    dc_framebuffer *framebuffer
    );

 /*
  * Reset framebuffer.
  *
  * This function should be called before other framebuffer setting
  * to clear all framebuffer states.
  *
  *  @core:
  *      Point to core object.
  */
gctVOID viv_conf_framebuffer_reset(viv_dc_core *core);

 /*
  * Set panel configuration.
  *
  *  @core:
  *      Point to core object.
  *
  *  @dep_polarity:
  *      Polarity of data enable.
  *      Parameters: SET_POSITIVE / SET_NEGATIVE
  *
  *  @dap_polarity:
  *      Polarity of data.
  *      Parameters: SET_POSITIVE / SET_NEGATIVE
  *
  *  @clockp_polarity:
  *      Polarity of clock.
  *      Parameters: SET_POSITIVE / SET_NEGATIVE
  */
gctVOID viv_conf_panel_set_config(
    viv_dc_core *core,
    gctBOOL dep_polarity,
    gctBOOL dap_polarity,
    gctBOOL clockp_polarity
    );

 /*
  * Enable/Disable output.
  *
  *  @core:
  *      Point to core object.
  *
  *  @enable:
  *      Enable output or not.
  *     When output is disabled, all pixels will be black. This allows panel to
  *     have correct timing but without any pixels.
  *      Parameters: SET_ENABLE / SET_DISABLE
  */
gctVOID viv_conf_output_enable(
    viv_dc_core *core,
    gctBOOL enable
    );

 /*
  * Select output type.
  *
  *  @core:
  *      Point to core object.
  *
  *  @output_type:
  *      Ouput bus type.
  *      Parameters: vivDBI / vivDPI
  */
gctVOID viv_conf_output_select(
    viv_dc_core *core,
    viv_output_type output_type
    );

 /*
  *  Reset DBI interface to idle state.
  *
  *  @core:
  *      Point to core object.
  */
gctVOID viv_conf_output_dbi_reset(
    viv_dc_core *core
    );

 /*
  *  Set DBI output configuration.
  *
  *  @core:
  *      Point to core object.
  *
  *  @dbi_type:
  *      DBI inferface type.
  *      Parameters: vivDBI_AFIXED / vivDBI_ACLOCK / vivDBI_B
  *
  *  @dbi_format
  *      DBI interface data format.
  *      Parameters: vivD8R3G3B2 / vivD8R4G4B4 / vivD8R5G5B5 / vivD8R6G6B6 / vivD8R8G8B8 /
                    vivD9R6G6B6 / vivD16R3G3B2 / vivD16R4G4B4 / vivD16R5G5B5 /
                    vivD16R6G6B6OP1 / vivD16R6G6B6OP2 / vivD16R8G8B8OP1 / vivD16R8G8B8OP2
  *
  *  @dbi_polarity:
  *      DBI D/CX Pin Polarity
  *      Parameters: SET_POSITIVE / SET_NEGATIVE
  *
  *  @dbi_actime:
  *      Time unit for AC characteristics.
  */
gctVOID viv_conf_output_dbi_set_config(
    viv_dc_core *core,
    viv_dbi_type dbi_type,
    viv_format_type dbi_format,
    gctBOOL dbi_polarity,
    gctUINT dbi_actime
    );

 /*
  *  Set DPI interface data format.
  *
  *  @core:
  *      Point to core object.
  *
  *  @dpi_format:
  *      DPI data format.
  *      Parameters: vivD16CFG1 / vivD16CFG2/ vivD16CFG3 / vivD18CFG1 / vivD18CFG2 / vivD24
  */
gctVOID viv_conf_output_dpi_set_format(
    viv_dc_core *core,
    viv_format_type dpi_format
    );

 /*
  *  Write DBI AC characteristics definition register1/2.
  *
  *  @core:
  *      Point to core object.
  *
  *  @reg_select:
  *      Select register. 2 registers in all.
  *      Parameters: 1 / 2
  *
  *  @dbi_period:
  *      Single write period duration cycle number = Setting*(DbiAcTimeUnit+1).
  *
  *  @dbi_eor_assert:
  *      Cycle number = Setting*(DbiAcTimeUnit+1).
  *      Parameters:
  *     vivDBI_AFIXED: not used
  *     vivDBI_ACLOCK: time to assert E
  *     vivDBI_B:      time to assert WRX
  *
  *  @dbi_eor_assert:
  *      Cycle number = Setting*(DbiAcTimeUnit+1).
  *      Parameters:
  *     vivDBI_AFIXED: time to assert CSX
  *     vivDBI_ACLOCK: not used
  *     vivDBI_B:      time to assert CSX
  */
gctVOID viv_conf_output_dbi_set_write(
    viv_dc_core *core,
    gctUINT reg_select,
    gctUINT dbi_period,
    gctUINT dbi_eor_assert,
    gctUINT dbi_cs_assert
    );

 /*
  *  DBI Command input/output port.
  *  Writes to this register will send command or data to the DBI port.
  *
  *  @core:
  *      Point to core object.
  *
  *  @type:
  *      Determing the input parameter is command or data.
  *      Parameters: vivDBI_COMMAND_ADDRESS / vivDBI_COMMAND_MEM / vivDBI_COMMAND_DATA
  *
  *  @command:
  *      Content of command or data.
  */
gctVOID viv_conf_output_dbi_set_command(
    viv_dc_core *core,
    viv_dbi_command_type type,
    gctUINT command
    );

 /*
  * Enable/Disable dither.
  *
  *  @core:
  *      Point to core object.
  *
  *  @enable:
  *      Enable dither or not.
  *     When enabled, R8G8B8 mode show better on panels with fewer bits per pixel.
  *      Parameters: SET_ENABLE / SET_DISABLE
  */
gctVOID viv_conf_dither_enable(
    viv_dc_core *core,
    gctBOOL enable
    );

 /*
  * Set dither setting.
  *
  *  @core:
  *      Point to core object.
  *
  *  @red_channel:
  *      Significant bit width for red channel.
  *
  *  @green_channel:
  *      Significant bits width for green channel.
  *
  *  @blue_channel:
  *      Significant bits width for blue channel.
  *
  *  @table_low:
  *      Dither table of refreence value is 64bits. These are the lower 32 bits.
  *     Y0_X0    3:0  Dither reference value at coordinate (0, 0)
  *     Y0_X1    7:4  Dither reference value at coordinate (1, 0)
  *     Y0_X2   11:8  Dither reference value at coordinate (2, 0)
  *     Y0_X3  15:12  Dither reference value at coordinate (3, 0)
  *     Y1_X0  19:16  Dither reference value at coordinate (0, 1)
  *     Y1_X1  23:20  Dither reference value at coordinate (1, 1).
  *     Y1_X2  27:24  Dither reference value at coordinate (2, 1)
  *     Y1_X3  31:28  Dither reference value at coordinate (3, 1)
  *
  *  @table_high:
  *      Dither table of reference value is 64bits. These are the high 32 bits.
  *     Y2_X0    3:0  Dither reference value at coordinate (0, 2)
  *     Y2_X1    7:4  Dither reference value at coordinate (1, 2)
  *     Y2_X2   11:8  Dither reference value at coordinate (2, 2)
  *     Y2_X3  15:12  Dither reference value at coordinate (3, 2)
  *     Y3_X0  19:16  Dither reference value at coordinate (0, 3)
  *     Y3_X1  23:20  Dither reference value at coordinate (1, 3).
  *     Y3_X2  27:24  Dither reference value at coordinate (2, 3)
  *     Y3_X3  31:28  Dither reference value at coordinate (3, 3)
  */
gctVOID viv_conf_dither_set_config(
    viv_dc_core *core,
    gctUINT red_channel,
    gctUINT green_channel,
    gctUINT blue_channel,
    gctUINT table_low,
    gctUINT table_high
    );

 /*
  * Enable/Disable gamma correction.
  *
  *  @core:
  *      Point to core object.
  *
  *  @enable:
  *      Enable gamma correction or not.
  *     When Gamma is enabled, the R, G, and B channels will berouted
  *     through the Gamma LUT to perform gamma correction.
  *      Parameters: SET_ENABLE / SET_DISABLE
  */
gctVOID viv_conf_gamma_enable(
    viv_dc_core *core,
    gctBOOL enable
    );

 /*
  * Set gamma translation value.
  *
  *  @core:
  *      Point to core object.
  *
  *  @index:
  *      Index into the Gamma LUT.
  *     Each index is corresponding to one set of red+green+blue gamma value.
  *
  *  @red:
  *      Gamma correction RED value
  *
  *  @green:
  *      Gamma correction GREEN value
  *
  *  @blue:
  *      Gamma correction BLUE value
  */
gctVOID viv_conf_gamma_set_config(
    viv_dc_core *core,
    gctUINT index,
    gctUINT red,
    gctUINT green,
    gctUINT blue
    );

 /*
  * Enable/Disable hardware cursor.
  *
  *  @core:
  *      Point to core object.
  *
  *  @enable:
  *      Enable hardware cursor or not
  *      Parameters: SET_ENABLE / SET_DISABLE
  */
gctVOID viv_conf_cursor_enable(
    viv_dc_core *core,
    gctBOOL enable
    );

 /*
  * Set cursor type.
  *
  *  @core:
  *      Point to core object.
  *
  *  @type:
  *      Cursor type.
  *      Parameters: vivCURSOR_ARGB / vivCURSOR_MASK
  */
gctVOID viv_conf_cursor_set_type(
    viv_dc_core *core,
    viv_cursor_type type
    );

 /*
  * Set cursor location in the screen.
  *
  *  @core:
  *      Point to core object.
  *
  *  @x:
  *      X-coordination(in number of pixels) of cursor.
  *
  *  @y:
  *      Y-coordination(in number of pixels) of cursor.
  */
gctVOID viv_conf_cursor_set_pos(
    viv_dc_core *core,
    gctUINT x,
    gctUINT y
    );

 /*
  * Set cursor hot spot inside cursor image.
  *
  *  @core:
  *      Point to core object.
  *
  *  @x:
  *      Number of pixels of the horizontal offset X-coordinate
  *      within the 32x32 pixel cursor image for the cursor hotspot.
  *
  *  @y:
  *      Number of pixels of the vertical offset Y-coordinate
  *      within the 32x32 pixel cursor image for the cursor hotspot.
  */
gctVOID viv_conf_cursor_set_hotspot(
    viv_dc_core *core,
    gctUINT x,
    gctUINT y
    );

 /*
  * Set color for the masked cursor.
  *
  *  @core:
  *      Point to core object.
  *
  *  @bg:
  *      Cursor background color.
  *
  *  @fg:
  *      Cursor foreground color.
  */
gctVOID viv_conf_cursor_set_color(
    viv_dc_core *core,
    gctUINT bg,
    gctUINT fg
    );

 /*
  * Set cursor address in memory.
  *
  *  @core:
  *      Point to core object.
  *
  *  @addr:
  *      Cursor image address in memory that dc hardware can access.
  */
gctVOID viv_conf_cursor_set_address(
    viv_dc_core *core,
    gctUINT addr
    );

 /*
  * Set destination address in memory.
  *
  *  @core:
  *      Point to core object.
  *
  *  @addr:
  *      destination address in memory that dc hardware write memory to memory result to.
  */
gctVOID viv_conf_dest_set_address(
    viv_dc_core *core,
    gctUINT addr
    );

 /*
  * Enable/Disable interrupt.
  *
  *  @core:
  *      Point to core object.
  *
  *  @enable:
  *      Enable dc interrupt or not
  *      Parameters: SET_ENABLE / SET_DISABLE
  */
gctVOID viv_conf_interrupt_enable(
    viv_dc_core *core,
    gctBOOL enable
    );

 /*
  * Commit the commands to DC after setting.
  *
  *  @core:
  *      Point to core object.
  */
gctVOID viv_conf_commit(
    viv_dc_core *core
    );

 /*
  * Get interrupt.
  *
  * Return interrupt signal if existed.
  * The signal will be pulled up only after enable interrupt through viv_conf_interrupt_enable().
  *
  *  @core:
  *      Point to core object.
  */
gctUINT viv_conf_interrupt_get(
    viv_dc_core *core
    );

 /*
  * Get hardware info.
  *
  *  @core:
  *      Point to core object.
  *
  *  @chip_id:
  *      Show the ID for the chip in BCD.
  *
  *  @chip_revision:
  *      Show the revision for the chip in BCD.
  *
  *  @chip_patch_revision:
  *      Show the patch revision level for the chip.
  *
  *  @chip_info:
  *      Show chip info.
  *
  *  @product_id:
  *      Show product id.
  *
  *  @product_date:
  *      Show the release date for the IP.
  *
  *  @product_time:
  *      Show the release time for the IP.
  */
gctVOID viv_conf_info_get(
    viv_dc_core *core,
    gctUINT* chip_id,
    gctUINT* chip_revision,
    gctUINT* chip_patch_revision,
    gctUINT* chip_info,
    gctUINT* product_id,
    gctUINT* eco_id,
    gctUINT* customer_id,
    gctUINT* product_date,
    gctUINT* product_time
    );

gctVOID dc_conf_overlay(
    viv_dc_core *core,
    dc_overlay *overlay
    );

gctVOID dc_conf_dest (
    viv_dc_core *core,
    dc_dest *dest
    );

gctVOID viv_conf_shadow_register_pending_enable(
    viv_dc_core *core,
    gctBOOL enable
    );

gctUINT32 viv_conf_flip_in_progress_value(
    viv_dc_core *core
    );

gctVOID dc_conf_query_chip_identity(
    viv_dc_core *core,
    viv_interface_query_chip_identity_ptr Identity
    );

/*
 * Get the frame counter.
 *
 * Return the frame counter.
 *
 * @core:
 *      Point to core object.
 */
gctUINT viv_conf_frame_counter_get(
    viv_dc_core *core
    );


#endif

