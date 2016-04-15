/**************************************************************************
 *
 * eom (external output manager)
 *
 * Copyright 2015 Samsung Electronics co., Ltd. All Rights Reserved.
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
#include "eom_internal.h"
#include "eom-log.h"
#include "eom-wayland.h"
#include "eom-private.h"
#include "eom-client-protocol.h"
#include "xdg-shell-client-protocol.h"
#include <Ecore_Wayland.h>

typedef struct _EomWaylandClientInfo {
	/* wl */
	struct wl_display *display;
	struct wl_registry *registry;

	struct wl_eom *eom;

	/* eom wayland output list */
	struct wl_list eom_wl_output_list;
	int num_outputs;

	notify_func func;
} EomWaylandClientInfo;

typedef struct _EomWaylandOutput {
	eom_output_id id;
	struct wl_output *output;

	/* current output data */
	int32_t x;
	int32_t y;
	int32_t physical_width;
	int32_t physical_height;
	enum wl_output_subpixel subpixel;
	const char *make;
	const char *model;
	enum wl_output_transform transform;

	uint32_t flags;
	int32_t width;
	int32_t height;
	int32_t refresh;

	int32_t factor;

	/* current eom data */
	enum wl_eom_type eom_type;
	enum wl_eom_status eom_status;
	enum wl_eom_mode eom_mode;
	enum wl_eom_attribute eom_attribute;
	enum wl_eom_attribute_state eom_attribute_state;

	/* client info */
	EomWaylandClientInfo *client_info;

	struct wl_list link;
} EomWaylandOutput;

static EomWaylandClientInfo wl_client_info;
static int eom_wayland_init;

static eom_output_type_e
_convert_to_eom_output_type(enum wl_eom_type eom_type)
{
	eom_output_type_e output_type = EOM_OUTPUT_TYPE_UNKNOWN;

	switch (eom_type) {
	case WL_EOM_TYPE_NONE:
		output_type = EOM_OUTPUT_TYPE_UNKNOWN;
		break;
	case WL_EOM_TYPE_VGA:
		output_type = EOM_OUTPUT_TYPE_VGA;
		break;
	case WL_EOM_TYPE_DIVI:
		output_type = EOM_OUTPUT_TYPE_DVII;
		break;
	case WL_EOM_TYPE_DIVD:
		output_type = EOM_OUTPUT_TYPE_DVID;
		break;
	case WL_EOM_TYPE_DIVA:
		output_type = EOM_OUTPUT_TYPE_DVIA;
		break;
	case WL_EOM_TYPE_COMPOSITE:
		output_type = EOM_OUTPUT_TYPE_COMPOSITE;
		break;
	case WL_EOM_TYPE_SVIDEO:
		output_type = EOM_OUTPUT_TYPE_SVIDEO;
		break;
	case WL_EOM_TYPE_LVDS:
		output_type = EOM_OUTPUT_TYPE_LVDS;
		break;
	case WL_EOM_TYPE_COMPONENT:
		output_type = EOM_OUTPUT_TYPE_COMPONENT;
		break;
	case WL_EOM_TYPE_9PINDIN:
		output_type = EOM_OUTPUT_TYPE_9PINDIN;
		break;
	case WL_EOM_TYPE_DISPLAYPORT:
		output_type = EOM_OUTPUT_TYPE_DISPLAYPORT;
		break;
	case WL_EOM_TYPE_HDMIA:
		output_type = EOM_OUTPUT_TYPE_HDMIA;
		break;
	case WL_EOM_TYPE_HDMIB:
		output_type = EOM_OUTPUT_TYPE_HDMIB;
		break;
	case WL_EOM_TYPE_TV:
		output_type = EOM_OUTPUT_TYPE_TV;
		break;
	case WL_EOM_TYPE_EDP:
		output_type = EOM_OUTPUT_TYPE_EDP;
		break;
	case WL_EOM_TYPE_VIRTUAL:
		output_type = EOM_OUTPUT_TYPE_VIRTUAL;
		break;
	case WL_EOM_TYPE_DSI:
		output_type = EOM_OUTPUT_TYPE_DSI;
		break;
	default:
		ERR("no type.");
		break;
	}

	return output_type;
}

