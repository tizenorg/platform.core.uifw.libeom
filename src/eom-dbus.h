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

#ifndef __EOM_DBUS_H__
#define __EOM_DBUS_H__

#include <stdbool.h>
#include <glib.h>
#include <glib-object.h>
#include "eom.h"

#define EOM_DBUS_SERVER     "org.eom.server"
#define EOM_DBUS_CLIENT     "org.eom.client"
#define EOM_DBUS_INTERFACE  "org.eom.interface"
#define EOM_DBUS_PATH       "/org/eom/path"

#define STR_LEN 128

typedef void (*MethodFunc) (void *data, GValueArray *noti_array);

typedef struct _EomDBusClientMethod {
	char name[STR_LEN];
	MethodFunc func;
	void *data;
	struct _EomDBusClientMethod *next;
} EomDBusClientMethod;

bool eom_dbus_client_connect(void);
void eom_dbus_client_disconnect(void);
GValueArray *eom_dbus_client_send_message(char *method, GValueArray *array);
bool eom_dbus_client_add_method(EomDBusClientMethod *method);
void eom_dbus_client_remove_method(EomDBusClientMethod *method);

#endif /* __EOM_DBUS_H__ */
