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

#include <glib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "eom.h"
#include "eom_internal.h"
#include "eom-log.h"
#include "eom-dbus.h"
#include "eom-private.h"

typedef struct
{
    eom_output_notify_type_e type;         /**< External output type */
    eom_output_id            output_id;    /**< External output id */
} eom_output_notify_base_s;

typedef struct
{
    eom_output_notify_type_e type;         /**< External output type */
    eom_output_id            output_id;    /**< External output id */
} eom_output_notify_add_s;

typedef struct
{
    eom_output_notify_type_e type;         /**< External output type */
    eom_output_id            output_id;    /**< External output id */
} eom_output_notify_remove_s;

typedef struct
{
    eom_output_notify_type_e type;         /**< External output type */
    eom_output_id            output_id;    /**< External output id */
    eom_output_mode_e        old_mode;     /**< External output mode */
    eom_output_mode_e        new_mode;     /**< External output mode */
} eom_output_notify_mode_change_s;

typedef struct
{
    eom_output_notify_type_e     type;         /**< External output type */
    eom_output_id                output_id;    /**< External output id */
    eom_output_attribute_e       attribute;    /**< External output attribute */
    eom_output_attribute_state_e attr_state;   /**< External output attribute state */
} eom_output_notify_attribute_change_s;

typedef union
{
    eom_output_notify_base_s             base;             /**< Base structure for eom notify */
    eom_output_notify_add_s              output_add;       /**< #EOM_OUTPUT_NOTIFY_ADD notify */
    eom_output_notify_remove_s           output_remove;    /**< #EOM_OUTPUT_NOTIFY_REMOVE notify */
    eom_output_notify_mode_change_s      mode_change;      /**< #EOM_OUTPUT_NOTIFY_MODE_CHANGED notify */
    eom_output_notify_attribute_change_s attr_change;      /**< #EOM_OUTPUT_NOTIFY_ATTRIBUTE_CHANGED notify */
} eom_output_notify_s;

typedef struct
{
    eom_output_notify_type_e type;
    eom_output_added_cb add_func;
    eom_output_removed_cb remove_func;
    eom_mode_changed_cb mode_change_func;
    eom_attribute_changed_cb attribute_change_func;
    void        *user_data;
} eom_output_notify_cb_info;

bool eom_debug_on;

static pthread_mutex_t     eom_lock;

static bool                dbus_initialized;
static EomDBusClientMethod dbus_method;

static GList  *cb_info_list;
static GList  *output_info_list;

static void _eom_output_process_notify_cb (void *data, GValueArray *array);
static eom_output_info* _eom_find_output_info (eom_output_id output_id);
static eom_output_info* _eom_alloc_output_info (int output_id, int output_type);
static const char*
TYPE (eom_output_type_e output_type)
{
    switch (output_type)
    {
    case EOM_OUTPUT_TYPE_VGA:
        return "VGA";
    case EOM_OUTPUT_TYPE_DVII:
        return "DVII";
    case EOM_OUTPUT_TYPE_DVID:
        return "DVID";
    case EOM_OUTPUT_TYPE_DVIA:
        return "DVIA";
    case EOM_OUTPUT_TYPE_COMPOSITE:
        return "COMPOSITE";
    case EOM_OUTPUT_TYPE_SVIDEO:
        return "SVIDEO";
    case EOM_OUTPUT_TYPE_LVDS:
        return "LVDS";
    case EOM_OUTPUT_TYPE_COMPONENT:
        return "COMPONENT";
    case EOM_OUTPUT_TYPE_9PINDIN:
        return "9PINDIN";
    case EOM_OUTPUT_TYPE_DISPLAYPORT:
        return "DISPLAYPORT";
    case EOM_OUTPUT_TYPE_HDMIA:
        return "HDMIA";
    case EOM_OUTPUT_TYPE_HDMIB:
        return "HDMIB";
    case EOM_OUTPUT_TYPE_TV:
        return "TV";
    case EOM_OUTPUT_TYPE_EDP:
        return "EDP";
    case EOM_OUTPUT_TYPE_VIRTUAL:
        return "VIRTUAL";
    case EOM_OUTPUT_TYPE_DSI:
        return "DSI";
    default:
        return "UNKNOWN";
    }
    return "UNKNOWN";
}

static void
_eom_get_debug_evn (void)
{
    char *env = getenv("EOM_DEBUG");

    if(env)
    {
        eom_debug_on = (atoi(env) > 0) ? true : false;
#ifdef HAVE_DLOG
        fprintf (stderr, "eom dlog on\n");
#else
        fprintf (stderr, "eom dlog off\n");
#endif
        INFO ("EOM_DEBUG = %s", env);
    }
    else
        eom_debug_on = 0;
}

