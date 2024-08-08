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


#ifndef _VIV_DC_PLATFORM_H_
#define _VIV_DC_PLATFORM_H_

#include <math.h>
#include "viv_dc_util.h"

/***********************************/
/*  Platform specific DC Function  */
/***********************************/

gctINT viv_dc_device_init(
    gctVOID
    );

gctVOID viv_dc_device_deinit(
    gctVOID
    );

gctINT viv_dc_device_call(
    gctPOINTER args
    );

gctINT viv_dc_platform_init(
    gctPOINTER args
    );

gctVOID viv_dc_platform_deinit(
    gctPOINTER args
    );

/****************************************/
/*  Platform Specific Generic Function  */
/****************************************/

gctINT viv_os_device_open(
    gctCHAR* name
    );

gctVOID viv_os_device_close(
    gctINT handle
    );

gctINT viv_os_device_call(
    gctINT handle,
    gctUINT32 code,
    gctPOINTER args
    );

gctPOINTER viv_os_file_open(
    gctCHAR* path,
    gctINT type
    );

gctINT viv_os_file_close(
    gctPOINTER p
    );

gctINT viv_os_file_seek(
    gctPOINTER p,
    gctSIZE_T offset,
    gctBOOL end
    );

gctSIZE_T viv_os_file_read(
    gctPOINTER ptr,
    gctSIZE_T size,
    gctPOINTER p
    );

gctUINT16 viv_os_file_read_word(
    gctPOINTER p
    );

gctUINT8 viv_os_file_read_byte(
    gctPOINTER p
    );

gctUINT32 viv_os_file_read_dword(
    gctPOINTER p
    );

gctUINT dc_os_file_tell(
    gctPOINTER p
    );

gctINT dc_os_file_write_word(
    gctPOINTER p,
    gctUINT16 w
    );

gctINT dc_os_file_write_dword(
    gctPOINTER p,
    gctUINT32 dw
    );

gctINT dc_os_file_write_long(
    gctPOINTER p,
    gctINT l)
    ;

gctINT viv_os_file_read_long(
    gctPOINTER p
    );

gctPOINTER viv_os_mem_alloc(
    gctSIZE_T size
    );

gctVOID viv_os_mem_free(
    gctPOINTER addr
    );

gctPOINTER viv_os_memset(
    viv_dc_os *os,
    gctPOINTER addr,
    gctINT value,
    gctSIZE_T num
    );

gctPOINTER viv_os_memcpy(
    gctPOINTER dst,
    gctPOINTER src,
    gctSIZE_T n
    );

gctINT viv_os_memcmp(
    gctCHAR *s1,
    gctCHAR *s2,
    gctSIZE_T n
    );

gctINT viv_os_fgets(
    gctPOINTER Buffer,
    gctUINT32 BufferLen,
    gctPOINTER File
    );

gctINT viv_os_fgets(
    gctPOINTER Buffer,
    gctUINT32 BufferLen,
    gctPOINTER File
    );

gctVOID viv_os_print(
    gctCHAR *message,
    ...
    );

gctPOINTER dc_os_memcpy(
    gctPOINTER dst,
    gctPOINTER src,
    gctSIZE_T num
    );

gctVOID dc_os_sprint(
    gctCHAR *str,
    gctCHAR *message,
    ...
    );

gctVOID viv_os_sprint(
    gctCHAR *str,
    gctCHAR *message,
    ...
    );

gctVOID viv_os_usleep(
    gctSIZE_T time
    );

gctPOINTER
viv_os_create_window(
    gctUINT32 X,
    gctUINT32 Y,
    gctUINT32 Width,
    gctUINT32 Height,
    gctPOINTER * Window
    );

viv_status_type
viv_os_window_draw_image(
    gctPOINTER Window,
    gctINT Left,
    gctINT Top,
    gctINT Right,
    gctINT Bottom,
    gctINT Width,
    gctINT Height,
    gctINT BitsPerPixel,
    gctPOINTER Bits
    );

gctVOID
dc_os_sleep(
    gctUINT32 Delay
    );

#endif