static eom_output_mode_e
_convert_to_eom_output_mode(enum wl_eom_mode eom_mode)
{
	eom_output_mode_e output_mode = EOM_OUTPUT_MODE_NONE;

	switch (eom_mode) {
	case WL_EOM_MODE_NONE:
		output_mode = EOM_OUTPUT_MODE_NONE;
		break;
	case WL_EOM_MODE_MIRROR:
		output_mode = EOM_OUTPUT_MODE_MIRROR;
		break;
	case WL_EOM_MODE_PRESENTATION:
		output_mode = EOM_OUTPUT_MODE_PRESENTATION;
		break;
	default:
		ERR("no mode.");
		break;
	}

	return output_mode;
}

static eom_output_attribute_e
_convert_to_eom_output_attribute(enum wl_eom_attribute eom_attribute)
{
	eom_output_attribute_e output_attribute = EOM_OUTPUT_ATTRIBUTE_NONE;

	switch (eom_attribute) {
	case WL_EOM_ATTRIBUTE_NONE:
		output_attribute = EOM_OUTPUT_ATTRIBUTE_NONE;
		break;
	case WL_EOM_ATTRIBUTE_NORMAL:
		output_attribute = EOM_OUTPUT_ATTRIBUTE_NORMAL;
		break;
	case WL_EOM_ATTRIBUTE_EXCLUSIVE_SHARED:
		output_attribute = EOM_OUTPUT_ATTRIBUTE_EXCLUSIVE_SHARE;
		break;
	case WL_EOM_ATTRIBUTE_EXCLUSIVE:
		output_attribute = EOM_OUTPUT_ATTRIBUTE_EXCLUSIVE;
		break;
	default:
		ERR("no attribute.");
		break;
	}

	return output_attribute;
}

static eom_output_attribute_state_e
_convert_to_eom_output_attribute_state(
		enum wl_eom_attribute_state eom_attribute_state)
{
	eom_output_attribute_state_e output_attribute_state =
		EOM_OUTPUT_ATTRIBUTE_STATE_NONE;

	switch (eom_attribute_state) {
	case WL_EOM_ATTRIBUTE_STATE_NONE:
		output_attribute_state = EOM_OUTPUT_ATTRIBUTE_STATE_NONE;
		break;
	case WL_EOM_ATTRIBUTE_STATE_ACTIVE:
		output_attribute_state = EOM_OUTPUT_ATTRIBUTE_STATE_ACTIVE;
		break;
	case WL_EOM_ATTRIBUTE_STATE_INACTIVE:
		output_attribute_state = EOM_OUTPUT_ATTRIBUTE_STATE_INACTIVE;
		break;
	case WL_EOM_ATTRIBUTE_STATE_LOST:
		output_attribute_state = EOM_OUTPUT_ATTRIBUTE_STATE_LOST;
		break;
	default:
		ERR("no attribute state.");
		break;
	}

	return output_attribute_state;
}

static enum wl_eom_attribute
_convert_to_wl_eom_attribute(eom_output_attribute_e attr)
{
	enum wl_eom_attribute eom_attribute = WL_EOM_ATTRIBUTE_NONE;

	switch (attr) {
	case EOM_OUTPUT_ATTRIBUTE_NONE:
		eom_attribute = WL_EOM_ATTRIBUTE_NONE;
		break;
	case EOM_OUTPUT_ATTRIBUTE_NORMAL:
		eom_attribute = WL_EOM_ATTRIBUTE_NORMAL;
		break;
	case EOM_OUTPUT_ATTRIBUTE_EXCLUSIVE_SHARE:
		eom_attribute = WL_EOM_ATTRIBUTE_EXCLUSIVE_SHARED;
		break;
	case EOM_OUTPUT_ATTRIBUTE_EXCLUSIVE:
		eom_attribute = WL_EOM_ATTRIBUTE_EXCLUSIVE;
		break;
	default:
		ERR("no wl attribute.");
		break;
	}

	return eom_attribute;
}


