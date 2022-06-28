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
 * @brief Advertiser implementation of Distance Measurement application.
 *
 * This file contains the source code for an advertiser for a Distance Measurement applicaiton.
 */

#include "ble_advdata.h"
#include "ble_srv_common.h"
#include "advertiser.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "app_timer.h"
#include "boards.h"
#include "ble_radio_notification.h"
 
#define APP_ADV_DURATION                    BLE_GAP_ADV_TIMEOUT_GENERAL_UNLIMITED   /**< The advertising duration set to unlimited. */
#define MAX_SRV_DATA_LEN 10
#define ESL_FRAME 0x10

static uint16_t mEncodeAdvLen = 0;
#define BEACON_SERV_UUID               0xFEA0
static ble_advdata_manuf_data_t m_manuf_advdata;                                    /**< Storage for manufacturer specific advertising data. */
static uint8_t                  m_enc_advdata[BLE_GAP_ADV_SET_DATA_SIZE_MAX];       /**< Advertising data in encoded form. */
static ble_uuid_t m_adv_uuids[] =                      
{
    {BEACON_SERV_UUID, BLE_UUID_TYPE_BLE}
};

//static
static dm_ble_adv_t m_advertising_ctx;  

//adv data
static kBeaconAdvData_t kBeaconAdv;
static int gapTotalAdvNum = 0;
static bool m_advertising_mode = false;

//function
static ret_code_t advertising_set_parameters_data(void);
static void advertising_update_adv_data(void);
static uint32_t advertising_encode_adv_data(void);
static void ble_on_radio_active_evt(bool radio_active);

//temperature
int gChipTemp = 32;

ret_code_t advertising_init(void)
{
    //init adv data
    if (advertising_encode_adv_data() != NRF_SUCCESS)
    {
        NRF_LOG_INFO("encode adv data failed");
        return NRF_ERROR_NULL;
    }
    m_advertising_ctx.conn_cfg_tag = BLE_CONN_CFG_TAG_DEFAULT;
    if (!m_advertising_ctx.initialized)
    {
        m_advertising_ctx.adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET;
    }

    memset(&m_advertising_ctx.adv_data, sizeof(m_advertising_ctx.adv_data), 0);
    m_advertising_ctx.adv_data.adv_data.p_data = &m_enc_advdata[0];
    m_advertising_ctx.adv_data.adv_data.len = mEncodeAdvLen;

    //adv interval
    m_advertising_ctx.adv_params.interval = 1600;
    m_advertising_ctx.adv_params.duration = 0;

    m_advertising_ctx.adv_params.p_peer_addr     = NULL;
    m_advertising_ctx.adv_params.filter_policy   = BLE_GAP_ADV_FP_ANY;
    m_advertising_ctx.adv_params.primary_phy           = BLE_GAP_PHY_AUTO;
    m_advertising_ctx.adv_params.secondary_phy         = BLE_GAP_PHY_AUTO;
    m_advertising_ctx.adv_params.max_adv_evts          = 0;                   
    m_advertising_ctx.adv_params.properties.type = BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED;

    //set paramaters
    uint32_t err_code = advertising_set_parameters_data();
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_INFO("set paramaters failed");
        return err_code;
    }

    //init radio event
    err_code = ble_radio_notification_init(APP_IRQ_PRIORITY_LOW,
                                           NRF_RADIO_NOTIFICATION_DISTANCE_800US,
                                           ble_on_radio_active_evt);
    err_code = sd_radio_notification_cfg_set(NRF_RADIO_NOTIFICATION_TYPE_INT_ON_INACTIVE,
               NRF_RADIO_NOTIFICATION_DISTANCE_2680US);
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}

//update advertisement content
extern int16_t adcEddystoneAdvTemp;
static void advertising_update_adv_data(void)
{
    uint16_t batt;

    // Battery voltage (bit 10:8 - integer, but 7:0 fraction)
    batt = 3000;
    kBeaconAdv.vBatt[0] = HI_UINT16(batt);
    kBeaconAdv.vBatt[1] = LO_UINT16(batt);

    // Temperature - 19.5 (Celcius) for example
    kBeaconAdv.temp[0]  = (adcEddystoneAdvTemp >> 8);
    kBeaconAdv.temp[1]  = (adcEddystoneAdvTemp & 0xFF) ;

    // advertise packet cnt;
    kBeaconAdv.advCnt[0] = BREAK_UINT32(gapTotalAdvNum, 3);
    kBeaconAdv.advCnt[1] = BREAK_UINT32(gapTotalAdvNum, 2);
    kBeaconAdv.advCnt[2] = BREAK_UINT32(gapTotalAdvNum, 1);
    kBeaconAdv.advCnt[3] = BREAK_UINT32(gapTotalAdvNum, 0);

    // running time
    uint32_t rtcCount = app_timer_cnt_get();
    uint32_t nUTCMSec = rtcCount / 32768;
    uint32_t time100MiliSec = nUTCMSec / 100; // 1-second resolution for now
    kBeaconAdv.secCnt[0] = BREAK_UINT32(time100MiliSec, 3);
    kBeaconAdv.secCnt[1] = BREAK_UINT32(time100MiliSec, 2);
    kBeaconAdv.secCnt[2] = BREAK_UINT32(time100MiliSec, 1);
    kBeaconAdv.secCnt[3] = BREAK_UINT32(time100MiliSec, 0);
}