static bool
_eom_mutex_init (void)
{
    static bool init = false;

    if (init)
        return true;

    if (pthread_mutex_init (&eom_lock, NULL))
    {
        ERRNO ("fail: mutex init");
        return false;
    }

    init = true;

    return true;
}

void
_eom_mutex_lock (void)
{
    if (!_eom_mutex_init ())
        return;

    pthread_mutex_lock (&eom_lock);
}

void
_eom_mutex_unlock (void)
{
    pthread_mutex_unlock (&eom_lock);
}

static bool
_eom_dbus_init (void)
{
    if (dbus_initialized)
        return true;

    if (!eom_dbus_client_connect ())
        return false;

    snprintf (dbus_method.name, sizeof (dbus_method.name), "%s", "Notify");
    dbus_method.func = _eom_output_process_notify_cb;
    dbus_method.data = NULL;
    eom_dbus_client_add_method (&dbus_method);

    dbus_initialized = true;

    INFO ("dbus init");

    return true;
}

static void
_eom_dbus_deinit (void)
{
    if (!dbus_initialized)
        return;

    /* An output instance and a callback can be created and be added only by user.
     * If there is cb_info_list, it means that user is still
     * watching and interested with eom dbus message.
     */
    if (cb_info_list)
        return;

    eom_dbus_client_remove_method (&dbus_method);
    eom_dbus_client_disconnect ();

    dbus_initialized = false;
}

static void
_eom_set_output_info_mode (eom_output_info *output_info, int output_mode)
{
    RET_IF_FAIL (output_info != NULL);

    if (output_info->output_mode == output_mode)
        return;

    output_info->output_mode = output_mode;
}

static void
_eom_set_output_attribute (eom_output_info *output_info, eom_output_attribute_e attribute)
{
    RET_IF_FAIL (output_info != NULL);

    output_info->attribute = attribute;
}

static void
_eom_set_output_attribute_state (eom_output_info *output_info, eom_output_attribute_state_e state)
{
    RET_IF_FAIL (output_info != NULL);

    output_info->state = state;
}

static void
_eom_set_output_info_size (eom_output_info *output_info, int w, int h)
{
    RET_IF_FAIL (output_info != NULL);

    output_info->width = w;
    output_info->height = h;
}

static void
_eom_set_output_info_phy_size (eom_output_info *output_info, int w_mm, int h_mm)
{
    RET_IF_FAIL (output_info != NULL);

    output_info->mm_width = w_mm;
    output_info->mm_height = h_mm;
}

static void
_eom_reset_output_info (eom_output_info *output_info)
{
    RET_IF_FAIL (output_info != NULL);

    output_info->output_mode = EOM_OUTPUT_MODE_NONE;
    output_info->width = 0;
    output_info->height = 0;
    output_info->mm_width = 0;
    output_info->mm_height = 0;
}

static void
_eom_output_call_notify_cb (eom_output_notify_s *notify)
{
    GList *l;

    for (l = cb_info_list; l; l = g_list_next (l))
    {
        eom_output_notify_cb_info *cb_info = (eom_output_notify_cb_info*)l->data;

        if (!cb_info || cb_info->type != notify->base.type)
            continue;

        if (notify->base.type == EOM_OUTPUT_NOTIFY_ADD)
        {
            if (cb_info->add_func == NULL)
                continue;

            INFO ("cb_info: type(%d) output_id(%d)", notify->base.type, notify->base.output_id);

            cb_info->add_func (notify->base.output_id, cb_info->user_data);
        }
        else if (notify->base.type == EOM_OUTPUT_NOTIFY_REMOVE)
        {
            if (cb_info->remove_func == NULL)
                continue;

            INFO ("cb_info: type(%d) output_id(%d)", notify->base.type, notify->base.output_id);

            cb_info->remove_func (notify->base.output_id, cb_info->user_data);
        }
        else if (notify->base.type == EOM_OUTPUT_NOTIFY_MODE_CHANGED)
        {
            if (cb_info->mode_change_func == NULL)
                continue;

            INFO ("cb_info: type(%d) output_id(%d)", notify->base.type, notify->base.output_id);

            cb_info->mode_change_func (notify->base.output_id, cb_info->user_data);
        }
        else if (notify->base.type == EOM_OUTPUT_NOTIFY_ATTRIBUTE_CHANGED)
        {
            if (cb_info->attribute_change_func == NULL)
                continue;

            INFO ("cb_info: type(%d) output_id(%d)", notify->base.type, notify->base.output_id);

            cb_info->attribute_change_func (notify->base.output_id, cb_info->user_data);
        }
        else
        {
            INFO ("cb_info: type(%d) output_id(%d)", notify->base.type, notify->base.output_id);
            continue;
        }
    }
}

