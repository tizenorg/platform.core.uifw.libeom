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

#ifndef __EOM_H__
#define __EOM_H__

/**
 * @addtogroup CAPI_UI_EOM_MODULE
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <tizen.h>
#include <Evas.h>

/**
 * @file eom.h
 */
#ifndef TIZEN_ERROR_EOM
#define TIZEN_ERROR_EOM			(-0x02F20000 | 0x00)
#endif
/**
 * @brief Enumeration of External Output Manager (EOM) error type
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 */
typedef enum {
	EOM_ERROR_NONE = TIZEN_ERROR_NONE,                            /**< Success */
	EOM_ERROR_OUT_OF_MEMORY = TIZEN_ERROR_OUT_OF_MEMORY,          /**< Memory allocation failure */
	EOM_ERROR_INVALID_PARAMETER = TIZEN_ERROR_INVALID_PARAMETER,  /**< Invalid input parameter */
	EOM_ERROR_NO_SUCH_DEVICE = TIZEN_ERROR_NO_SUCH_DEVICE,        /**< Invalid external output instance */
	EOM_ERROR_CONNECTION_FAILURE = TIZEN_ERROR_EOM | 0x01,        /**< Connection failure */
	EOM_ERROR_MESSAGE_SENDING_FAILURE = TIZEN_ERROR_EOM | 0x02,   /**< Message sending failure */
	EOM_ERROR_MESSAGE_OPERATION_FAILURE = TIZEN_ERROR_EOM | 0x03, /**< Message operation failure */
} eom_error_e;

/**
 * @brief Enumeration of external output type
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 */
typedef enum {
	EOM_OUTPUT_TYPE_UNKNOWN,      /**< Unknown output type */
	EOM_OUTPUT_TYPE_VGA,          /**< VGA output */
	EOM_OUTPUT_TYPE_DVII,         /**< DVII output type */
	EOM_OUTPUT_TYPE_DVID,         /**< DVID output type */
	EOM_OUTPUT_TYPE_DVIA,         /**< DVIA output type */
	EOM_OUTPUT_TYPE_COMPOSITE,    /**< Composite output type */
	EOM_OUTPUT_TYPE_SVIDEO,       /**< SVIDEO output type */
	EOM_OUTPUT_TYPE_LVDS,         /**< LVDS output type */
	EOM_OUTPUT_TYPE_COMPONENT,    /**< Component output type */
	EOM_OUTPUT_TYPE_9PINDIN,      /**< 9PinDIN output type */
	EOM_OUTPUT_TYPE_DISPLAYPORT,  /**< DisplayPort output type */
	EOM_OUTPUT_TYPE_HDMIA,        /**< HDMIA output type */
	EOM_OUTPUT_TYPE_HDMIB,        /**< HDMIB output type */
	EOM_OUTPUT_TYPE_TV,           /**< TV output type */
	EOM_OUTPUT_TYPE_EDP,          /**< EDP output type */
	EOM_OUTPUT_TYPE_VIRTUAL,      /**< VIRTUAL output type */
	EOM_OUTPUT_TYPE_DSI,          /**< DSI output type */
	EOM_OUTPUT_TYPE_MAX,
} eom_output_type_e;

/**
 * @brief Enumeration of external output mode
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 */
typedef enum {
	EOM_OUTPUT_MODE_NONE,          /**< None */
	EOM_OUTPUT_MODE_MIRROR,        /**< Mirror mode */
	EOM_OUTPUT_MODE_PRESENTATION,  /**< Presentation mode */
	EOM_OUTPUT_MODE_MAX,
} eom_output_mode_e;