static void
_eom_wayland_client_call_notify(EomWaylandOutput *eom_wl_output,
		eom_output_notify_type_e type)
{
	GArray *array = NULL;
	GValue v = G_VALUE_INIT;

	array = g_array_new(FALSE, FALSE, sizeof(GValue));

	/* 0: notify_type */
	g_value_init(&v, G_TYPE_INT);
	g_value_set_int(&v, type);
	array = g_array_append_val(array, v);
	g_value_unset(&v);

	/* 1:output_id */
	g_value_init(&v, G_TYPE_INT);
	g_value_set_int(&v, eom_wl_output->id);
	array = g_array_append_val(array, v);
	g_value_unset(&v);

	/* 2:output_type */
	g_value_init(&v, G_TYPE_INT);
	g_value_set_int(&v,
		_convert_to_eom_output_type(eom_wl_output->eom_type));
	array = g_array_append_val(array, v);
	g_value_unset(&v);

	/* 3:output_mode */
	g_value_init(&v, G_TYPE_INT);
	g_value_set_int(&v,
		_convert_to_eom_output_mode(eom_wl_output->eom_mode));
	array = g_array_append_val(array, v);
	g_value_unset(&v);

	/* 4:w */
	g_value_init(&v, G_TYPE_INT);
	g_value_set_int(&v, eom_wl_output->width);
	array = g_array_append_val(array, v);
	g_value_unset(&v);

	/* 5:h */
	g_value_init(&v, G_TYPE_INT);
	g_value_set_int(&v, eom_wl_output->height);
	array = g_array_append_val(array, v);
	g_value_unset(&v);

	/* 6:w_mm */
	g_value_init(&v, G_TYPE_INT);
	g_value_set_int(&v, eom_wl_output->physical_width);
	array = g_array_append_val(array, v);
	g_value_unset(&v);

	/* 7:h_mm */
	g_value_init(&v, G_TYPE_INT);
	g_value_set_int(&v, eom_wl_output->physical_height);
	array = g_array_append_val(array, v);
	g_value_unset(&v);

	/* 8:pid */
	g_value_init(&v, G_TYPE_INT);
	g_value_set_int(&v, eom_wl_output->physical_width);
	array = g_array_append_val(array, v);
	g_value_unset(&v);

	/* 9:attri */
	g_value_init(&v, G_TYPE_INT);
	g_value_set_int(&v,
		_convert_to_eom_output_attribute(eom_wl_output->eom_attribute));
	array = g_array_append_val(array, v);
	g_value_unset(&v);

	/* 10:state */
	g_value_init(&v, G_TYPE_INT);
	g_value_set_int(&v,
		_convert_to_eom_output_attribute_state(
			eom_wl_output->eom_attribute_state));
	array = g_array_append_val(array, v);
	g_value_unset(&v);

	if (eom_wl_output->client_info && eom_wl_output->client_info->func)
		eom_wl_output->client_info->func(NULL, array);

	if (array)
		g_array_free(array, FALSE);
}


static EomWaylandOutput *
_eom_wayland_client_find_output_from_wl_output(
		struct wl_list *eom_wl_output_list, int output_id)
{
	EomWaylandOutput *eom_wl_output = NULL;
	EomWaylandOutput *tmp = NULL;
	EomWaylandOutput *ret = NULL;

	if (!wl_list_empty(eom_wl_output_list)) {
		wl_list_for_each_safe(eom_wl_output,
			tmp, eom_wl_output_list, link) {
			if (eom_wl_output->id == output_id) {
				ret = eom_wl_output;
				break;
			}
		}
	}

	return ret;
}

static EomWaylandOutput *
_eom_wayland_client_find_output_from_eom_output(
		struct wl_list *eom_wl_output_list, eom_output_id id)
{
	EomWaylandOutput *eom_wl_output = NULL;
	EomWaylandOutput *tmp = NULL;
	EomWaylandOutput *ret = NULL;