static void
_eom_output_process_notify_cb (void *data, GValueArray *array)
{
    eom_output_notify_s notify;
    eom_output_info *output_info;
    int notify_type, output_id, output_type, output_mode, w, h, w_mm, h_mm, pid, attr, state;
    GValue *v;

    RET_IF_FAIL (array != NULL);
    RET_IF_FAIL (array->n_values == 11);

    /* 11 args 0: notify_type 1:output_id, 2:output_type, 3:output_mode, 4:w, 5:h, 6:w_mm, 7:h_mm, 8:pid, 9:attri, 10:state */
    v = g_value_array_get_nth (array, 0);
    notify_type = g_value_get_int (v);
    v = g_value_array_get_nth (array, 1);
    output_id = g_value_get_int (v);
    v = g_value_array_get_nth (array, 2);
    output_type = g_value_get_int (v);
    v = g_value_array_get_nth (array, 3);
    output_mode = g_value_get_int (v);
    v = g_value_array_get_nth (array, 4);
    w = g_value_get_int (v);
    v = g_value_array_get_nth (array, 5);
    h = g_value_get_int (v);
    v = g_value_array_get_nth (array, 6);
    w_mm = g_value_get_int (v);
    v = g_value_array_get_nth (array, 7);
    h_mm = g_value_get_int (v);
    v = g_value_array_get_nth (array, 8);
    pid = g_value_get_int (v);
    v = g_value_array_get_nth (array, 9);
    attr = g_value_get_int (v);
    v = g_value_array_get_nth (array, 10);
    state = g_value_get_int (v);

    INFO ("notify: %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
          notify_type, output_id, output_type, output_mode, w, h, w_mm, h_mm, pid, attr, state);

    memset (&notify, 0, sizeof (eom_output_notify_s));
    notify.base.type = notify_type;
    notify.base.output_id = output_id;

    _eom_mutex_lock ();

    output_info = _eom_find_output_info (output_id);
    GOTO_IF_FAIL (output_info != NULL, exit);

    switch (notify_type)
    {
    case EOM_OUTPUT_NOTIFY_ADD:
        {
            INFO ("'%s(%d)' added", TYPE(output_type), output_id);

            _eom_set_output_info_mode (output_info, output_mode);
            _eom_set_output_info_size (output_info, w, h);
            _eom_set_output_info_phy_size (output_info, w_mm, h_mm);

            _eom_mutex_unlock ();
            _eom_output_call_notify_cb (&notify);
            _eom_mutex_lock ();
        }
        break;

    case EOM_OUTPUT_NOTIFY_REMOVE:
        {
            INFO ("'%s(%d)' removed", TYPE(output_type), output_id);
            _eom_reset_output_info (output_info);

            _eom_mutex_unlock ();
            _eom_output_call_notify_cb (&notify);
            _eom_mutex_lock ();
        }
        break;

    case EOM_OUTPUT_NOTIFY_MODE_CHANGED:
        {
            eom_output_mode_e old_mode;

            old_mode = output_info->output_mode;
            output_info->output_mode = output_mode;

            INFO ("'%s(%d)' mode changed(%d=>%d)",
                    TYPE(output_type), output_id, old_mode, output_info->output_mode);

            notify.mode_change.old_mode = old_mode;
            notify.mode_change.new_mode = output_info->output_mode;

            _eom_mutex_unlock ();
            _eom_output_call_notify_cb (&notify);
            _eom_mutex_lock ();
        }
        break;

    case EOM_OUTPUT_NOTIFY_ATTRIBUTE_CHANGED:
        {
            int current_pid = getpid();

            if (current_pid == pid)
            {
                INFO ("'%s(%d)'pid(%d)", TYPE(output_type), output_id, pid);
                _eom_set_output_attribute(output_info, attr);
                _eom_set_output_attribute_state(output_info, state);
                notify.attr_change.output_id = output_id;
                notify.attr_change.attribute = attr;
                notify.attr_change.attr_state = state;

                _eom_mutex_unlock ();
                _eom_output_call_notify_cb (&notify);
                _eom_mutex_lock ();
            }
        }
        break;

    default:
        INFO ("'unknown(%d)' notified", notify.base.type);

        _eom_mutex_unlock ();
        _eom_output_call_notify_cb (&notify);
        _eom_mutex_lock ();
        break;
    }

exit:
    _eom_mutex_unlock ();
}