//encode rawdata to BLE adv content
static uint32_t advertising_encode_adv_data(void)
{
    ble_advdata_t m_adv_data;
    ble_advdata_service_data_t m_srv_adv;
    ble_uuid_t    m_eddy_service_uuid;

    //get adv content
    kBeaconAdv.frameType = 0x20;
    kBeaconAdv.version = 0;
    advertising_update_adv_data();

    /////////////////
    m_srv_adv.service_uuid = EDDYSTONE_SERVICE_UUID;
    m_srv_adv.data.size = sizeof(kBeaconAdv);
    m_srv_adv.data.p_data = (uint8_t*)&kBeaconAdv;

    //service uuid
    m_eddy_service_uuid.type = BLE_UUID_TYPE_BLE;
    m_eddy_service_uuid.uuid = EDDYSTONE_SERVICE_UUID;
    
    //adv data
    memset(&m_adv_data, 0, sizeof(m_adv_data));
    m_adv_data.name_type = BLE_ADVDATA_NO_NAME;
    m_adv_data.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    m_adv_data.uuids_complete.uuid_cnt = 1;
    m_adv_data.uuids_complete.p_uuids = &m_eddy_service_uuid;
    m_adv_data.service_data_count = 1;
    m_adv_data.p_service_data_array = &m_srv_adv;

    //encode adv data
    mEncodeAdvLen = BLE_GAP_ADV_SET_DATA_SIZE_MAX;
    uint32_t ret = ble_advdata_encode(
                       &m_adv_data,
                       m_enc_advdata,
                       &mEncodeAdvLen);
    APP_ERROR_CHECK (ret);


    return 0;
}

static ret_code_t advertising_set_parameters_data(void)
{
    ret_code_t ret = sd_ble_gap_adv_set_configure(&m_advertising_ctx.adv_handle, 
      &m_advertising_ctx.adv_data, &m_advertising_ctx.adv_params);
    if (ret != NRF_SUCCESS) {
        NRF_LOG_ERROR("Can't configure advertising data");
        return ret;
    }

    m_advertising_ctx.conn_cfg_tag = 1;
    m_advertising_ctx.initialized = true;
    return NRF_SUCCESS;
}


ret_code_t advertising_start(void)
{
    uint32_t err_code;
    //check need stop privous adv
    if (m_advertising_mode)
    {
        sd_ble_gap_adv_stop(m_advertising_ctx.adv_handle);
        m_advertising_mode = false;
        NRF_LOG_INFO("Stop priv adv");

    }

    //init adv
    err_code = advertising_init();
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_INFO("init adv failed:%d", err_code);
        return err_code;
    }

    //start advertisement
    err_code = sd_ble_gap_adv_start(m_advertising_ctx.adv_handle, m_advertising_ctx.conn_cfg_tag);
    if (err_code != NRF_SUCCESS)
    {
        NRF_LOG_INFO("Can't start advertising:%d", err_code);
        return err_code;
    }
    else
    {
         NRF_LOG_INFO("start adv, interval:%d", m_advertising_ctx.adv_params.interval);
    }
    
    m_advertising_mode = true;

    return NRF_SUCCESS;
}

ret_code_t advertising_stop(void)
{
    ret_code_t ret = sd_ble_gap_adv_stop(m_advertising_ctx.adv_handle);
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_ERROR("Can't stop advertising");
        return ret;
    }

    m_advertising_mode = false;
    return NRF_SUCCESS;
}


/**@brief Callback function that triggers GPIO pin state change on change of radio activity
 *
 * @param[in] radio_active   Flag that informs if radio was activated or deactivated
 */
static void ble_on_radio_active_evt(bool radio_active)
{
    if (radio_active && m_advertising_mode)
    {
        advertising_encode_adv_data();
        gapTotalAdvNum++;
    }
}