	if (!wl_list_empty(eom_wl_output_list)) {
		wl_list_for_each_safe(eom_wl_output,
			tmp, eom_wl_output_list, link) {
			if (eom_wl_output->id == id) {
				ret = eom_wl_output;
				break;
			}
		}
	}

	return ret;
}


static void
_eom_wl_output_handle_geometry(void *data,
			struct wl_output *wl_output,
			int32_t x,
			int32_t y,
			int32_t physical_width,
			int32_t physical_height,
			int32_t subpixel,
			const char *make,
			const char *model,
			int32_t transform)
{
	EomWaylandOutput *eom_wl_output = (EomWaylandOutput *) data;

	INFO("wl_output:%p x:%d y:%d phy(w:%d h:%d) p:%d m:%s model:%s t:%d\n",
		wl_output, x, y, physical_width, physical_height,
		subpixel, make, model, transform);

	/* save vaules if it is different before */
	if (eom_wl_output->x != x)
		eom_wl_output->x = x;
	if (eom_wl_output->y != y)
		eom_wl_output->y = y;
	if (eom_wl_output->physical_height)
		eom_wl_output->physical_height = physical_height;
	if (eom_wl_output->physical_width)
		eom_wl_output->physical_width = physical_width;
	if (eom_wl_output->subpixel)
		eom_wl_output->subpixel = subpixel;
	if (eom_wl_output->transform)
		eom_wl_output->transform = transform;

}


static void
_eom_wl_output_handle_mode(void *data,
			struct wl_output *wl_output,
			uint32_t flags,
			int32_t width,
			int32_t height,
			int32_t refresh)
{
	EomWaylandOutput *eom_wl_output = (EomWaylandOutput *) data;

	INFO("wl_output:%p flags:%d width:%d height:%d refresh:%d\n",
		wl_output, flags, width, height, refresh);

	/* save vaules if it is different before */
	if (eom_wl_output->flags != flags)
		eom_wl_output->flags = flags;
	if (eom_wl_output->width != width)
		eom_wl_output->width = width;
	if (eom_wl_output->height != height)
		eom_wl_output->height = height;
	if (eom_wl_output->refresh != refresh)
		eom_wl_output->refresh = refresh;
}

static void
_eom_wl_output_handle_done(void *data,
			struct wl_output *wl_output)
{
	INFO("wl_output:%p\n", wl_output);
}

static void
_eom_wl_output_handle_scale(void *data,
			struct wl_output *wl_output,
			int32_t factor)
{
	EomWaylandOutput *eom_wl_output = (EomWaylandOutput *) data;

	INFO("wl_output:%p factor:%d\n", wl_output, factor);

	/* save vaules if it is different before */
	if (eom_wl_output->factor != factor)
		eom_wl_output->factor = factor;
}

static const struct wl_output_listener eom_wl_output_listener = {
	_eom_wl_output_handle_geometry,
	_eom_wl_output_handle_mode,
	_eom_wl_output_handle_done,
	_eom_wl_output_handle_scale,
};

static void
_eom_wl_eom_output_count(void *data,
			struct wl_eom *wl_eom,
			uint32_t count)
{
	EomWaylandClientInfo *eom_client_info = (EomWaylandClientInfo *)data;
	/*EomWaylandOutput *eom_wl_output = NULL;*/

	INFO("COUNT - %d", count);

	if (count == 0) {
		INFO("not support external output");
		return;
	}

	eom_client_info->num_outputs = count;
}

