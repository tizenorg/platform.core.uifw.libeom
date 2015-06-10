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

#ifndef __EOM_CONNECT_H__
#define __EOM_CONNECT_H__

/**
 * @ingroup
 * @addtogroup CAPI_UI_EOM_MODULE
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <eom.h>

/**
 * @file eom-connect.h
 */

/**
 * @brief Set mode to external output.
 * @param[in] output : The pointer of external output instance
 * @param[in] output_id : eom output id
 * @return #EOM_ERROR_NONE if this function succeeds, otherwise error status value
 * @retval #EOM_ERROR_NONE Successful
 * @retval #EOM_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #EOM_ERROR_SEND_MESSAGE_FAILE Message sending failure
 * @retval #EOM_ERROR_OPERATE_MESSAGE_FAILE Message operation failure
 * see #eom_output_mode_e
 */
int eom_output_set_mode(eom_output_id output_id, eom_output_mode_e mode);


#ifdef __cplusplus
}
#endif

/**
* @}
*/

#endif /* __EOM_CONNECT_H__ */