static eom_output_info*
_eom_find_output_info (eom_output_id output_id)
{
    GList *l;

    for (l = output_info_list; l; l = g_list_next (l))
    {
        eom_output_info *output_info = (eom_output_info*)l->data;

        if (output_info && (output_id == output_info->id))
            return output_info;
    }

    return NULL;
}

static void
_eom_free_output_info (eom_output_info **output_info)
{
    if (output_info && *output_info)
    {
        free(*output_info);
        *output_info = NULL;
    }
}

static eom_output_info*
_eom_alloc_output_info (int output_id, int output_type)
{
    eom_output_info *output_info = NULL;

    output_info = calloc (1, sizeof (eom_output_info));
    GOTO_IF_FAIL (output_info != NULL, fail);

    output_info->id = output_id;
    output_info->type = output_type;
    output_info->output_mode = EOM_OUTPUT_MODE_NONE;
    output_info->attribute = EOM_OUTPUT_ATTRIBUTE_NONE;
    output_info->state = EOM_OUTPUT_ATTRIBUTE_STATE_NONE;
    output_info->width = 0;
    output_info->height = 0;
    output_info->mm_width = 0;
    output_info->mm_height = 0;

    INFO ("alloc '%s(%d)''s info", TYPE (output_type), output_id);

    return output_info;

fail:
    if (output_info)
        free (output_info);

    ERR ("fail: alloc '%s(%d)''s info", TYPE (output_type), output_id);

    return NULL;
}

API int
eom_init (void)
{
    bool ret;

    _eom_get_debug_evn ();

    _eom_mutex_lock ();

    g_type_init ();

    ret = _eom_dbus_init ();

    _eom_mutex_unlock ();

    INFO ("eom init %s", (ret)?"success":"failed");

    return (ret)?EOM_ERROR_NONE:EOM_ERROR_CONNECTION_FAILURE;
}

API void
eom_deinit (void)
{
    GList *l;

    _eom_mutex_lock ();
    _eom_dbus_deinit ();

    // TODO: redesign the life-cycle of output_infos
    //destory output_info.
    for (l = output_info_list; l; l = g_list_next (l))
    {
        eom_output_info *output_info = (eom_output_info*)l->data;
        output_info_list = g_list_remove (output_info_list, output_info);

        _eom_free_output_info(&output_info);
    }

    _eom_mutex_unlock ();

    INFO ("eom deinit");
}

API eom_output_id*
eom_get_eom_output_ids (int *count)
{
    GValueArray *ret_array = NULL;
    eom_output_id *output_ids = NULL;
    int i;

    if (!count)
    {
        set_last_result (EOM_ERROR_INVALID_PARAMETER);
        return NULL;
    }

    _eom_mutex_lock ();

    ret_array = eom_dbus_client_send_message ("GetOutputIDs", NULL);

    if (!ret_array)
    {
        *count = 0;
        _eom_mutex_unlock ();
        set_last_result (EOM_ERROR_NONE);
        return NULL;
    }

    if (ret_array->n_values == 0)
    {
        g_value_array_free (ret_array);
        *count = 0;
        _eom_mutex_unlock ();
        set_last_result (EOM_ERROR_NONE);
        return NULL;
    }

    output_ids = calloc (ret_array->n_values, sizeof (eom_output_id));
    GOTO_IF_FAIL (output_ids != NULL, fail);

    *count = ret_array->n_values;
    for (i = 0; i < ret_array->n_values; i++)
    {
        GValue *v = g_value_array_get_nth (ret_array, i);
        output_ids[i] = g_value_get_int (v);
        INFO ("output_ids: %d", output_ids[i]);
    }

    g_value_array_free (ret_array);
    ret_array = NULL;

    // TODO: redesign the life-cycle of output_infos
    for (i = 0; i < *count; i++)
    {
        /* add output_info to output_info_list */
        eom_output_info *output_info;
        eom_output_id output_id = output_ids[i];
        GValueArray *msg_array;
        GValue v = G_VALUE_INIT;

        output_info = _eom_find_output_info (output_id);
        if (output_info)
            continue;

        msg_array = g_value_array_new (0);

        g_value_init (&v, G_TYPE_INT);
        g_value_set_int (&v, output_id);
        msg_array = g_value_array_append (msg_array, &v);

        ret_array = eom_dbus_client_send_message("GetOutputInfo", msg_array);
        g_value_array_free (msg_array);

        if (ret_array)
        {
            /* 0:output_id, 1:output_type, 2:output_mode, 3:w, 4:h, 5:w_mm, 6:h_mm */
            output_info = _eom_alloc_output_info (g_value_get_int(g_value_array_get_nth (ret_array, 0)),
                                                  g_value_get_int(g_value_array_get_nth (ret_array, 1)));
            if (output_info)
            {
                output_info_list = g_list_append (output_info_list, output_info);
                _eom_set_output_info_mode (output_info, g_value_get_int(g_value_array_get_nth (ret_array, 2)));
                _eom_set_output_info_size (output_info, g_value_get_int(g_value_array_get_nth (ret_array, 3)),
                                                        g_value_get_int(g_value_array_get_nth (ret_array, 4)));
                _eom_set_output_info_phy_size (output_info, g_value_get_int(g_value_array_get_nth (ret_array, 5)),
                                                            g_value_get_int(g_value_array_get_nth (ret_array, 6)));
                INFO ("GetOutputInfo: %s(%d)", TYPE (output_info->type), output_info->id);
            }

            g_value_array_free (ret_array);
            ret_array = NULL;
        }
        else
            ERR ("fail: get id(%d)'s information", output_id);
    }

    _eom_mutex_unlock ();

    set_last_result (EOM_ERROR_NONE);

    return output_ids;

fail:
    if (ret_array)
        g_value_array_free (ret_array);

    if (output_ids)
        free (output_ids);

    *count = 0;

    _eom_mutex_unlock ();

    set_last_result (EOM_ERROR_OUT_OF_MEMORY);

    return NULL;
}