static void
_eom_wl_eom_output_info(void *data,
			struct wl_eom *wl_eom,
			uint32_t output_id,
			uint32_t type,
			uint32_t mode,
			uint32_t w,
			uint32_t h,
			uint32_t w_mm,
			uint32_t h_mm,
			uint32_t connection)
{
	EomWaylandClientInfo *eom_client_info = (EomWaylandClientInfo *) data;
	EomWaylandOutput *eom_wl_output = NULL;

	INFO("INFO - id : %d, type : %d, mode : %d, w : %d, h : %d, w_mm : %d, h_mm : %d, conn : %d\n",
		output_id, type, mode, w, h, w_mm, h_mm, connection);

	/* make external output info */
	eom_wl_output = calloc(1, sizeof(EomWaylandOutput));
	if (!eom_wl_output) {
		ERR("Fail to allocate the eom_output.\n");
		return;
	}

	eom_wl_output->id = output_id;
	eom_wl_output->eom_type = type;
	eom_wl_output->eom_mode = mode;
	eom_wl_output->width = w;
	eom_wl_output->height = h;
	eom_wl_output->physical_width = w_mm;
	eom_wl_output->physical_height = h_mm;
	eom_wl_output->eom_status = connection;

	wl_list_insert(&eom_client_info->eom_wl_output_list, &eom_wl_output->link);	
}

static void
_eom_wl_eom_output_type(void *data,
			struct wl_eom *wl_eom,
			uint32_t output_id,
			uint32_t type,
			uint32_t status)
{
	EomWaylandClientInfo *eom_client_info = (EomWaylandClientInfo *)data;
	EomWaylandOutput *eom_wl_output = NULL;

	INFO("TYPE - id : %d, type : %d, status : %d\n", output_id, type, status);

	eom_wl_output = _eom_wayland_client_find_output_from_wl_output(
		&eom_client_info->eom_wl_output_list, output_id);
	RET_IF_FAIL(eom_wl_output != NULL);

	/* save the output type */
	if (eom_wl_output->eom_type != type)
		eom_wl_output->eom_type = type;

	/* check the connection status and call the notify */
	if (eom_wl_output->eom_status != status) {
		eom_wl_output->eom_status = status;

		if (status == WL_EOM_STATUS_CONNECTION)
			_eom_wayland_client_call_notify(eom_wl_output,
				EOM_OUTPUT_NOTIFY_ADD);
		else if (status == WL_EOM_STATUS_DISCONNECTION)
			_eom_wayland_client_call_notify(eom_wl_output,
				EOM_OUTPUT_NOTIFY_REMOVE);
	}
}

static void
_eom_wl_eom_output_mode(void *data,
			struct wl_eom *wl_eom,
			uint32_t output_id,
			uint32_t mode)
{
	EomWaylandClientInfo *eom_client_info = (EomWaylandClientInfo *)data;
	EomWaylandOutput *eom_wl_output = NULL;

	INFO("MODE - id : %d, mode : %d\n", output_id, mode);

	eom_wl_output = _eom_wayland_client_find_output_from_wl_output(
		&eom_client_info->eom_wl_output_list, output_id);
	RET_IF_FAIL(eom_wl_output != NULL);

	/* check the eom mode and call the notify */
	if (eom_wl_output->eom_mode != mode) {
		eom_wl_output->eom_mode = mode;

		_eom_wayland_client_call_notify(eom_wl_output,
			EOM_OUTPUT_NOTIFY_MODE_CHANGED);
	}
}

static void
_eom_wl_eom_output_attribute(void *data,
			 struct wl_eom *wl_eom,
			 uint32_t output_id,
			 uint32_t attribute,
			 uint32_t attribute_state,
			 uint32_t error)
{
	EomWaylandClientInfo *eom_client_info = (EomWaylandClientInfo *)data;
	EomWaylandOutput *eom_wl_output = NULL;

	INFO("ATTRIBUTE - id : %d, attribute : %d, state : %d\n", output_id, attribute, attribute_state);

	eom_wl_output = _eom_wayland_client_find_output_from_wl_output(
		&eom_client_info->eom_wl_output_list, output_id);
	RET_IF_FAIL(eom_wl_output != NULL);

	/* check the eom attribute and call the notify */
	if ((eom_wl_output->eom_attribute != attribute) ||
		(eom_wl_output->eom_attribute_state != attribute_state)) {
		eom_wl_output->eom_attribute = attribute;
		eom_wl_output->eom_attribute_state = attribute_state;

		_eom_wayland_client_call_notify(eom_wl_output,
			EOM_OUTPUT_NOTIFY_ATTRIBUTE_CHANGED);
	}
}


