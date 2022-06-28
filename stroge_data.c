#include <stdio.h>
#include <stdint.h>
#include <float.h>
#include "nrf_log.h"
#include "stroge_data.h"
#define  DETLA_TEMPERATURE 40
typedef  struct storget{

  uint16_t temp_data ; 
  uint32_t adc_to_temp_data ; 
  uint32_t between_two_value_defference ; 
}storget_data ; 


static storget_data gap_target_data[]={
{0 ,9520,29 },
{1 ,9491,26 },
{2 ,9465,52 },
{3 ,9413,52 },
{4 ,9361,41 },
{5 ,9320,43 },
{6 ,9277,37 },
{7 ,9240,34 },
{8 ,9206,41 },
{9 ,9165,45 },
{10,9120,48 },
{11,9072,44 },
{12,9028,82 },
{13,8946,51 },
{14,8895,49 },
{15,8846,45 },
{16,8801,29 },
{17,8772,29 },
{18,8743,40 },
{19,8703,51 },
{20,8652,52 },
{21,8600,39 },
{22,8561,60 },
{23,8501,51 },
{24,8450,60 },
{25,8390,57 },
{26,8333,73 },
{27,8260,63 },
{28,8173,77 },
{29,8086,101},
{30,7999,75 },
{31,7912,104},
{32,7825,90 },
{33,7738,110},
{34,7651,90 },
{35,7564,100},
{36,7477,110},
{37,7390,100},
{38,7303,120},
{39,7216,109},
{40,7129,101},
{41,7042,90 },
{42,6955,100},
{43,6868,100},
{44,6781,90 },
{45,6694,99 },
{46,6607,91 },
{47,6520,109},
{48,6433,81 },
{49,6346,70 },
{50,6259,60 },
{51,6172,60 },
{52,6085,47 },
{53,5998,33 },
{54,5911,30 },
{55,5850,93 },
{56,5757,93 },
{57,5664,93 },
{58,5571,93 },
{59,5478,93 },
{60,5385,93 },
{61,5292,93 },
{62,5199,93 },
{63,5106,93 },
{64,5013,93 },
{65,4920,93 },
{66,4827,93 },
{67,4734,84 },
{68,4650,70 },
{69,4580,70 },
{70,4510,107},
{71,4403,103},
{72,4300,100},
{73,4200,80 },
{74,4150,90 },
{75,4080,84 },
{76,3990,99 },
{77,3920,67 },
{78,3810,89 },
{79,3750,82 },
{80,3690,72 },
{81,3600,97 },
{82,3540,60 },
{83,3460,80 },
{84,3400,79 },
{85,3310,81 },
{86,3250,70 },
{87,3180,70 },
{88,3085,85 },
{89,3020,55 },
{90,2960,80 },
{91,2878,65 },
{92,2800,70 },
{93,2754,57 },
{94,2675,62 },
{95,2600,69 },
{96,2537,57 },
{97,2465,80 },
{98,2415,50 },
{99,2345,60 },
{100,2270,46},
{101,2160,62},
{102,2102,57},
{103,2065,45},
{104,2020,60},
{105,1980,40},
{106,1940,40},
{107,1885,50},
{108,1847,75},
{109,1775,35},
{110,1700,40},
{111,1660,50},
{112,1610,50},
{113,1560,30},
{114,1530,34},
{115,1496,56},
{116,1440,60},
{117,1380,50},
{118,1330,32},
{119,1298,44},
{120,1254,46}
}; 


static float adc_value_to_temp_situation(uint32_t big_measure_value , uint32_t small_measure_value ,uint16_t samle_value_temp,float target);
float search_data(float target)
{
  //target *100 ; 
   uint32_t target_val = (uint32_t)target; 

  //int 
   int left = 0 ;
   int right = sizeof(gap_target_data) / sizeof(gap_target_data[0]) -1 ;
   while(left < right)
   {
      int mid = (left + right) /2 ;
       uint32_t res = gap_target_data[mid].adc_to_temp_data;
      if(target_val == res)
      {
        return gap_target_data[mid].temp_data - DETLA_TEMPERATURE ; 
        
      }
      //当前值大于目标值并且下一个值小于目标值
      else if(gap_target_data[mid].adc_to_temp_data > target_val && gap_target_data[mid+1].adc_to_temp_data < target_val)
      {
        //printf("two temp,temp1 :%d,temp2:%d\n",gap_target_data[mid].temp_data ,gap_target_data[mid+1].temp_data );

        return adc_value_to_temp_situation(gap_target_data[mid].adc_to_temp_data,gap_target_data[mid+1].adc_to_temp_data,gap_target_data[mid].temp_data,target_val);
      }
      //当前值小于目标值并且上一个值大于目标值
      else if(gap_target_data[mid].adc_to_temp_data < target_val && gap_target_data[mid-1].adc_to_temp_data > target_val)
      {
        //printf("two temp_value , value1:%d,value2:%d\n",gap_target_data[mid-1].temp_data,gap_target_data[mid].temp_data);
        
        return adc_value_to_temp_situation(gap_target_data[mid-1].adc_to_temp_data,gap_target_data[mid].adc_to_temp_data,gap_target_data[mid-1].temp_data,target_val);
      }
      else if(gap_target_data[mid].adc_to_temp_data > target_val)
      {
        left = mid +1 ; 
      }
      else 
      {
        right = mid  ; 
      }
   }
}

//number1 > number2 and number1 > target , number2 < target , val == number1.temp

//onec situation
static float adc_value_to_temp_situation(uint32_t big_measure_value , uint32_t small_measure_value ,uint16_t samle_value_temp,float target)
{
  float ans =(1.0-((float)(target - small_measure_value))/(float)((big_measure_value - small_measure_value)))+ samle_value_temp - DETLA_TEMPERATURE ;
  //NRF_LOG_INFO("measure temp value :%d\n",(int)(ans * 100)); 
  return ans ; 
}