API int
eom_set_output_added_cb (eom_output_added_cb callback, void *user_data)
{
    eom_output_notify_cb_info *cb_info;
    GList *l;

    RETV_IF_FAIL (callback != NULL, EOM_ERROR_INVALID_PARAMETER);

    _eom_mutex_lock ();

    for (l = cb_info_list; l; l = g_list_next (l))
    {
        eom_output_notify_cb_info *cb_info = (eom_output_notify_cb_info*)l->data;

        if (cb_info && (cb_info->add_func != NULL) && (cb_info->add_func == callback))
        {
            _eom_mutex_unlock ();
            return EOM_ERROR_NONE;
        }
    }

    cb_info = calloc (1, sizeof (eom_output_notify_cb_info));
    GOTO_IF_FAIL (cb_info != NULL, fail);

    cb_info->type = EOM_OUTPUT_NOTIFY_ADD;
    cb_info->add_func = callback;
    cb_info->user_data = user_data;

    cb_info_list = g_list_append (cb_info_list, cb_info);

    _eom_mutex_unlock ();

    return EOM_ERROR_NONE;

fail:
    _eom_mutex_unlock ();

    return EOM_ERROR_OUT_OF_MEMORY;
}

API int
eom_unset_output_added_cb (eom_output_added_cb callback)
{
    GList *l;

    RETV_IF_FAIL (callback != NULL, EOM_ERROR_INVALID_PARAMETER);

    _eom_mutex_lock ();

    for (l = cb_info_list; l; l = g_list_next (l))
    {
        eom_output_notify_cb_info *cb_info = (eom_output_notify_cb_info*)l->data;

        if (!cb_info || (cb_info->add_func == NULL) || (cb_info->add_func != callback))
            continue;

        cb_info_list = g_list_remove (cb_info_list, cb_info);
        free (cb_info);

        _eom_mutex_unlock ();

        return EOM_ERROR_NONE;
    }

    _eom_mutex_unlock ();

    return EOM_ERROR_INVALID_PARAMETER;
}

API int
eom_set_output_removed_cb (eom_output_removed_cb callback, void *user_data)
{
    eom_output_notify_cb_info *cb_info;
    GList *l;

    RETV_IF_FAIL (callback != NULL, EOM_ERROR_INVALID_PARAMETER);

    _eom_mutex_lock ();

    for (l = cb_info_list; l; l = g_list_next (l))
    {
        eom_output_notify_cb_info *cb_info = (eom_output_notify_cb_info*)l->data;

        if (cb_info && (cb_info->remove_func != NULL) && (cb_info->remove_func == callback))
        {
            _eom_mutex_unlock ();
            return EOM_ERROR_NONE;
        }
    }

    cb_info = calloc (1, sizeof (eom_output_notify_cb_info));
    GOTO_IF_FAIL (cb_info != NULL, fail);

    cb_info->type = EOM_OUTPUT_NOTIFY_REMOVE;
    cb_info->remove_func = callback;
    cb_info->user_data = user_data;

    cb_info_list = g_list_append (cb_info_list, cb_info);

    _eom_mutex_unlock ();

    return EOM_ERROR_NONE;

fail:
    _eom_mutex_unlock ();

    return EOM_ERROR_OUT_OF_MEMORY;
}

