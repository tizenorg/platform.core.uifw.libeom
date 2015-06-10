/**************************************************************************

eom (external output manager)

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

#ifndef __EOM_INTERNAL_H__
#define __EOM_INTERNAL_H__

/**
 * @brief Enumeration of eom notify type
 * @since_tizen 2.4
 */
typedef enum {
	EOM_OUTPUT_NOTIFY_NONE,               /**< None notify */
	EOM_OUTPUT_NOTIFY_ADD,                /**< Output add notify */
	EOM_OUTPUT_NOTIFY_REMOVE,             /**< Output remove notify */
	EOM_OUTPUT_NOTIFY_MODE_CHANGED,       /**< Mode change notify */
	EOM_OUTPUT_NOTIFY_ATTRIBUTE_CHANGED,  /**< Attribute change notify */
	EOM_OUTPUT_NOTIFY_MAX,
} eom_output_notify_type_e;

#endif /* __EOM_INTERNAL_H__ */
