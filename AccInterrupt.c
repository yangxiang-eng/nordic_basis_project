#if ACC_SENSOR_ENABLE
#include "AccInterrupt.h"
#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"
#include "boards.h"
#include "nrf_log.h"
#include "app_timer.h"

acc_interrupt_evt acc_interrupt_handler = NULL;

APP_TIMER_DEF(acc_evt_delay_timer_id);
static uint8_t gapAccTimerRun = false;
static void delay_timer_handler(void*p)
{
    gapAccTimerRun = false;
    acc_interrupt_handler(ACC_INT_BTN_1);
}

#ifdef ACC_INT_BTN_2
APP_TIMER_DEF(acc2_evt_delay_timer_id);
static uint8_t gapAcc2TimerRun = false;
static void delay_acc2_timer_handler(void*p)
{
    gapAccTimerRun = false;
    acc_interrupt_handler(ACC_INT_BTN_2);
}
#endif



// Event handler for the GPIOTE event for toggling LED
void button_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    if(pin == ACC_INT_BTN_1 && acc_interrupt_handler)
    {
        if (!gapAccTimerRun)
        {
            gapAccTimerRun = true;
            app_timer_start(acc_evt_delay_timer_id, APP_TIMER_TICKS(50), NULL);
        }
    }
    
    #ifdef ACC_INT_BTN_2
    if(pin == ACC_INT_BTN_2 && acc_interrupt_handler)
    {
        if (!gapAcc2TimerRun)
        {
            gapAccTimerRun = true;
            app_timer_start(acc2_evt_delay_timer_id, APP_TIMER_TICKS(50), NULL);
        }
    }
    #endif
    
}

void Acc_interrupt_init(acc_interrupt_evt accEvtCallback)
{
    nrf_drv_gpiote_init();

    acc_interrupt_handler = accEvtCallback;

    //Configure button with pullup and event on both high and low transition
    nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(false);
    config.pull = NRF_GPIO_PIN_PULLDOWN;

    nrf_drv_gpiote_in_init(ACC_INT_BTN_1, &config, button_event_handler); //Assign button config to a GPIOTE channel
    nrf_drv_gpiote_in_event_enable(ACC_INT_BTN_1, true);                  //Enable event and interrupt

#ifdef ACC_INT_BTN_2
    nrf_drv_gpiote_in_init(ACC_INT_BTN_2, &config, button_event_handler); //Assign button config to a GPIOTE channel
    nrf_drv_gpiote_in_event_enable(ACC_INT_BTN_2, true);                  //Enable event and interrupt
#endif



    uint32_t err_code=app_timer_create(&acc_evt_delay_timer_id,
                                       APP_TIMER_MODE_SINGLE_SHOT,delay_timer_handler);
    APP_ERROR_CHECK(err_code);
    gapAccTimerRun = false;

#ifdef ACC_INT_BTN_2
    err_code=app_timer_create(&acc2_evt_delay_timer_id,
                                       APP_TIMER_MODE_SINGLE_SHOT,delay_acc2_timer_handler);
    APP_ERROR_CHECK(err_code);
    gapAcc2TimerRun = false;
#endif
        
}

#endif