static const struct wl_eom_listener eom_wl_eom_listener = {
	_eom_wl_eom_output_count,
	_eom_wl_eom_output_info,
	_eom_wl_eom_output_type,
	_eom_wl_eom_output_mode,
	_eom_wl_eom_output_attribute,
};


static void
_eom_wl_registry_handle_global(void *data, struct wl_registry *registry,
		uint32_t name, const char *interface, uint32_t version)
{
	EomWaylandClientInfo *ci = (EomWaylandClientInfo *)data;
	/*EomWaylandOutput *eom_wl_output = NULL;*/
	struct wl_output *output = NULL;
	struct wl_eom *eom = NULL;

	if (strcmp(interface, "wl_output") == 0) {
		output = wl_registry_bind(registry, name,
			&wl_output_interface, 1);
		if (!output)
			ERR("Error. fail to bind  %s.\n", interface);
		else {
			INFO("bind %s.\n", interface);
#if 0
			/* create the eom_wl_output */
			eom_wl_output = calloc(1, sizeof(EomWaylandOutput));
			if (!eom_wl_output) {
				ERR("Fail to allocate the eom_output.\n");
				return;
			}
			ci->num_outputs++;
			eom_wl_output->id = ci->num_outputs;
			eom_wl_output->output = output;
			wl_list_insert(&ci->eom_wl_output_list,
				&eom_wl_output->link);

			/* add listener */
			wl_output_add_listener(eom_wl_output->output,
				&eom_wl_output_listener, eom_wl_output);
#endif
		}
	} else if (strcmp(interface, "wl_eom") == 0) {
		eom = wl_registry_bind(registry, name, &wl_eom_interface, 1);
		if (!eom)
			ERR("Error. fail to bind  %s.\n", interface);
		else {
			INFO("bind %s.\n", interface);

			ci->eom = eom;

			/* add listener */
			wl_eom_add_listener(ci->eom, &eom_wl_eom_listener, ci);
		}
	} else
		INFO("Not bind %s.\n", interface);
}

static void
_eom_wl_registry_handle_global_remove(void *data,
		struct wl_registry *registry, uint32_t name)
{

}

static const struct wl_registry_listener eom_registry_listener = {
	_eom_wl_registry_handle_global,
	_eom_wl_registry_handle_global_remove
};

static bool
_eom_wayland_client_initialize()
{
	int ecore_count = -1;

	ecore_count = ecore_wl_init(NULL);
	GOTO_IF_FAIL(ecore_count > 0, fail);

	wl_list_init(&wl_client_info.eom_wl_output_list);

	wl_client_info.display = ecore_wl_display_get();
	GOTO_IF_FAIL(wl_client_info.display != NULL, fail);

	/* get the registry */
	wl_client_info.registry =
		wl_display_get_registry(wl_client_info.display);
	GOTO_IF_FAIL(wl_client_info.registry != NULL, fail);

	/* get the global objects */
	wl_registry_add_listener(wl_client_info.registry,
		&eom_registry_listener, &wl_client_info);
	wl_display_dispatch(wl_client_info.display);
	wl_display_roundtrip(wl_client_info.display);
#if 0
	/* remove type none eom_wl_outputs */
	if (!wl_list_empty(&wl_client_info.eom_wl_output_list)) {
		EomWaylandOutput *eom_wl_output = NULL;
		EomWaylandOutput *tmp = NULL;

		wl_list_for_each_safe(eom_wl_output, tmp,
			&wl_client_info.eom_wl_output_list, link) {
			if (eom_wl_output->eom_type == WL_EOM_TYPE_NONE) {
				WARN("[EOM_CLIENT] eom_type is NONE. remove output:%d\n",
					eom_wl_output->id);
				wl_output_destroy(eom_wl_output->output);
				wl_list_remove(&eom_wl_output->link);
				free(eom_wl_output);
				eom_wl_output = NULL;
			}
		}
	}
#endif
	if (wl_client_info.num_outputs == 0)
		WARN("[EOM_CLIENT] not support external output at this device.\n");

	/* output list */
	if (wl_list_empty(&wl_client_info.eom_wl_output_list))
		WARN("[EOM_CLIENT] no wl output at this device.\n");

	INFO("[EOM_CLIENT] wayland client init.");

	return true;
fail:
	return false;
}

