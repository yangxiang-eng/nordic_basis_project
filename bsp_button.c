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
#include "bsp_button.h"
#include <stddef.h>
#include <stdio.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_error.h"
#include "bsp_config.h"
#include "boards.h"
#include "nrf_log.h"

#ifndef BSP_SIMPLE
#include "app_timer.h"
#include "app_button.h"
#endif // BSP_SIMPLE

static uint32_t         m_app_ticks_per_100ms = 0;

static bsp_event_callback_t   m_registered_callback         = NULL;


static bsp_button_event_cfg_t m_events_list[BUTTONS_NUMBER] = {
    {BSP_EVENT_KEY_0_PUSH, BSP_EVENT_KEY_0_LONG_PUSH, BSP_EVENT_KEY_0_PUSH_REL}
#ifdef BSP_BUTTON_1
    ,{BSP_EVENT_KEY_1_PUSH, BSP_EVENT_NOTHING, BSP_EVENT_KEY_1_PUSH_REL}
#endif
    };


static void bsp_button_event_handler(uint8_t pin_no, uint8_t button_action);

APP_TIMER_DEF(m_button_timer_id);

static const app_button_cfg_t app_buttons[BUTTONS_NUMBER] =
{
    #ifdef BSP_BUTTON_0
    {BSP_BUTTON_0, false, BUTTON_PULL, bsp_button_event_handler},
    #endif // BUTTON_0

    #ifdef BSP_BUTTON_1
    {BSP_BUTTON_1, false, BUTTON_PULL, bsp_button_event_handler},
    #endif // BUTTON_1

    #ifdef BSP_BUTTON_2
    {BSP_BUTTON_2, false, BUTTON_PULL, bsp_button_event_handler},
    #endif // BUTTON_2

    #ifdef BSP_BUTTON_3
    {BSP_BUTTON_3, false, BUTTON_PULL, bsp_button_event_handler},
    #endif // BUTTON_3

};


bool bsp_button_is_pressed(uint32_t button)
{
    return bsp_board_button_state_get(button);
}

/**@brief Function for handling button events.
 *
 * @param[in]   pin_no          The pin number of the button pressed.
 * @param[in]   button_action   Action button.
 */
static void bsp_button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    bsp_event_t        event  = BSP_EVENT_NOTHING;
    uint32_t           button = 0;
    uint32_t           err_code;
    static uint8_t     current_long_push_pin_no;              /**< Pin number of a currently pushed button, that could become a long push if held long enough. */
    static bsp_event_t release_event_at_push[BUTTONS_NUMBER]; /**< Array of what the release event of each button was last time it was pushed, so that no release event is sent if the event was bound after the push of the button. */

    button = bsp_board_pin_to_button_idx(pin_no);

    if (button < BUTTONS_NUMBER)
    {
        switch (button_action)
        {
            case APP_BUTTON_PUSH:
                event = m_events_list[button].push_event;
                if (m_events_list[button].long_push_event != BSP_EVENT_NOTHING)
                {
                    err_code = app_timer_start(m_button_timer_id, BSP_MS_TO_TICK(BSP_LONG_PUSH_TIMEOUT_MS), (void*)&current_long_push_pin_no);
                    if (err_code == NRF_SUCCESS)
                    {
                        current_long_push_pin_no = pin_no;
                    }
                }
                release_event_at_push[button] = m_events_list[button].release_event;
                break;
            case APP_BUTTON_RELEASE:
                (void)app_timer_stop(m_button_timer_id);
                if (release_event_at_push[button] == m_events_list[button].release_event)
                {
                    event = m_events_list[button].release_event;
                }
                break;
            case BSP_BUTTON_ACTION_LONG_PUSH:
                event = m_events_list[button].long_push_event;
        }
    }

    if ((event != BSP_EVENT_NOTHING) && (m_registered_callback != NULL))
    {
        m_registered_callback(event);
    }
}

/**@brief Handle events from button timer.
 *
 * @param[in]   p_context   parameter registered in timer start function.
 */
static void button_timer_handler(void * p_context)
{
    bsp_button_event_handler(*(uint8_t *)p_context, BSP_BUTTON_ACTION_LONG_PUSH);
}


uint32_t button_init(uint32_t ticks_per_100ms, bsp_event_callback_t callback)
{
    uint32_t err_code = NRF_SUCCESS;

    m_app_ticks_per_100ms = ticks_per_100ms;
    m_registered_callback = callback;

    // BSP will support buttons and generate events

    /*
    for (num = 0; ((num < BUTTONS_NUMBER) && (err_code == NRF_SUCCESS)); num++)
    {
        err_code = bsp_event_to_button_action_assign(num, BSP_BUTTON_ACTION_PUSH, (bsp_event_t)BSP_EVENT_KEY_0_PUSH);
        if (err_code == NRF_SUCCESS)
        {
        	err_code = bsp_event_to_button_action_assign(num, BSP_BUTTON_ACTION_LONG_PUSH, (bsp_event_t) BSP_EVENT_KEY_0_LONG_PUSH);
        }
        if (err_code == NRF_SUCCESS)
        {
        	err_code = bsp_event_to_button_action_assign(num, BSP_BUTTON_ACTION_RELEASE, (bsp_event_t)BSP_EVENT_KEY_0_PUSH_REL);
        }
    }
    */
    
    if (err_code == NRF_SUCCESS)
    {
        err_code = app_button_init((app_button_cfg_t *)app_buttons,
                                   BUTTONS_NUMBER,
                                   ticks_per_100ms / 2);
    }

    if (err_code == NRF_SUCCESS)
    {
        err_code = app_button_enable();
    }

    if (err_code == NRF_SUCCESS)
    {
        err_code = app_timer_create(&m_button_timer_id,
                                    APP_TIMER_MODE_SINGLE_SHOT,
                                    button_timer_handler);
    }


    return err_code;
}


uint32_t bsp_event_to_button_action_assign(uint32_t button, bsp_button_action_t action, bsp_event_t event)
{
    uint32_t err_code = NRF_SUCCESS;

    if (button < BUTTONS_NUMBER)
    {
        switch (action)
        {
            case BSP_BUTTON_ACTION_PUSH:
                m_events_list[button].push_event = event;
                break;
            case BSP_BUTTON_ACTION_LONG_PUSH:
                m_events_list[button].long_push_event = event;
                break;
            case BSP_BUTTON_ACTION_RELEASE:
                m_events_list[button].release_event = event;
                break;
            default:
                err_code = NRF_ERROR_INVALID_PARAM;
                break;
        }
    }
    else
    {
        err_code = NRF_ERROR_INVALID_PARAM;
    }

    return err_code;
}