API int
eom_unset_output_removed_cb (eom_output_removed_cb callback)
{
    GList *l;

    RETV_IF_FAIL (callback != NULL, EOM_ERROR_INVALID_PARAMETER);

    _eom_mutex_lock ();

    for (l = cb_info_list; l; l = g_list_next (l))
    {
        eom_output_notify_cb_info *cb_info = (eom_output_notify_cb_info*)l->data;

        if (!cb_info || (cb_info->remove_func == NULL) || (cb_info->remove_func != callback))
            continue;

        cb_info_list = g_list_remove (cb_info_list, cb_info);
        free (cb_info);

        _eom_mutex_unlock ();

        return EOM_ERROR_NONE;
    }

    _eom_mutex_unlock ();

    return EOM_ERROR_INVALID_PARAMETER;
}

API int
eom_set_mode_changed_cb (eom_mode_changed_cb callback, void *user_data)
{
    eom_output_notify_cb_info *cb_info;
    GList *l;

    RETV_IF_FAIL (callback != NULL, EOM_ERROR_INVALID_PARAMETER);

    _eom_mutex_lock ();

    for (l = cb_info_list; l; l = g_list_next (l))
    {
        eom_output_notify_cb_info *cb_info = (eom_output_notify_cb_info*)l->data;

        if (cb_info && (cb_info->mode_change_func != NULL) && (cb_info->mode_change_func == callback))
        {
            _eom_mutex_unlock ();
            return EOM_ERROR_NONE;
        }
    }

    cb_info = calloc (1, sizeof (eom_output_notify_cb_info));
    GOTO_IF_FAIL (cb_info != NULL, fail);

    cb_info->type = EOM_OUTPUT_NOTIFY_MODE_CHANGED;
    cb_info->mode_change_func = callback;
    cb_info->user_data = user_data;

    cb_info_list = g_list_append (cb_info_list, cb_info);

    _eom_mutex_unlock ();

    return EOM_ERROR_NONE;

fail:
    _eom_mutex_unlock ();

    return EOM_ERROR_OUT_OF_MEMORY;
}

API int
eom_unset_mode_changed_cb (eom_mode_changed_cb callback)
{
    GList *l;

    RETV_IF_FAIL (callback != NULL, EOM_ERROR_INVALID_PARAMETER);

    _eom_mutex_lock ();

    for (l = cb_info_list; l; l = g_list_next (l))
    {
        eom_output_notify_cb_info *cb_info = (eom_output_notify_cb_info*)l->data;

        if (!cb_info || (cb_info->mode_change_func == NULL) || (cb_info->mode_change_func != callback))
            continue;

        cb_info_list = g_list_remove (cb_info_list, cb_info);
        free (cb_info);

        _eom_mutex_unlock ();

        return EOM_ERROR_NONE;
    }

    _eom_mutex_unlock ();

    return EOM_ERROR_INVALID_PARAMETER;
}

API int
eom_set_attribute_changed_cb (eom_attribute_changed_cb callback, void *user_data)
{
    eom_output_notify_cb_info *cb_info;
    GList *l;

    RETV_IF_FAIL (callback != NULL, EOM_ERROR_INVALID_PARAMETER);

    _eom_mutex_lock ();

    for (l = cb_info_list; l; l = g_list_next (l))
    {
        eom_output_notify_cb_info *cb_info = (eom_output_notify_cb_info*)l->data;

        if (cb_info && (cb_info->attribute_change_func != NULL) && (cb_info->attribute_change_func == callback))
        {
            _eom_mutex_unlock ();
            return EOM_ERROR_NONE;
        }
    }

    cb_info = calloc (1, sizeof (eom_output_notify_cb_info));
    GOTO_IF_FAIL (cb_info != NULL, fail);

    cb_info->type = EOM_OUTPUT_NOTIFY_ATTRIBUTE_CHANGED;
    cb_info->attribute_change_func = callback;
    cb_info->user_data = user_data;

    cb_info_list = g_list_append (cb_info_list, cb_info);

    _eom_mutex_unlock ();

    return EOM_ERROR_NONE;

fail:
    _eom_mutex_unlock ();

    return EOM_ERROR_OUT_OF_MEMORY;
}

API int
eom_unset_attribute_changed_cb (eom_attribute_changed_cb callback)
{
    GList *l;

    RETV_IF_FAIL (callback != NULL, EOM_ERROR_INVALID_PARAMETER);

    _eom_mutex_lock ();

    for (l = cb_info_list; l; l = g_list_next (l))
    {
        eom_output_notify_cb_info *cb_info = (eom_output_notify_cb_info*)l->data;

        if (!cb_info || (cb_info->attribute_change_func == NULL) || (cb_info->attribute_change_func != callback))
            continue;

        cb_info_list = g_list_remove (cb_info_list, cb_info);
        free (cb_info);

        _eom_mutex_unlock ();

        return EOM_ERROR_NONE;
    }

    _eom_mutex_unlock ();

    return EOM_ERROR_INVALID_PARAMETER;
}

