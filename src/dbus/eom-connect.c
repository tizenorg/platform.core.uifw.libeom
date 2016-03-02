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
#include <config.h>

#include "eom.h"
#include "eom-log.h"
#include "eom-dbus.h"
#include "eom-connect.h"
#include "eom-private.h"

API int
eom_output_set_mode(eom_output_id output_id, eom_output_mode_e mode)
{
	bool ret = false;
	GValueArray *msg_array;
	GValueArray *ret_array;
	GValue v = G_VALUE_INIT;

	RETV_IF_FAIL(mode < EOM_OUTPUT_MODE_MAX, EOM_ERROR_INVALID_PARAMETER);

	_eom_mutex_lock();

	INFO("mode: %d\n", mode);

	msg_array = g_value_array_new(0);

	g_value_init(&v, G_TYPE_INT);
	g_value_set_int(&v, output_id);
	msg_array = g_value_array_append(msg_array, &v);
	g_value_set_int(&v, mode);
	msg_array = g_value_array_append(msg_array, &v);

	ret_array = eom_dbus_client_send_message("SetMode", msg_array);
	g_value_array_free(msg_array);
	if (!ret_array) {
		_eom_mutex_unlock();
		return EOM_ERROR_MESSAGE_SENDING_FAILURE;
	}

	ret = g_value_get_int(g_value_array_get_nth(ret_array, 0));

	g_value_array_free(ret_array);

	INFO("SetMode: %s", (ret) ? "success" : "failed");

	_eom_mutex_unlock();

	return (ret) ? EOM_ERROR_NONE : EOM_ERROR_MESSAGE_OPERATION_FAILURE;
}