static void
_eom_wayland_client_deinitialize()
{
	EomWaylandOutput *eom_wl_output = NULL;
	EomWaylandOutput *tmp = NULL;

	/* remove all eom_wl_outputs */
	if (!wl_list_empty(&wl_client_info.eom_wl_output_list)) {
		wl_list_for_each_safe(eom_wl_output, tmp,
			&wl_client_info.eom_wl_output_list, link) {
			if (eom_wl_output->output)
				wl_output_destroy(eom_wl_output->output);
			free(eom_wl_output);
			eom_wl_output = NULL;
		}
	}
	ecore_wl_shutdown();

	INFO("[EOM_CLIENT] wayland client deinit.");
}

bool
eom_wayland_client_init(notify_func func)
{
	bool ret = false;

	if (eom_wayland_init)
		return true;

	ret = _eom_wayland_client_initialize();
	GOTO_IF_FAIL(ret != false, fail);

	wl_client_info.func = func;

	eom_wayland_init = 1;

	return ret;
fail:
	return false;
}

void
eom_wayland_client_deinit(GList *cb_info_list)
{
	if (!eom_wayland_init)
		return;

	_eom_wayland_client_deinitialize();

	eom_wayland_init = 0;

	memset(&wl_client_info, 0x0, sizeof(EomWaylandClientInfo));
}

GArray *
eom_wayland_client_get_output_ids(void)
{
	GArray *array = NULL;
	GValue v = G_VALUE_INIT;
	EomWaylandOutput *eom_wl_output = NULL;
	EomWaylandOutput *tmp = NULL;

	if (wl_list_empty(&wl_client_info.eom_wl_output_list)) {
		ERR("error. no outputs.\n");
		return NULL;
	}

	array = g_array_new(FALSE, FALSE, sizeof(GValue));

	wl_list_for_each_safe(eom_wl_output, tmp,
		&wl_client_info.eom_wl_output_list, link) {

		if (eom_wl_output) {
			g_value_init(&v, G_TYPE_INT);
			g_value_set_int(&v, eom_wl_output->id);
			array = g_array_append_val(array, v);
			g_value_unset(&v);
		}
	}

	/* returned array will be freed by caller */
	return array;
}

GArray *
eom_wayland_client_get_output_info(eom_output_id output_id)
{
	GArray *array = NULL;
	GValue v = G_VALUE_INIT;
	EomWaylandOutput *eom_wl_output = NULL;

	eom_wl_output = _eom_wayland_client_find_output_from_eom_output(
		&wl_client_info.eom_wl_output_list, output_id);
	RETV_IF_FAIL(eom_wl_output != NULL, NULL);

	array = g_array_new(FALSE, FALSE, sizeof(GValue));

	/* 0:output_id */
	g_value_init(&v, G_TYPE_INT);
	g_value_set_int(&v, eom_wl_output->id);
	array = g_array_append_val(array, v);
	g_value_unset(&v);

	/* 1:output_type */
	g_value_init(&v, G_TYPE_INT);
	g_value_set_int(&v, eom_wl_output->eom_type);
	array = g_array_append_val(array, v);
	g_value_unset(&v);

	/* 2:output_mode */
	g_value_init(&v, G_TYPE_INT);
	g_value_set_int(&v, eom_wl_output->eom_mode);
	array = g_array_append_val(array, v);
	g_value_unset(&v);

	/* 3:w */
	g_value_init(&v, G_TYPE_INT);
	g_value_set_int(&v, eom_wl_output->width);
	array = g_array_append_val(array, v);
	g_value_unset(&v);

	/* 4:h */
	g_value_init(&v, G_TYPE_INT);
	g_value_set_int(&v, eom_wl_output->height);
	array = g_array_append_val(array, v);
	g_value_unset(&v);

	/* 5:w_mm */
	g_value_init(&v, G_TYPE_INT);
	g_value_set_int(&v, eom_wl_output->physical_width);
	array = g_array_append_val(array, v);
	g_value_unset(&v);

	/* 6:h_mm */
	g_value_init(&v, G_TYPE_INT);
	g_value_set_int(&v, eom_wl_output->physical_height);
	array = g_array_append_val(array, v);
	g_value_unset(&v);

	/* 7:attribute */
	g_value_init(&v, G_TYPE_INT);
	g_value_set_int(&v, eom_wl_output->eom_attribute);
	array = g_array_append_val(array, v);
	g_value_unset(&v);

	/* returned array will be freed by caller */
	return array;
}