API int
eom_set_output_attribute (eom_output_id output_id, eom_output_attribute_e attr)
{
    bool ret = false;
    GValueArray *msg_array;
    GValueArray *ret_array;
    GValue v = G_VALUE_INIT;
    int pid = 0;

    RETV_IF_FAIL (output_id != 0, EOM_ERROR_INVALID_PARAMETER);
    RETV_IF_FAIL (attr > EOM_OUTPUT_ATTRIBUTE_NONE, EOM_ERROR_INVALID_PARAMETER);
    RETV_IF_FAIL (attr < EOM_OUTPUT_ATTRIBUTE_MAX, EOM_ERROR_INVALID_PARAMETER);

    _eom_mutex_lock ();

    pid = getpid();

    INFO ("output_id: %d, pid: %d, mode: %d\n", output_id, pid, attr);

    msg_array = g_value_array_new (0);

    /* 0:output_id, 1:pid, 2:eom_attribuete_e */
    g_value_init (&v, G_TYPE_INT);
    g_value_set_int (&v, output_id);
    msg_array = g_value_array_append (msg_array, &v);
    g_value_set_int (&v, pid);
    msg_array = g_value_array_append (msg_array, &v);
    g_value_set_int (&v, attr);
    msg_array = g_value_array_append (msg_array, &v);

    ret_array = eom_dbus_client_send_message ("SetOutputAttribute", msg_array);
    g_value_array_free (msg_array);
    if (!ret_array)
    {
       _eom_mutex_unlock ();
        return EOM_ERROR_MESSAGE_SENDING_FAILURE;
    }

    ret = g_value_get_int (g_value_array_get_nth (ret_array, 0));

    g_value_array_free (ret_array);

    INFO ("SetOutputAttribute: %s", (ret)?"success":"failed");

    _eom_mutex_unlock ();

    return (ret)?EOM_ERROR_NONE:EOM_ERROR_MESSAGE_OPERATION_FAILURE;
}


API int
eom_get_output_type (eom_output_id output_id, eom_output_type_e *type)
{
    eom_output_info *output_info = NULL;

    RETV_IF_FAIL (output_id != 0, EOM_ERROR_INVALID_PARAMETER);

    _eom_mutex_lock ();

    output_info = _eom_find_output_info (output_id);
    if (!output_info)
    {
        *type = EOM_OUTPUT_TYPE_UNKNOWN;
        _eom_mutex_unlock ();
        set_last_result (EOM_ERROR_NO_SUCH_DEVICE);
        return EOM_ERROR_NO_SUCH_DEVICE;
    }
    *type = output_info->type;

    _eom_mutex_unlock ();

    set_last_result (EOM_ERROR_NONE);
    return EOM_ERROR_NONE;
}

API int
eom_get_output_mode (eom_output_id output_id, eom_output_mode_e *mode)
{
    eom_output_info *output_info = NULL;

    RETV_IF_FAIL (output_id != 0, EOM_ERROR_INVALID_PARAMETER);

    _eom_mutex_lock ();

    output_info = _eom_find_output_info (output_id);
    if (!output_info)
    {
        *mode = EOM_OUTPUT_MODE_NONE;
        _eom_mutex_unlock ();
        set_last_result (EOM_ERROR_NO_SUCH_DEVICE);
        return EOM_ERROR_NO_SUCH_DEVICE;
    }
    *mode = output_info->output_mode;

    _eom_mutex_unlock ();

    set_last_result (EOM_ERROR_NONE);
    return EOM_ERROR_NONE;
}

API int
eom_get_output_attribute (eom_output_id output_id, eom_output_attribute_e *attribute)
{
    eom_output_info *output_info = NULL;

    RETV_IF_FAIL (output_id != 0, EOM_ERROR_INVALID_PARAMETER);

    _eom_mutex_lock ();

    output_info = _eom_find_output_info (output_id);
    if (!output_info)
    {
        *attribute = EOM_OUTPUT_ATTRIBUTE_NONE;
        _eom_mutex_unlock ();
        set_last_result (EOM_ERROR_NO_SUCH_DEVICE);
        return EOM_ERROR_NO_SUCH_DEVICE;
    }
    *attribute = output_info->attribute;

    _eom_mutex_unlock ();

    set_last_result (EOM_ERROR_NONE);
    return EOM_ERROR_NONE;
}