/**
 * @brief Enumeration of External Output Manager (EOM) attributes
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 */
typedef enum {
	EOM_OUTPUT_ATTRIBUTE_NONE,             /**< None */
	EOM_OUTPUT_ATTRIBUTE_NORMAL,           /**< Normal presentation mode window showing on external output */
	EOM_OUTPUT_ATTRIBUTE_EXCLUSIVE_SHARE,  /**< Exclusive share presentation mode window showing on external output */
	EOM_OUTPUT_ATTRIBUTE_EXCLUSIVE,        /**< Exclusive presentation mode window showing on external output */
	EOM_OUTPUT_ATTRIBUTE_MAX,
} eom_output_attribute_e;

/**
 * @brief Enumeration of External Output Manager (EOM) attribute state
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 */
typedef enum {
	EOM_OUTPUT_ATTRIBUTE_STATE_NONE,      /**< None */
	EOM_OUTPUT_ATTRIBUTE_STATE_ACTIVE,    /**< Output attribute is active */
	EOM_OUTPUT_ATTRIBUTE_STATE_INACTIVE,  /**< Output attribute is inactive */
	EOM_OUTPUT_ATTRIBUTE_STATE_LOST,      /**< Cannot use external output */
	EOM_OUTPUT_ATTRIBUTE_STATE_MAX,
} eom_output_attribute_state_e;

/**
 * @brief Definition for external output ID
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 */
typedef unsigned int eom_output_id;

/**
 * @brief Called when External Output Manager (EOM) module sends output
 *           connection notification.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @param[in] output_id  The output id which is connected output
 * @param[in] user_data  The pointer of user data which is passed to
 *                                   eom_output_added_cb function
 * @see eom_set_output_added_cb()
 * @see eom_unset_output_added_cb()
 */
typedef void (*eom_output_added_cb)(eom_output_id output_id, void *user_data);

/**
 * @brief Called when External Output Manager (EOM) module sends output
 *           disconnection notification.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @param[in] output_id  The output id which is connected output
 * @param[in] user_data  The pointer of user data which is passed to
 *                                   eom_output_removed_cb function
 * @see eom_set_output_removed_cb()
 * @see eom_unset_output_removed_cb()
 */
typedef void (*eom_output_removed_cb)(eom_output_id output_id, void *user_data);

/**
 * @brief Called when External Output Manager (EOM) module sends output
 *           mode changing notification.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @param[in] output_id  The output id which is connected output
 * @param[in] user_data  The pointer of user data which is passed to
 *                                   eom_mode_changed_cb function
 * @see eom_set_mode_changed_cb()
 * @see eom_unset_mode_changed_cb()
 */
typedef void (*eom_mode_changed_cb)(eom_output_id output_id, void *user_data);

/**
 * @brief Called when External Output Manager (EOM) module sends output
 *           attribute changing notification.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @param[in] output_id  The output id which is connected output
 * @param[in] user_data  The pointer of user data which is passed to
 *                                   eom_attribute_changed_cb function
 * @see eom_set_attribute_changed_cb()
 * @see eom_unset_attribute_changed_cb()
 */
typedef void (*eom_attribute_changed_cb)(eom_output_id output_id, void *user_data);

/**
 * @brief Initializes External Output Manager (EOM).
 * @details User should call this function previously for using EOM.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @remarks After all using, call eom_deinit() function for resource returning.
 * @return 0 on success, otherwise a negative error value
 * @retval #EOM_ERROR_NONE Successful
 * @retval #EOM_ERROR_CONNECTION_FAILURE The EOM connection failure
 * @see eom_deinit()
 * @see #eom_error_e
 */
int eom_init(void);

/**
 * @brief Finalizes External Output Manager (EOM).
 * @details User should call this function after using EOM to release all
 *              resources of EOM.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @see eom_init()
 */
void eom_deinit(void);

/**
 * @brief Registers a callback function to get output connection notification
 *           from External Output Manager (EOM) module.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @param[in] callback  The function pointer of eom_output_added_cb callback
 *                                function
 * @param[in] user_data  The pointer of user data which is passed to
 *                                   eom_output_added_cb function
 * @return 0 on success, otherwise a negative error value
 * @retval #EOM_ERROR_NONE Successful
 * @retval #EOM_ERROR_OUT_OF_MEMORY Memory allocation failure
 * @retval #EOM_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre eom_init()
 * @pre eom_get_eom_output_ids()
 * @see eom_unset_output_added_cb()
 * @see #eom_output_added_cb
 */
