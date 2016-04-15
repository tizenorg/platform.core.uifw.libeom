/**************************************************************************
 *
 * eom (external output manager)
 *
 * Copyright 2014 Samsung Electronics co., Ltd. All Rights Reserved.
 *
 * Contact:
 * SooChan Lim <sc1.lim@samsung.com>
 * Boram Park <boram1288.park@samsung.com>
 * Changyeon Lee <cyeon.lee@samsung.com>
 * JunKyeong Kim <jk0430.kim@samsung.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
**************************************************************************/

#ifndef __EOM_PRIVATE_H__
#define __EOM_PRIVATE_H__

#include "eom.h"
#include <Elementary.h>

typedef struct {
	eom_output_id id;
	eom_output_type_e type;
	eom_output_mode_e output_mode;
	eom_output_attribute_e attribute;
	eom_output_attribute_state_e state;

	int width;
	int height;
	int mm_width;
	int mm_height;
} eom_output_info;

/* notify callback function prototype */
typedef void (*notify_func) (void *data, GArray *array);

void _eom_mutex_lock(void);
void _eom_mutex_unlock(void);

#endif /* __EOM_PRIVATE_H__ */
