#ifndef __UI_EOM_DOC_H__
#define __UI_EOM_DOC_H__

/**************************************************************************

External Output Manager (EOM)

Copyright 2014 Samsung Electronics co., Ltd. All Rights Reserved.

Contact:
SooChan Lim <sc1.lim@samsung.com>
Boram Park <boram1288.park@samsung.com>
Changyeon Lee <cyeon.lee@samsung.com>
JunKyeong Kim <jk0430.kim@samsung.com>

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

/**
 * @ingroup CAPI_UI_FRAMEWORK
 * @defgroup CAPI_UI_EOM_MODULE External Output Manager
 * @brief The External Output Manager provides APIs for external outputs.
 *
 * @section CAPI_UI_EOM_MODULE_HEADER Required Header
 *   #include <eom.h>
 *
 * @section CAPI_UI_EOM_MODULE_OVERVIEW Overview
 * The External Output Manager (EOM) is for managing external outputs.
 * User can get the information of external outputs and control them via
 * eom API.
 *
 * @sample code
#include <stdio.h>
#include <gio/gio.h>
#include <eom.h>
#include <Elementary.h>
#include <Ecore_X.h>

typedef struct
{
    Evas_Object *external_window;

    int hdmi_output_id;
    int virtual_output_id;
} SampleInfo;

static void
set_external_rect_bg_window (Evas_Object* win)
{
    Evas_Object *bg, *rect;

    bg = elm_bg_add (win);
    elm_win_resize_object_add (win, bg);
    evas_object_size_hint_weight_set (bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_show (bg);

    rect = evas_object_rectangle_add (evas_object_evas_get(win));
    evas_object_color_set (rect, 0, 0, 0, 0);
    evas_object_render_op_set (rect, EVAS_RENDER_COPY);

    elm_win_resize_object_add (win, rect);
    evas_object_size_hint_weight_set (rect, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_show (rect);
    evas_object_show (win);
}

static void
sample_notify_cb_output_add (eom_output_id output_id, void *user_data)
{
    SampleInfo *info = (SampleInfo*)user_data;

    if (info->hdmi_output_id != output_id)
    {
        printf ("OUTPUT ADDED. SKIP. my output ID is %d\n", info->hdmi_output_id);
        return;
    }

    printf ("output(%d) connected\n", output_id);

    if (!info->external_window)
    {
        info->external_window = elm_win_add(NULL, "external_win", ELM_WIN_BASIC);
        if (eom_set_output_window(info->hdmi_output_id, info->external_window) == EOM_ERROR_NONE)
        {
            set_external_rect_bg_window(info->external_window);
        }
        else
        {
            evas_object_del (info->external_window);
            info->external_window = NULL;
        }
    }
}

static void
sample_notify_cb_output_remove (eom_output_id output_id, void *user_data)
{
    SampleInfo *info = (SampleInfo*)user_data;

    if (info->hdmi_output_id != output_id)
    {
        printf ("OUTPUT REMOVED. SKIP. my output ID is %d\n", info->hdmi_output_id);
        return;
    }

    printf ("output(%d) disconnected\n", output_id);

    if (info->external_window)
    {
        evas_object_del (info->external_window);
        info->external_window = NULL;
    }
}

static void
sample_notify_cb_mode_changed (eom_output_id output_id, void *user_data)
{
    SampleInfo *info = (SampleInfo*)user_data;
    eom_output_mode_e mode = EOM_OUTPUT_MODE_NONE;

    if (info->hdmi_output_id != output_id)
    {
        printf ("MODE CHANGED. SKIP. my output ID is %d\n", info->hdmi_output_id);
        return;
    }

    eom_get_output_mode(output_id, &mode);
    printf ("output(%d) mode changed(%d)\n", output_id, mode);
}

static void
sample_notify_cb_attribute_changed (eom_output_id output_id, void *user_data)
{
    SampleInfo *info = (SampleInfo*)user_data;
    eom_output_attribute_e attribute = EOM_OUTPUT_ATTRIBUTE_NONE;
    eom_output_attribute_state_e state = EOM_OUTPUT_ATTRIBUTE_STATE_NONE;

    if (info->hdmi_output_id != output_id)
    {
        printf ("ATTR CHANGED. SKIP. my output ID is %d\n", info->hdmi_output_id);
        return;
    }

    eom_get_output_attribute(output_id, &attribute);
    eom_get_output_attribute_state(output_id, &state);

    printf ("output(%d) attribute changed(%d, %d)\n", output_id, attribute, state);
    if (state == EOM_OUTPUT_ATTRIBUTE_STATE_ACTIVE)
    {
        //active : display image to external window\n");
    }
    else if (state == EOM_OUTPUT_ATTRIBUTE_STATE_INACTIVE)
    {
        //inactive : stop displaying image to external window and destroy external window
        if (info->external_window)
        {
            sample_destroy_window (info->external_window);
            info->external_window = NULL;
        }
    }
    else if (state == EOM_OUTPUT_ATTRIBUTE_STATE_LOST)
    {
        //lost : stop displaying image to external window, destroy external window and unset callbacks
        if (info->external_window)
        {
            sample_destroy_window (info->external_window);
            info->external_window = NULL;
        }

        eom_unset_output_added_cb(sample_notify_cb_output_add);
        eom_unset_output_removed_cb(sample_notify_cb_output_remove);
        eom_unset_mode_changed_cb(sample_notify_cb_mode_changed);
        eom_unset_attribute_changed_cb(sample_notify_cb_attribute_changed);

        eom_deinit ();
    }
}

int
sample_get_output_id (const char *output_name)
{
    eom_output_id *output_ids = NULL;
    eom_output_id output_id = 0;
    eom_output_type_e output_type = EOM_OUTPUT_TYPE_UNKNOWN;
    int id_cnt = 0;
    int i;

    // get output_ids
    output_ids = eom_get_eom_output_ids(&id_cnt);
    if (id_cnt == 0)
    {
        printf ("no external outputs supported\n");
        return 0;
    }

    // find output ids interested
    for (i = 0; i < id_cnt; i++)
    {
        eom_get_output_type(output_ids[i], &output_type);
        if (!strncmp(output_name, "HDMI", 4))
        {
            if (output_type == EOM_OUTPUT_TYPE_HDMIA || output_type == EOM_OUTPUT_TYPE_HDMIB)
            {
                output_id = output_ids[i];
                break;
            }
        }
        else if (!strncmp(output_name, "Virtual", 7))
        {
            if (output_type == EOM_OUTPUT_TYPE_VIRTUAL)
            {
                output_id = output_ids[i];
                break;
            }
        }
    }

    if (output_ids)
        free (output_ids);

    return output_id;
}

int
elm_main(int argc, char **argv)
{
    SampleInfo *info;
    eom_error_e ret;
    eom_output_mode_e output_mode = EOM_OUTPUT_MODE_NONE;
    memset (info, 0x00, sizeof(SampleInfo));

    if (eom_init () != EOM_ERROR_NONE)
        return 0;

    // get output id
    info->hdmi_output_id = sample_get_output_id ("HDMI");
    if (info->hdmi_output_id == 0)
    {
        printf ("error : HDMI output id is NULL.\n");
        eom_deinit ();
        return 0;
    }
    ret = eom_set_output_attribute(info->hdmi_output_id, EOM_OUTPUT_ATTRIBUTE_NORMAL);
    if (ret != EOM_ERROR_NONE)
    {
        printf ("Set attribute fail. Cannot use external output\n");
        eom_deinit ();
        return 0;
    }

    if (ret == EOM_ERROR_NONE)
    {
        eom_get_output_mode(info->hdmi_output_id, &output_mode);
        if (output_mode != EOM_OUTPUT_MODE_NONE)
        {
            //create external window(info->external_window)
            info->external_window = elm_win_add(NULL, "external_win", ELM_WIN_BASIC);
            if (eom_set_output_window(info->hdmi_output_id, info->external_window) == EOM_ERROR_NONE)
            {
                set_external_rect_bg_window(info->external_window);
            }
            else
            {
                evas_object_del (info->external_window);
                info->external_window = NULL;
            }
        }

        eom_set_output_added_cb(sample_notify_cb_output_add, info);
        eom_set_output_removed_cb(sample_notify_cb_output_remove, info);
        eom_set_mode_changed_cb(sample_notify_cb_mode_changed, info);
        eom_set_attribute_changed_cb(sample_notify_cb_attribute_changed, info);
    }

    elm_run();

    if (ret == EOM_ERROR_NONE)
    {
        eom_unset_output_added_cb(sample_notify_cb_output_add);
        eom_unset_output_removed_cb(sample_notify_cb_output_remove);
        eom_unset_mode_changed_cb(sample_notify_cb_mode_changed);
        eom_unset_attribute_changed_cb(sample_notify_cb_attribute_changed);

        eom_deinit ();
    }

    elm_shutdown();

    return 0;
}
 * @endcode
 */

#endif /* __UI_EOM_DOC_H__ */