int eom_set_output_added_cb(eom_output_added_cb callback, void *user_data);

/**
 * @brief Unregisters the callback function.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @param[in] callback  The function pointer of eom_output_added_cb callback
 *                                function
 * @return 0 on success, otherwise a negative error value
 * @retval #EOM_ERROR_NONE Successful
 * @retval #EOM_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre eom_init()
 * @pre eom_get_eom_output_ids()
 * @see eom_set_output_added_cb()
 * @see #eom_output_added_cb
 */
int eom_unset_output_added_cb(eom_output_added_cb callback);

/**
 * @brief Registers a callback function to get output disconnection
 *           notification from External Output Manager (EOM) module.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @param[in] callback  The function pointer of eom_output_removed_cb callback
 *                                function
 * @param[in] user_data  The pointer of user data which is passed to
 *                                   eom_output_removed_cb function
 * @return 0 on success, otherwise a negative error value
 * @retval #EOM_ERROR_NONE Successful
 * @retval #EOM_ERROR_OUT_OF_MEMORY Memory allocation failure
 * @retval #EOM_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre eom_init()
 * @pre eom_get_eom_output_ids()
 * @see eom_unset_output_removed_cb()
 * @see #eom_output_removed_cb
 */
int eom_set_output_removed_cb(eom_output_removed_cb callback, void *user_data);

/**
 * @brief Unregisters the callback function.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @param[in] callback  The function pointer of eom_output_removed_cb callback
 *                                function
 * @return 0 on success, otherwise a negative error value
 * @retval #EOM_ERROR_NONE Successful
 * @retval #EOM_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre eom_init()
 * @pre eom_get_eom_output_ids()
 * @see eom_set_output_removed_cb()
 * @see #eom_output_removed_cb
 */
int eom_unset_output_removed_cb(eom_output_removed_cb callback);

/**
 * @brief Registers a callback function to get output mode changing
 *           notification from External Output Manager (EOM) module.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @param[in] callback  The function pointer of eom_mode_changed_cb callback
 *                                function
 * @param[in] user_data  The pointer of user data which is passed to
 *                                   eom_mode_changed_cb function
 * @return 0 on success, otherwise a negative error value
 * @retval #EOM_ERROR_NONE Successful
 * @retval #EOM_ERROR_OUT_OF_MEMORY Memory allocation failure
 * @retval #EOM_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre eom_init()
 * @pre eom_get_eom_output_ids()
 * @see eom_unset_mode_changed_cb()
 * @see #eom_mode_changed_cb
 */
int eom_set_mode_changed_cb(eom_mode_changed_cb callback, void *user_data);

/**
 * @brief Unregisters the callback function.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @param[in] callback  The function pointer of eom_mode_changed_cb callback
 *                                function
 * @return 0 on success, otherwise a negative error value
 * @retval #EOM_ERROR_NONE Successful
 * @retval #EOM_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre eom_init()
 * @pre eom_get_eom_output_ids()
 * @see eom_set_mode_changed_cb()
 * @see #eom_mode_changed_cb
 */
int eom_unset_mode_changed_cb(eom_mode_changed_cb callback);

/**
 * @brief Registers a callback function to get output attribute changing
 *           notification from External Output Manager (EOM) module.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @param[in] callback  The function pointer of eom_attribute_changed_cb
 *                                callback function
 * @param[in] user_data  The pointer of user data which is passed to
 *                                   eom_attribute_changed_cb function
 * @return 0 on success, otherwise a negative error value
 * @retval #EOM_ERROR_NONE Successful
 * @retval #EOM_ERROR_OUT_OF_MEMORY Memory allocation failure
 * @retval #EOM_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre eom_init()
 * @pre eom_get_eom_output_ids()
 * @see eom_unset_attribute_changed_cb()
 * @see #eom_attribute_changed_cb
 */