GArray *
eom_wayland_client_set_attribute(eom_output_id output_id,
		eom_output_attribute_e attr)
{
	GArray *array = NULL;
	GValue v = G_VALUE_INIT;
	EomWaylandOutput *eom_wl_output = NULL;
	int ret = 0;

	eom_wl_output = _eom_wayland_client_find_output_from_eom_output(
		&wl_client_info.eom_wl_output_list, output_id);
	GOTO_IF_FAIL(eom_wl_output != NULL, fail);

	wl_eom_set_attribute(wl_client_info.eom, eom_wl_output->id,
		_convert_to_wl_eom_attribute(attr));

	/* TODO:
	  * wait for the result of set_attribute.
	  * this should be the blocking call.
	  */
	wl_display_dispatch(wl_client_info.display);
	wl_display_roundtrip(wl_client_info.display);

	ret = 1;

	array = g_array_new(FALSE, FALSE, sizeof(GValue));
	g_value_init(&v, G_TYPE_INT);
	g_value_set_int(&v, ret);
	array = g_array_append_val(array, v);
	g_value_unset(&v);

	/* returned array will be freed by caller */
	return array;
fail:

	return NULL;
}

GArray *
eom_wayland_client_set_window(eom_output_id output_id, Evas_Object *win)
{
	GArray *array = NULL;
	GValue v = G_VALUE_INIT;
	Ecore_Wl_Window *e_wl_win = NULL;
	EomWaylandOutput *eom_wl_output = NULL;
	struct wl_shell_surface *shell_surface = NULL;
	struct xdg_surface *xdg_shell_surface = NULL;
	int ret = 0;

	e_wl_win = elm_win_wl_window_get(win);
	GOTO_IF_FAIL(e_wl_win != NULL, fail);

	eom_wl_output =	_eom_wayland_client_find_output_from_eom_output(
		&wl_client_info.eom_wl_output_list, output_id);
	GOTO_IF_FAIL(eom_wl_output != NULL, fail);

	/* set full screen at output */
	xdg_shell_surface = ecore_wl_window_xdg_surface_get(e_wl_win);
	if (xdg_shell_surface) {
		xdg_surface_set_fullscreen(xdg_shell_surface,
			eom_wl_output->output);
	} else {
		shell_surface = ecore_wl_window_shell_surface_get(e_wl_win);
		if (shell_surface) {
			wl_shell_surface_set_fullscreen(shell_surface,
				WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT,
				0, eom_wl_output->output);
		} else {
			ERR("no wl surface.\n");
			goto fail;
		}
	}

	ret = 1;

	array = g_array_new(FALSE, FALSE, sizeof(GValue));
	g_value_init(&v, G_TYPE_INT);
	g_value_set_int(&v, ret);
	array = g_array_append_val(array, v);
	g_value_unset(&v);

	/* returned array will be freed by caller */
	return array;
fail:

	return NULL;
}

