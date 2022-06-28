/**
 * Copyright (c) 2020, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Use in source and binary forms, redistribution in binary form only, with
 * or without modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 2. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 3. This software, with or without modification, must only be used with a Nordic
 *    Semiconductor ASA integrated circuit.
 *
 * 4. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/**
 * @brief Advertiser module header of Distance Measurement application.
 *
 */

#ifndef _ADVERTISE_H
#define _ADVERTISE_H

#include "ble_gap.h"

typedef struct
{
    bool                    initialized;
    bool                    advertising_start_pending;                        /**< Flag to keep track of ongoing advertising operations. */
    uint8_t                 conn_cfg_tag;    /**< Variable to keep track of what connection settings will be used if the advertising results in a connection. */
    ble_gap_adv_params_t    adv_params;                        /**< GAP advertising parameters. */
    uint8_t                 adv_handle;                                       /**< Handle for the advertising set. */
    ble_gap_adv_data_t      adv_data;                                         /**< Advertising data. */

} dm_ble_adv_t;

#define HI_UINT16(a) (((a) >> 8) & 0xFF)
#define LO_UINT16(a) ((a) & 0xFF)
#define BREAK_UINT32( var, ByteNum )  (uint8_t)((uint32_t)(((var) >>((ByteNum) * 8)) & 0x00FF))

typedef struct
{
    uint8_t   frameType;      // TLM
    uint8_t   version;        // 0x00 for now
    uint8_t   vBatt[2];       // Battery Voltage, 1mV/bit, Big Endian
    uint8_t   temp[2];        // Temperature. Signed 8.8 fixed point
    uint8_t   advCnt[4];      // Adv count since power-up/reboot
    uint8_t   secCnt[4];      // Time since power-up/reboot
} kBeaconAdvData_t;

#define RTC_2_MS(a)  ((a*125) >> (10 + APP_TIMER_CONFIG_RTC_FREQUENCY))

#define EDDYSTONE_SERVICE_UUID 0xFEAA

//init adv
ret_code_t advertising_init(void);

//start advertisement
ret_code_t advertising_start(void);

//stop adv
ret_code_t advertising_stop(void);


#endif /* _ADVERTISE_H */