int eom_set_attribute_changed_cb(eom_attribute_changed_cb callback, void *user_data);

/**
 * @brief Unregisters the callback function.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @param[in] callback  The function pointer of eom_attribute_changed_cb
 *                                callback function
 * @return 0 on success, otherwise a negative error value
 * @retval #EOM_ERROR_NONE Successful
 * @retval #EOM_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre eom_init()
 * @pre eom_get_eom_output_ids()
 * @see eom_set_attribute_changed_cb()
 * @see #eom_attribute_changed_cb
 */
int eom_unset_attribute_changed_cb(eom_attribute_changed_cb callback);

/**
 * @brief Gets the IDs and count of external output.
 * @details This function returns the IDs of external output which are
 *              available to connect to target device, and the count of them
 *              also. User can get the id of external output.
 * which user want to watch.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @remarks User should free return value by using free().
 * The specific error code can be obtained using the get_last_result() method.
 * Error codes are described in Exception section.
 * @param[out] count  The count of the eom_output_id supported by system
 * @return The array of the eom_output_id if this function succeeds, otherwise
 *             NULL
 * @retval The pointer of #eom_output_id
 * @exception #EOM_ERROR_NONE Successful
 * @exception #EOM_ERROR_INVALID_PARAMETER Invalid parameter
 * @exception #EOM_ERROR_OUT_OF_MEMORY Memory allocation failure
 * @pre eom_init()
 * @see #eom_output_id
 * @see #eom_error_e
 */
eom_output_id *eom_get_eom_output_ids(int *count);

/**
 * @brief Gets type of external output.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @remarks The specific error code can be obtained using the get_last_result()
 * method. Error codes are described in Exception section.
 * @param[in] output_id  The id of external output device
 * @param[out] type  The type of external output instance
 * @return 0 on success, otherwise a negative error value
 * @retval #EOM_ERROR_NONE Successful
 * @retval #EOM_ERROR_INVALID_PARAMETER Invalid parameter
 * @pre eom_init()
 * @pre eom_get_eom_output_ids()
 * @see #eom_output_id
 * @see #eom_error_e
 * @see #eom_output_type_e
 */
int eom_get_output_type(eom_output_id output_id, eom_output_type_e *type);

/**
 * @brief Gets mode of external output.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @remarks The specific error code can be obtained using the get_last_result()
 * method. Error codes are described in Exception section.
 * @param[in] output_id  The id of external output device
 * @param[out] mode  The mode of external output instance
 * @return 0 on success, otherwise a negative error value
 * @retval #EOM_ERROR_NONE Successful
 * @retval #EOM_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #EOM_ERROR_NO_SUCH_DEVICE Invalid external output instance
 * @pre eom_init()
 * @pre eom_get_eom_output_ids()
 * @see #eom_output_id
 * @see #eom_error_e
 * @see #eom_output_mode_e
 */
int eom_get_output_mode(eom_output_id output_id, eom_output_mode_e *mode);

/**
 * @brief Gets attribute of external output.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @remarks The specific error code can be obtained using the get_last_result()
 * method. Error codes are described in Exception section.
 * @param[in] output_id  The id of external output device
 * @param[out] attribute  The attribute of external output instance
 * @return 0 on success, otherwise a negative error value
 * @retval #EOM_ERROR_NONE Successful
 * @retval #EOM_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #EOM_ERROR_NO_SUCH_DEVICE Invalid external output instance
 * @pre eom_init()
 * @pre eom_get_eom_output_ids()
 * @see #eom_output_id
 * @see #eom_error_e
 * @see #eom_output_attribute_e
 */
int eom_get_output_attribute(eom_output_id output_id, eom_output_attribute_e *attribute);

