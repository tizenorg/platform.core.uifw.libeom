/**************************************************************************

eom (external output manager)

Copyright 2014 Samsung Electronics co., Ltd. All Rights Reserved.

Contact:
SooChan Lim <sc1.lim@samsung.com>
Boram Park <boram1288.park@samsung.com>
Changyeon Lee <cyeon.lee@samsung.com>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

#include <config.h>
#include <dbus/dbus.h>
#include <string.h>
#include "eom.h"
#include "eom-log.h"
#include "eom-dbus.h"

#define REPLY_TIME  1000
#define ARGV_NUM    64

typedef struct _EomDBusClientInfo {
	DBusConnection *conn;
	char name[STR_LEN];
	char rule[STR_LEN];
	GSource *src;
	EomDBusClientMethod *methods;
	int fd;
} EomDBusClientInfo;

static EomDBusClientInfo client_info;

static void _eom_dbus_client_deinit(EomDBusClientInfo *info);

static int
_eom_dbus_need_private_conn(void)
{
	char *env = getenv("EOM_PRIVATE_CONN");

	if (env) {
		return (atoi(env) > 0) ? 1 : 0;
		INFO("EOM_PRIVATE_CONN = %s", env);
	}

	return 0;
}

static int
_eom_dbus_convert_gvalue_to_message(GValueArray *array, DBusMessage *msg)
{
	DBusMessageIter iter;
	int i;

	if (!array)
		return 1;

	if (array->n_values <= 0)
		return 1;

	dbus_message_iter_init_append(msg, &iter);

	INFO("[EOM_CLIENT:%s] n_values(%d)", client_info.name, array->n_values);

	for (i = 0; i < array->n_values; i++) {
		GValue *v = g_value_array_get_nth(array, i);
		GType type = v->g_type;

		INFO("[EOM_CLIENT:%s] type(%d)", client_info.name, (int)type);

		switch (type) {
		case G_TYPE_INT:
			{
				int integer = g_value_get_int(v);

				if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32, &integer)) {
					ERR("[EOM_CLIENT:%s] failed: int append", client_info.name);
					return 0;
				}
			}
			break;
		case G_TYPE_UINT:
			{
				unsigned int uinteger = g_value_get_uint(v);

				if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &uinteger)) {
					ERR("[EOM_CLIENT:%s] failed: uint append", client_info.name);
					return 0;
				}
			}
			break;
		case G_TYPE_STRING:
			{
				char *string = (char *)g_value_get_string(v);

				if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, (void *)&string)) {
					ERR("[EOM_CLIENT:%s] failed: uint append", client_info.name);
					return 0;
				}
			}
			break;
		case G_TYPE_VARIANT:
			{
				GVariant *variant = g_value_get_variant(v);
				int data_size = g_variant_get_size(variant);
				void *data = (void *)g_variant_get_data(variant);
				DBusMessageIter sub;

				RETV_IF_FAIL(data != NULL, 0);
				RETV_IF_FAIL(data_size > 0, 0);

				dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "y", &sub);
				if (!dbus_message_iter_append_fixed_array(&sub, DBUS_TYPE_BYTE, (void *)&data, data_size)) {
					ERR("[EOM_CLIENT:%s] failed: uint append", client_info.name);
					return 0;
				}
				dbus_message_iter_close_container(&iter, &sub);
			}
			break;
		default:
			return 0;
		}
	}

	return 1;
}

static GValueArray*
_eom_dbus_convert_message_to_gvalue(DBusMessage *msg)
{
	GValueArray *array;
	DBusMessageIter iter;

	if (!dbus_message_iter_init(msg, &iter))
		return NULL;

	array = g_value_array_new(0);

	do {
		int type = dbus_message_iter_get_arg_type(&iter);
		GValue v = G_VALUE_INIT;

		INFO("[EOM_CLIENT:%s] type(%c(%d))", client_info.name, (char)type, type);

		switch (type) {
		case DBUS_TYPE_INT32:
			{
				int integer = 0;
				dbus_message_iter_get_basic(&iter, &integer);
				g_value_init(&v, G_TYPE_INT);
				g_value_set_int(&v, integer);
				array = g_value_array_append(array, &v);
				g_value_unset(&v);
			}
			break;
		case DBUS_TYPE_UINT32:
			{
				unsigned int uinteger = 0;
				dbus_message_iter_get_basic(&iter, &uinteger);
				g_value_init(&v, G_TYPE_UINT);
				g_value_set_uint(&v, uinteger);
				array = g_value_array_append(array, &v);
				g_value_unset(&v);
			}
			break;
		case DBUS_TYPE_STRING:
			{
				char *string = NULL;
				dbus_message_iter_get_basic(&iter, &string);
				g_value_init(&v, G_TYPE_STRING);
				g_value_set_string(&v, string);
				array = g_value_array_append(array, &v);
				g_value_unset(&v);
			}
			break;
		default:
			NEVER_GET_HERE();
			g_value_array_free(array);
			return NULL;
		}
	} while (dbus_message_iter_has_next(&iter) && dbus_message_iter_next(&iter));

	return array;
}

static void
_eom_dbus_client_process_message(EomDBusClientInfo *info, DBusMessage *msg)
{
	EomDBusClientMethod **prev;
	DBusError err;

	dbus_error_init(&err);

	INFO("[CLIENT] Process a message (%s.%s)",
		dbus_message_get_interface(msg), dbus_message_get_member(msg));

	RET_IF_FAIL(info->conn != NULL);

	for (prev = &info->methods; *prev; prev = &(*prev)->next) {
		EomDBusClientMethod *method = *prev;

		if (!strcmp(dbus_message_get_member(msg), method->name)) {
			GValueArray *array = _eom_dbus_convert_message_to_gvalue(msg);

			if (method->func)
				method->func(method->data, array);

			if (array)
				g_value_array_free(array);

			dbus_error_free(&err);

			return;
		}
	}
}


gboolean
_eom_dbus_client_cb(GIOChannel *src, GIOCondition cond, gpointer data)
{
	EomDBusClientInfo *info = (EomDBusClientInfo *)data;

	if (!info || !info->conn || info->fd < 0)
		return false;

	do {
		if (info->conn)
			dbus_connection_read_write_dispatch(info->conn, 0);
	} while (info->conn &&
			dbus_connection_get_is_connected(info->conn) &&
			dbus_connection_get_dispatch_status(info->conn) ==
			DBUS_DISPATCH_DATA_REMAINS);

	return true;
}


static DBusHandlerResult
_eom_dbus_client_msg_handler(DBusConnection *connection, DBusMessage *msg, void *data)
{
	EomDBusClientInfo *info = (EomDBusClientInfo *)data;

	if (!info || !info->conn || !msg)
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

	INFO("[Client] Got a message (%s.%s)",
		dbus_message_get_interface(msg), dbus_message_get_member(msg));

	_eom_dbus_client_process_message(info, msg);

	return DBUS_HANDLER_RESULT_HANDLED;
}

static DBusHandlerResult
_eom_dbus_client_msg_filter(DBusConnection *conn, DBusMessage *msg, void *data)
{
	EomDBusClientInfo *info = (EomDBusClientInfo *)data;

	if (!info)
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

	if (dbus_message_is_signal(msg, DBUS_INTERFACE_LOCAL, "Disconnected")) {
		INFO("[EOM] disconnected by signal");
		_eom_dbus_client_deinit(info);

		return DBUS_HANDLER_RESULT_HANDLED;
	}

	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static int
_eom_dbus_client_init(EomDBusClientInfo *info)
{
	DBusError err;
	int ret;
	DBusObjectPathVTable vtable = {.message_function = _eom_dbus_client_msg_handler, };
	GIOChannel *channel;

	dbus_error_init(&err);

	if (_eom_dbus_need_private_conn())
		info->conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
	else
		info->conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);

	if (dbus_error_is_set(&err)) {
		ERR("[EOM] failed: connection (%s)", err.message);
		goto free_err;
	}
	if (!info->conn) {
		ERR("[EOM] failed: connection NULL");
		goto free_err;
	}

	ret = dbus_bus_request_name(info->conn, info->name, DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
	if (dbus_error_is_set(&err)) {
		ERR("[EOM] failed: request name (%s)", err.message);
		goto free_conn;
	}
	if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		ERR("[EOM] failed: Not Primary Owner (%d)", ret);
		goto free_conn;
	}

	snprintf(info->rule, sizeof(info->rule), "interface='%s'", EOM_DBUS_INTERFACE);

	dbus_bus_add_match(info->conn, info->rule, &err);
	dbus_connection_flush(info->conn);
	if (dbus_error_is_set(&err)) {
		ERR("[EOM] failed: add match (%s)", err.message);
		goto free_name;
	}

	if (!dbus_connection_register_object_path(info->conn, EOM_DBUS_PATH, &vtable, info)) {
		ERR("[EOM] failed: register object path");
		goto free_match;
	}

	dbus_connection_set_exit_on_disconnect(info->conn, FALSE);

	if (!dbus_connection_add_filter(info->conn, _eom_dbus_client_msg_filter, info, NULL)) {
		ERR("[EOM] failed: add filter (%s)", err.message);
		goto free_register;
	}

	if (!dbus_connection_get_unix_fd(info->conn, &info->fd) || info->fd < 0) {
		ERR("[EOM] failed: get fd");
		goto free_filter;
	}

	dbus_error_free(&err);

	channel = g_io_channel_unix_new(info->fd);
	g_io_channel_set_flags(channel, G_IO_FLAG_NONBLOCK, NULL);

	info->src = g_io_create_watch(channel, G_IO_IN);
	g_source_set_callback(info->src, (GSourceFunc)_eom_dbus_client_cb, (gpointer)info, NULL);
	g_source_attach(info->src, NULL);

	g_io_channel_unref(channel);

	INFO("[EOM_CLIENT] connected");

	return 1;

free_filter:
	dbus_connection_remove_filter(info->conn, _eom_dbus_client_msg_filter, info);
free_register:
	dbus_connection_unregister_object_path(info->conn, EOM_DBUS_PATH);
free_match:
	dbus_bus_remove_match(info->conn, info->rule, &err);
	dbus_error_free(&err);
free_name:
	dbus_bus_release_name(info->conn, info->name, &err);
	dbus_error_free(&err);
free_conn:
	dbus_connection_close(info->conn);
free_err:
	dbus_error_free(&err);
	info->conn = NULL;
	info->fd = -1;

	return 0;
}

static void
_eom_dbus_client_deinit(EomDBusClientInfo *info)
{
	DBusError err;

	if (!info->conn)
		return;

	if (info->src) {
		g_source_destroy(info->src);
		g_source_unref(info->src);
	}

	if (info->conn) {
		dbus_error_init(&err);
		dbus_bus_remove_match(info->conn, info->rule, &err);
		dbus_error_free(&err);
		dbus_bus_release_name(info->conn, info->name, &err);
		dbus_error_free(&err);
		dbus_connection_unref(info->conn);
	}

	memset(info, 0, sizeof(EomDBusClientInfo));
	info->fd = -1;

	INFO("[EOM] disconnected");
}

bool
eom_dbus_client_connect(void)
{
	if (client_info.conn)
		return true;

	snprintf(client_info.name, STR_LEN, "org.eom.client%d", getpid());

	client_info.fd = -1;

	if (!_eom_dbus_client_init(&client_info))
		return false;

	return true;
}

void
eom_dbus_client_disconnect(void)
{
	_eom_dbus_client_deinit(&client_info);
}

bool
eom_dbus_client_add_method(EomDBusClientMethod *method)
{
	EomDBusClientMethod **prev;

	for (prev = &client_info.methods; *prev; prev = &(*prev)->next);

	method->next = NULL;
	*prev = method;

	return true;
}

void
eom_dbus_client_remove_method(EomDBusClientMethod *method)
{
	EomDBusClientMethod **prev;

	for (prev = &client_info.methods; *prev; prev = &(*prev)->next)
		if (*prev == method) {
			*prev = method->next;
			method->next = NULL;
			break;
		}
}

GValueArray*
eom_dbus_client_send_message(char *method, GValueArray *array)
{
	DBusMessage *msg = NULL;
	DBusMessage *reply_msg = NULL;
	GValueArray *ret_array = NULL;
	DBusError err;

	RETV_IF_FAIL(client_info.conn != NULL, NULL);

	dbus_error_init(&err);

	msg = dbus_message_new_method_call(EOM_DBUS_SERVER, EOM_DBUS_PATH, EOM_DBUS_INTERFACE, method);
	GOTO_IF_FAIL(msg != NULL, err_send);

	INFO("[EOM_CLIENT:%s] Send message(%s)", client_info.name, method);

	if (!_eom_dbus_convert_gvalue_to_message(array, msg)) {
		ERR("[EOM_CLIENT:%s] failed: gvalue_to_message", client_info.name);
		goto err_send;
	}

	reply_msg = dbus_connection_send_with_reply_and_block(client_info.conn, msg, REPLY_TIME, &err);
	if (dbus_error_is_set(&err)) {
		ERR("[EOM_CLIENT:%s] failed: send (%s)", client_info.name, err.message);
		goto err_send;
	}
	GOTO_IF_FAIL(reply_msg != NULL, err_send);

	INFO("[EOM_CLIENT:%s] Got reply", client_info.name);

	ret_array = _eom_dbus_convert_message_to_gvalue(reply_msg);

	dbus_message_unref(msg);
	dbus_message_unref(reply_msg);
	dbus_error_free(&err);

	return ret_array;
err_send:
	if (msg)
		dbus_message_unref(msg);
	if (reply_msg)
		dbus_message_unref(reply_msg);

	dbus_error_free(&err);

	return NULL;
}