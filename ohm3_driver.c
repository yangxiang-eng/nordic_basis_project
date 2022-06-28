#include "ohm3_driver.h"
#include "boards.h"
#include "nrf_delay.h"
#include "nrf_log.h"
//#include "utc_clock.h"
#include <stdlib.h>

extern uint8_t Acc_ReadReg(uint8_t addr, uint8_t *buf);

extern uint8_t Acc_WriteReg(uint8_t addr, uint8_t buf);

static uint8_t OHM_ReadReg(uint8_t addr, uint8_t *buf)
{
    return Acc_ReadReg(addr, buf);
}

static uint8_t OHM_WriteReg(uint8_t addr, uint8_t buf)
{
    return Acc_WriteReg(addr, buf);
}

void OHM_SoftReset(void)
{
    Acc_WriteReg(regAddressCNTL2, 0x80);   
}

bool OHM_IsResetComplete(void)
{
    uint8_t CNTL2 = 0;
    Acc_ReadReg(regAddressCNTL2, &CNTL2);
    return ((CNTL2 & 0x55) == 0);
}


void OHM_SensorStoped(void)
{
    //read
    uint8_t INTREL;
    if (OHM_GetInt1Src(&INTREL) != 0)
    {
        NRF_LOG_INFO("OHM sensor stop error");
    }
    
    //turn off
    OHM_WriteReg(regAddressCNTL1, 0x0);

    //set cntl2
    OHM_WriteReg(regAddressCNTL2, 0x0);

    //set to default
    OHM_WriteReg(regAddressDataCtrl, 0x2);

     //to default
    OHM_WriteReg(regAddressWAKEUPCount, 0x0);

    //motion range to defualt
    OHM_WriteReg(regAddressWAKEUP1, 0x8);
    OHM_WriteReg(regAddressWAKEUP2, 0x0);

    //INT1 default for height
    OHM_WriteReg(regAddressINTCtrlReg1, 0x10);

     //heigh current, 16bit sensitive, standby
    OHM_WriteReg(regAddressCNTL1, 0x0);

    nrf_delay_ms(5);

    NRF_LOG_INFO("OHM sensor stop complete");
}

void OHM_PosDetectEnable(void)
{
    //heigh current, 16bit sensitive, standby
    OHM_WriteReg(regAddressCNTL1, 0x00);
    
    //set odh = 50hz
    OHM_WriteReg(regAddressCNTL2, 0x0);
    OHM_WriteReg(regAddressDataCtrl, 0x0);

    //setCNTL1reg to operator mode
    OHM_WriteReg(regAddressCNTL1, 0x80); //0x80

    NRF_LOG_INFO("OHM sensor pos enable");
}


bool OHM_StartSelfTest(void)
{
    return true;

    /*
    OHM_PosDetectEnable();
	
    nrf_delay_ms(20);
    
    AxesRaw_t avgAccNostdata, avgStAccdata, tmpAccdata;
    avgAccNostdata.AXIS_X = 0;
    avgAccNostdata.AXIS_Y = 0;
    avgAccNostdata.AXIS_Z = 0;
    for (int i = 0; i < 6; i++)
    {
        uint8_t response = OHM_GetPosAxesRaw(&tmpAccdata);
        if (!response)
        {
            NRF_LOG_INFO("self test OHM_GetAccAxesRaw failed ");
            return false;
        }

        if (i == 0)
        {
            continue;
        }

        avgAccNostdata.AXIS_X += tmpAccdata.AXIS_X;
        avgAccNostdata.AXIS_Y += tmpAccdata.AXIS_Y;
        avgAccNostdata.AXIS_Z += tmpAccdata.AXIS_Z ;
    }
    avgAccNostdata.AXIS_X = avgAccNostdata.AXIS_X / 5;
    avgAccNostdata.AXIS_Y = avgAccNostdata.AXIS_Y / 5;
    avgAccNostdata.AXIS_Z  = avgAccNostdata.AXIS_Z  / 5;


    //Enable Self Test
    OHM_WriteReg(regAddressINT1, 0x2);

    if( !OHM_WriteReg(regAddressSelftest, 0xCA))
    {
        NRF_LOG_INFO("self test failed ");
        return false;
    }

    //Wait for 90ms for stable output
    nrf_delay_ms(60);
    avgStAccdata.AXIS_X = 0;
    avgStAccdata.AXIS_Y = 0;
    avgStAccdata.AXIS_Z = 0;
    for (int i = 0; i < 6; i++)
    {
        uint8_t response = OHM_GetPosAxesRaw(&tmpAccdata);
        if (!response)
        {
            NRF_LOG_INFO("self test OHM_GetAccAxesRaw failed ");
            return false;
        }
        if (i == 0)
        {
            continue;
        }

        avgStAccdata.AXIS_X += tmpAccdata.AXIS_X;
        avgStAccdata.AXIS_Y += tmpAccdata.AXIS_Y;
        avgStAccdata.AXIS_Z += tmpAccdata.AXIS_Z ;
    }
    avgStAccdata.AXIS_X = avgStAccdata.AXIS_X / 5;
    avgStAccdata.AXIS_Y = avgStAccdata.AXIS_Y / 5;
    avgStAccdata.AXIS_Z  = avgStAccdata.AXIS_Z  / 5;

    //abos value
    int16_t nXMinus = abs(avgStAccdata.AXIS_X - avgAccNostdata.AXIS_X);
    int16_t nYMinus = abs(avgStAccdata.AXIS_Y - avgAccNostdata.AXIS_Y);
    int16_t nZMinus = abs(avgStAccdata.AXIS_Z - avgAccNostdata.AXIS_Z);

    OHM_WriteReg(regAddressSelftest, 0x0);
    
    //verify if is valid
    if ( (nXMinus <= 0 || nXMinus >= 900)
         ||(nYMinus <= 0 || nYMinus >= 900)
         ||(nZMinus <= 0 || nZMinus >= 900))
    {
        NRF_LOG_INFO("OHM self test failed:%d,%d,%d", nXMinus, nYMinus, nZMinus);
        return false;
    }

    NRF_LOG_INFO("OHM self test success:%d,%d,%d", nXMinus, nYMinus, nZMinus);
    return true;
    */
}