/**
 * @brief Gets attribute state of external output.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @remarks The specific error code can be obtained using the get_last_result()
 * method. Error codes are described in Exception section.
 * @param[in] output_id  The id of external output device
 * @param[out] state  The attribute state of external output instance
 * @return 0 on success, otherwise a negative error value
 * @retval #EOM_ERROR_NONE Successful
 * @retval #EOM_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #EOM_ERROR_NO_SUCH_DEVICE Invalid external output instance
 * @pre eom_init()
 * @pre eom_get_eom_output_ids()
 * @see #eom_output_id
 * @see #eom_error_e
 * @see #eom_output_attribute_state_e
 */
int eom_get_output_attribute_state(eom_output_id output_id, eom_output_attribute_state_e *state);

/**
 * @brief Gets resolution of external output.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @param[in] output_id  The id of external output device
 * @param[out] width  The width of external output instance
 * @param[out] height  The height of external output instance
 * @return 0 on success, otherwise a negative error value
 * @retval #EOM_ERROR_NONE Successful
 * @retval #EOM_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #EOM_ERROR_NO_SUCH_DEVICE Invalid external output instance
 * @pre eom_init()
 * @pre eom_get_eom_output_ids()
 * @see #eom_output_id
 * @see #eom_error_e
 */
int eom_get_output_resolution(eom_output_id output_id, int *width, int *height);

/**
 * @brief Gets physical width/height (millimeters) of external output.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @param[in] output_id  The id of external output device
 * @param[out] phy_width  The physical mm width of external output instance
 * @param[out] phy_height  The physical mm height of external output instance
 * @return 0 on success, otherwise a negative error value
 * @retval #EOM_ERROR_NONE Successful
 * @retval #EOM_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #EOM_ERROR_NO_SUCH_DEVICE Invalid external output instance
 * @pre eom_init()
 * @pre eom_get_eom_output_ids()
 * @see #eom_output_id
 * @see #eom_error_e
 */
int eom_get_output_physical_size(eom_output_id output_id, int *phy_width, int *phy_height);

/**
 * @brief Sets the attribute of the external output ID.
 * @details The application can set the External Output Manager (EOM) attribute
 *              to the external output ID.
 * The EOM module manages the windows to display on external output and
 * control the policy of external output. The application can recognize
 * the attribute state and manage the resources when the application receives
 * several notification callback from EOM module.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @param[in] output_id  The id of external output device
 * @param[in] attr  The attribute of the external output
 * @return 0 on success, otherwise a negative error value
 * @retval #EOM_ERROR_NONE Successful
 * @retval #EOM_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #EOM_ERROR_NO_SUCH_DEVICE Invalid external output instance
 * @retval #EOM_ERROR_MESSAGE_SENDING_FAILURE Communication failure with EOM
 *              module
 * @retval #EOM_ERROR_MESSAGE_OPERATION_FAILURE Operation failure
 * @pre eom_init()
 * @pre eom_get_eom_output_ids()
 * @see #eom_output_id
 * @see #eom_error_e
 */
int eom_set_output_attribute(eom_output_id output_id, eom_output_attribute_e attr);

/**
 * @brief Sets window to the external output best resolution of external output
 *           device.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 * @param[in] output_id  The id of external output device
 * @param[in] win  The pointer of evas object
 * @return 0 on success, otherwise a negative error value
 * @retval #EOM_ERROR_NONE Successful
 * @retval #EOM_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #EOM_ERROR_NO_SUCH_DEVICE Invalid external output instance
 * @retval #EOM_ERROR_MESSAGE_SENDING_FAILURE Communication failure with EOM
 *               module
 * @retval #EOM_ERROR_MESSAGE_OPERATION_FAILURE Operation failure
 * @pre eom_init()
 * @pre eom_get_eom_output_ids()
 * @see #eom_output_id
 * @see #eom_error_e
 */
int eom_set_output_window(eom_output_id output_id, Evas_Object *win);

#ifdef __cplusplus
}
#endif

/**
* @}
*/

#endif /* __EOM_H__ */
