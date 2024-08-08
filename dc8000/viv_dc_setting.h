/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Orbita Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Orbita Corporation. This is proprietary information owned by
*    Orbita Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Orbita Corporation.
*
*****************************************************************************/


#ifndef _VIV_DC_SETTING_H_
#define _VIV_DC_SETTING_H_

#include "viv_dc_util.h"

gctINT viv_set_framebuffer(
    Config* config
    );

gctINT viv_set_display(
    Config* config,
    viv_display_size_type type
    );

gctINT viv_set_panel(
    Config* config
    );

gctINT viv_set_output(
    Config* config,
    gctBOOL enable
    );

gctINT viv_set_gamma(
    Config* config,
    gctBOOL enable,
    gctUINT32 index,
    gctUINT16 r,
    gctUINT16 g,
    gctUINT16 b
    );

gctINT viv_set_dither(
    Config* config,
    gctBOOL enable,
    gctUINT32 format,
    gctUINT32 low,
    gctUINT32 high
    );

gctINT viv_set_cursor(
    Config* config,
    gctBOOL enable
    );

gctINT viv_move_cursor(
    Config* config,
    gctUINT32 x,
    gctUINT32 y
    );

gctINT viv_set_register(
    gctUINT32 offset,
    gctUINT32 write,
    gctUINT32* read,
    gctBOOL reset
    );

gctINT viv_reset_framebuffer(
    Config* config
    );

gctINT viv_reset_output_dbi(
    Config* config
    );

gctINT viv_set_commit(
    Config* config
    );

gctINT viv_obt_set_properties(gctINT count);

gctINT viv_map_framebuffer(
    Config* config
    );

gctINT viv_unmap_framebuffer(
    Config* config
    );

gctINT viv_map_cursor(
    Config* config
    );

gctINT viv_unmap_cursor(
    Config* config
    );

gctINT viv_alloc_buffer(
    gctUINT32 Size,
    gctPOINTER * Handle,
    gctUINT32 * HardwareAddress,
    gctPOINTER * Logical,
    gcvPOOL Pool
    );

gctINT viv_free_buffer(
    gctPOINTER Handle
    );

gctINT viv_set_overlay(
    Config* config
);

gctINT viv_set_dest(
    Config *config
    );

gctINT viv_query_chipinfo(
    gctBOOL *Features
    );

gctINT viv_obt_set_properties(gctINT count);

gctINT viv_obt_get_frame_showcount(
    int *count
);

gctINT viv_obt_update_cmd(
    gctUINT32 y_addr,
    gctUINT32 uv_addr,
    int i,
    int show_times
);

#endif