API int
eom_get_output_attribute_state (eom_output_id output_id, eom_output_attribute_state_e *state)
{
    eom_output_info *output_info = NULL;

    RETV_IF_FAIL (output_id != 0, EOM_ERROR_INVALID_PARAMETER);

    _eom_mutex_lock ();

    output_info = _eom_find_output_info (output_id);
    if (!output_info)
    {
        *state = EOM_OUTPUT_ATTRIBUTE_STATE_NONE;
        _eom_mutex_unlock ();
        set_last_result (EOM_ERROR_NO_SUCH_DEVICE);
        return EOM_ERROR_NO_SUCH_DEVICE;
    }
    *state = output_info->state;

    _eom_mutex_unlock ();

    set_last_result (EOM_ERROR_NONE);
    return EOM_ERROR_NONE;
}

API int
eom_get_output_resolution (eom_output_id output_id, int *width, int *height)
{
    eom_output_info *output_info = NULL;

    RETV_IF_FAIL (output_id != 0, EOM_ERROR_INVALID_PARAMETER);

    _eom_mutex_lock ();

    output_info = _eom_find_output_info (output_id);
    if (!output_info)
    {
        if (width)
            *width = 0;
        if (height)
            *height = 0;

        set_last_result (EOM_ERROR_NO_SUCH_DEVICE);
        _eom_mutex_unlock ();
        return EOM_ERROR_NO_SUCH_DEVICE;
    }

    if (width)
        *width = output_info->width;
    if (height)
        *height = output_info->height;

    _eom_mutex_unlock ();

    return EOM_ERROR_NONE;
}

API int
eom_get_output_physical_size (eom_output_id output_id, int *phy_width, int *phy_height)
{
    eom_output_info *output_info = NULL;

    RETV_IF_FAIL (output_id != 0, EOM_ERROR_INVALID_PARAMETER);

    _eom_mutex_lock ();

    output_info = _eom_find_output_info (output_id);
    if (!output_info)
    {
        if (phy_width)
            *phy_width = 0;
        if (phy_height)
            *phy_height = 0;

        _eom_mutex_unlock ();
        set_last_result (EOM_ERROR_NO_SUCH_DEVICE);
        return EOM_ERROR_NO_SUCH_DEVICE;
    }

    if (phy_width)
        *phy_width = output_info->mm_width;
    if (phy_height)
        *phy_height = output_info->mm_height;

    _eom_mutex_unlock ();

    return EOM_ERROR_NONE;
}

API int
eom_set_output_window (eom_output_id output_id, Evas_Object *win)
{
    bool ret = false;
    GValueArray *msg_array;
    GValueArray *ret_array;
    GValue v = G_VALUE_INIT;
    Ecore_X_Window xwin;
    int pid = 0;

    RETV_IF_FAIL (output_id != 0, EOM_ERROR_INVALID_PARAMETER);
    RETV_IF_FAIL (win != NULL, EOM_ERROR_INVALID_PARAMETER);

    _eom_mutex_lock ();

    pid = getpid();
    xwin = elm_win_xwindow_get (win);

    INFO ("output_id: %d, pid: %d, xwin: %d\n", output_id, pid, xwin);

    msg_array = g_value_array_new (0);

    /* 0:output_id, 1:pid, 2:eom_attribuete_e */
    g_value_init (&v, G_TYPE_INT);
    g_value_set_int (&v, output_id);
    msg_array = g_value_array_append (msg_array, &v);
    g_value_set_int (&v, pid);
    msg_array = g_value_array_append (msg_array, &v);
    g_value_set_int (&v, xwin);
    msg_array = g_value_array_append (msg_array, &v);

    ret_array = eom_dbus_client_send_message ("SetWindow", msg_array);
    g_value_array_free (msg_array);
    if (!ret_array)
    {
        _eom_mutex_unlock ();
        return EOM_ERROR_MESSAGE_SENDING_FAILURE;
    }

    ret = g_value_get_int (g_value_array_get_nth (ret_array, 0));

    g_value_array_free (ret_array);

    if (ret == 1)
    {
        const char *profile = "desktop";
        elm_win_profiles_set(win, &profile, 1);
        elm_win_fullscreen_set (win, EINA_TRUE);
        INFO ("SetWindow: success\n");
        _eom_mutex_unlock ();
        return EOM_ERROR_NONE;
    }


    INFO ("SetWindow: failed\n");
    _eom_mutex_unlock ();

    return EOM_ERROR_MESSAGE_OPERATION_FAILURE;
}