bool OHM_MotionDetectEnable(uint8_t odr, uint8_t accRange, uint8_t nDuration)
{
    if (accRange <= 1)
    {
        accRange = 1;
    }

    if (odr != OHM_ODR_50Hz && odr != OHM_ODR_100Hz)
    {
        odr = OHM_ODR_50Hz;
    }

    uint8_t readData = 0;
    
    /* to set the accelerometer in stand-by mode,
    to set the performance mode to High Resolution (full power), G-range to ¡À2g, and
    enable the Wake Up (motion detect) function.
    */
    OHM_WriteReg(regAddressCNTL1, 0x0);  //0x2

    //Read  from  the  Interrupt  Latch  Release  Register  (INT_REL)  to  clear  any  outstanding interrupts. 
    OHM_ReadReg(regAddressINTREL, &readData);
    OHM_WriteReg(regAddressDataCtrl, 0x2);

    //for the wake up function  50hz
    OHM_WriteReg(regAddressCNTL2, 0x6);

     //for the wake up for 1/odr = 1/50 * nDuration = 5ms
    OHM_WriteReg(regAddressWAKEUPCount, nDuration);

    //motion range
    uint16_t nRange = accRange * 4;//accRange * 1;
    nRange = (nRange << 4);
    OHM_WriteReg(regAddressWAKEUP1, ((nRange >> 8) & 0xFF));
    OHM_WriteReg(regAddressWAKEUP2, (nRange & 0xFF));


    // Write 0x30 to Interrupt Control Register 1 (INT_CTRL_REG1) to configure the hardware interrupt
    OHM_WriteReg(regAddressINTCtrlReg1, 0x30);

    OHM_WriteReg(regAddressCNTL1, 0x82);  //0x2


    uint8_t nIntValue = 0;
    OHM_GetInt1Src(&nIntValue);
    //NRF_LOG_INFO("OHM sensor motion enable:%d, %d", UTC_getSec(), (nRange >>4));
    NRF_LOG_INFO("OHM sensor enable\n");

    return true;

}

status_t OHM_GetInt1Src(uint8_t* val) 
{
  uint8_t INT_REL = 0;
  //read acc source
  OHM_ReadReg(regAddressIntSrc1, val);
  
  if( !OHM_ReadReg(regAddressINTREL, &INT_REL) )
    return MEMS_ERROR;
  
  return MEMS_SUCCESS;
}


status_t OHM_GetPosAxesRaw(AxesRaw_t* buff) 
{
  int16_t value = 0;
  double fResultValue;
#if HEIGH_RESOLUTION  
  uint8_t *valueL = (uint8_t *)(&value);
#endif    
  uint8_t *valueH = ((uint8_t *)(&value)+1);

#if HEIGH_RESOLUTION  
  if( !OHM_ReadReg(OHM_OUT_X_L, valueL) )
    return MEMS_ERROR;
#endif

  if( !OHM_ReadReg(OHM_OUT_X_H, valueH) )
    return MEMS_ERROR;

  fResultValue = value;
  buff->AXIS_X = (int16_t)(fResultValue / 15.6);

#if HEIGH_RESOLUTION    
  if( !OHM_ReadReg(OHM_OUT_Y_L, valueL) )
    return MEMS_ERROR;
#endif

  if( !OHM_ReadReg(OHM_OUT_Y_H, valueH) )
    return MEMS_ERROR;
  
  fResultValue = value;
  buff->AXIS_Y = (int16_t)(fResultValue / 15.6);

#if HEIGH_RESOLUTION    
  if( !OHM_ReadReg(OHM_OUT_Z_L, valueL) )
    return MEMS_ERROR;
#endif

  if( !OHM_ReadReg(OHM_OUT_Z_H, valueH) )
    return MEMS_ERROR;

  fResultValue = value;
  buff->AXIS_Z = (int16_t)(fResultValue / 15.6);
  
  return MEMS_SUCCESS; 
}


status_t OHM_GetAccAxesRaw(AxesRaw_t* buff) 
{
  return OHM_GetPosAxesRaw(buff);
}


