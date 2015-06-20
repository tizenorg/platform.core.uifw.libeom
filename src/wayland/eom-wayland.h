/**************************************************************************

eom (external output manager)

Copyright 2015 Samsung Electronics co., Ltd. All Rights Reserved.

Contact:
SooChan Lim <sc1.lim@samsung.com>

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

#ifndef __EOM_WAYLAND_H__
#define __EOM_WAYLAND_H__

#include <stdbool.h>
#include <glib.h>
#include <glib-object.h>
#include "eom.h"
#include "eom-private.h"

bool eom_wayland_client_init(notify_func func);
void eom_wayland_client_deinit(GList *cb_info_list);

GArray *eom_wayland_client_get_output_ids(void);
GArray *eom_wayland_client_get_output_info(eom_output_id output_id);
GArray *eom_wayland_client_set_attribute(eom_output_id output_id, eom_output_attribute_e attry);
GArray *eom_wayland_client_set_window(eom_output_id output_id, Evas_Object *win);


#endif /* __EOM_WAYLAND_H__ */

