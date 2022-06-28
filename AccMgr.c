#include "AccMgr.h"
#include <nrf_delay.h>
#include <nrf_drv_twi.h>
#include <stdio.h>
#include "boards.h"
#include "ohm3_driver.h"
#include "AccInterrupt.h"
#include "nrf_log.h"
#include "nrf_drv_twi.h"

static void acc_interrupt_callback(uint8_t event);




static const nrf_drv_twi_t i2c_instance = NRF_DRV_TWI_INSTANCE(0);
//static const nrf_drv_twi_t i2c_instance = NRFX_TWIM_INSTANCE(0);

/**
 * Initialize all hard- and software components that are needed for the I2C
 * communication.
 */
void Acc_mgr_init(void) 
{
    int8_t err;
    //const nrf_drv_twi_config_t i2c_instance_config = {.scl = ARDUINO_SCL_PIN,
    //                                                  .sda = ARDUINO_SDA_PIN,
    //                                                  .frequency =
    //                                                   NRF_TWI_FREQ_100K,
    //                                                  .interrupt_priority = 0};
    /* initiate TWI instance */

      const nrf_drv_twi_config_t i2c_instance_config =
      {
          .scl                = ARDUINO_SCL_PIN,
          .sda                = ARDUINO_SDA_PIN,
          .frequency          = NRF_DRV_TWI_FREQ_100K,
          .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
          .clear_bus_init     = true,
          .hold_bus_uninit = false
      };

    err = nrf_drv_twi_init(&i2c_instance, &i2c_instance_config, NULL, NULL);
    if (err) {
        /* Could be omitted if the prototyp is changed to non-void or an error
         * flag is introduced */
        NRF_LOG_INFO("Error %d: Initialization of I2C connection failed!\n", err);
    }
    /* enable TWI instance */
    nrf_drv_twi_enable(&i2c_instance);

    //reset
    OHM_SoftReset();

    //uninstall twi driver
    nrf_drv_twi_disable(&i2c_instance);
    nrf_drv_twi_uninit(&i2c_instance);

    //reopen twi driver
    nrf_delay_ms(15);
    err = nrf_drv_twi_init(&i2c_instance, &i2c_instance_config, NULL, NULL);
    APP_ERROR_CHECK(err);
    nrf_drv_twi_enable(&i2c_instance);
    while (!OHM_IsResetComplete())
    {
        nrf_delay_ms(5);
    }

    Acc_interrupt_init(acc_interrupt_callback);

    return;
}

static void acc_interrupt_callback(uint8_t event)
{
    uint8_t data;
    if ( nrf_gpio_pin_read(ACC_INT_BTN_1) == 0)
    {
        NRF_LOG_INFO("acc pin not height");
        return;
    }

    OHM_GetInt1Src(&data);

    NRF_LOG_INFO("acc evt:%d", data);
}

uint8_t Acc_ReadReg(uint8_t addr, uint8_t *buf)
{
    uint32_t err_code = 0;
    uint8_t readAddr[] = {addr};

    err_code = nrf_drv_twi_tx(&i2c_instance, OHM_ADDR_READ, readAddr, 1, true);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_twi_rx(&i2c_instance, OHM_ADDR_READ, buf, 1);
    APP_ERROR_CHECK(err_code);

    return true;
}

uint8_t Acc_WriteReg(uint8_t addr, uint8_t buf)
{
    uint8_t txBuf[] = {addr, buf};

    uint32_t err_code = nrf_drv_twi_tx(&i2c_instance, OHM_ADDR_READ, txBuf, sizeof(txBuf), true);
    APP_ERROR_CHECK(err_code);

    return true;
}


