/**
 * Copyright (c) 2014 - 2018, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/**@file
 *
 * @defgroup bsp Board Support Package
 * @{
 * @ingroup app_common
 *
 * @brief BSP module.
 * @details This module provides a layer of abstraction from the board.
 *          It allows the user to indicate certain states on LEDs in a simple way.
 *          Module functionality can be modified by additional defines:
 *          - BSP_SIMPLE reduces functionality of this module to enable
 *            and read state of the buttons
 *          - BSP_UART_SUPPORT enables support for UART
 */

#ifndef BSP_EXT_H__
#define BSP_EXT_H__

#include <stdint.h>
#include <stdbool.h>
#include "boards.h"

#define BSP_INIT_NONE    0        /**< This define specifies the type of initialization without support for LEDs and buttons (@ref bsp_init).*/
#define BSP_INIT_BUTTONS (1 << 1) /**< This bit enables buttons during initialization (@ref bsp_init).*/



/* BSP_UART_SUPPORT
 * This define enables UART support module.
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t bsp_button_action_t; /**< The different actions that can be performed on a button. */



/**@brief BSP events.
 *
 * @note Events from BSP_EVENT_KEY_0 to BSP_EVENT_KEY_LAST are by default assigned to buttons.
 */
typedef enum
{
    BSP_EVENT_NOTHING = 0,                  /**< Assign this event to an action to prevent the action from generating an event (disable the action). */

    BSP_EVENT_KEY_0_PUSH,
    BSP_EVENT_KEY_0_LONG_PUSH,
    BSP_EVENT_KEY_0_PUSH_REL,

     BSP_EVENT_KEY_1_PUSH,
     BSP_EVENT_KEY_1_LONG_PUSH,
     BSP_EVENT_KEY_1_PUSH_REL
} bsp_event_t;


typedef struct
{
    bsp_event_t push_event;      /**< The event to fire on regular button press. */
    bsp_event_t long_push_event; /**< The event to fire on long button press. */
    bsp_event_t release_event;   /**< The event to fire on button release. */
} bsp_button_event_cfg_t;

/**@brief BSP module event callback function type.
 *
 * @details     Upon an event in the BSP module, this callback function will be called to notify
 *              the application about the event.
 *
 * @param[in]   bsp_event_t BSP event type.
 */
typedef void (* bsp_event_callback_t)(bsp_event_t);


/**@brief       Function for initializing BSP.
 *
 * @details     The function initializes the board support package to allow state indication and
 *              button reaction. Default events are assigned to buttons.
 * @note        Before calling this function, you must initiate the following required modules:
 *              - @ref app_timer for LED support
 *              - @ref app_gpiote for button support
 *              - @ref app_uart for UART support
 *
 * @param[in]   type               Type of peripherals used.
 * @param[in]   ticks_per_100ms    Number of RTC ticks for 100 ms.
 * @param[in]   callback           Function to be called when button press/event is detected.
 *
 * @retval      NRF_SUCCESS               If the BSP module was successfully initialized.
 * @retval      NRF_ERROR_INVALID_STATE   If the application timer module has not been initialized.
 * @retval      NRF_ERROR_NO_MEM          If the maximum number of timers has already been reached.
 * @retval      NRF_ERROR_INVALID_PARAM   If GPIOTE has too many users.
 * @retval      NRF_ERROR_INVALID_STATE   If button or GPIOTE has not been initialized.
 */
uint32_t button_init(uint32_t ticks_per_100ms, bsp_event_callback_t callback);


/**@brief       Function for checking button states.
 *
 * @details     This function checks if the button is pressed. If the button ID is out of range,
 *              the function returns false.
 *
 * @param[in]   button                  Button ID to check.
 *
 * @retval      true                    If the button is pressed.
 * @retval      false                   If the button is not pressed.
 */
bool bsp_button_is_pressed(uint32_t button);


/**@brief       Function for assigning a specific event to a button.
 *
 * @details     This function allows redefinition of standard events assigned to buttons.
 *              To unassign events, provide the event @ref BSP_EVENT_NOTHING.
 *
 * @param[in]   button                   Button ID to be redefined.
 * @param[in]   action                   Button action to assign event to.
 * @param[in]   event                    Event to be assigned to button.
 *
 * @retval      NRF_SUCCESS              If the event was successfully assigned to button.
 * @retval      NRF_ERROR_INVALID_PARAM  If the button ID or button action was invalid.
 */
uint32_t bsp_event_to_button_action_assign(uint32_t button, bsp_button_action_t action, bsp_event_t event);



#ifdef __cplusplus
}
#endif

#endif // BSP_H__

/** @} */
