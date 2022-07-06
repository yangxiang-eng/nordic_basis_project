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
{1	,9800	,40 },
{2	,9760	,42 },
{3	,9718	,43 },
{4	,9675	,45 },
{5	,9630	,46 },
{6	,9584	,47 },
{7	,9537	,48 },
{8	,9489	,49 },
{9	,9440	,50 },
{10	,9390	,51 },
{11	,9339	,51 },
{12	,9288	,52 },
{13	,9236	,53 },
{14	,9183	,53 },
{15	,9130	,54 },
{16	,9076	,54 },
{17	,9022	,56 },
{18	,8966	,57 },
{19	,8909	,59 },
{20	,8850	,61 },
{21	,8789	,61 },
{22	,8728	,62 },
{23	,8666	,62 },
{24	,8604	,63 },
{25	,8541	,64 },
{26	,8477	,65 },
{27	,8412	,66 },
{28	,8346	,67 },
{29	,8273	,71 },
{30	,8202	,73 },
{31	,8129	,74 },
{32	,8055	,76 },
{33	,7979	,78 },
{34	,7901	,79 },
{35	,7822	,80 },
{36	,7742	,82 },
{37	,7660	,83 },
{38	,7577	,85 },
{39	,7492	,86 },
{40	,7406	,87 },
{41	,7319	,88 },
{42	,7231	,90 },
{43	,7141	,91 },
{44	,7050	,92 },
{45	,6958	,92 },
{46	,6866	,94 },
{47	,6772	,94 },
{48	,6678	,96 },
{49	,6582	,96 },
{50	,6486	,96 },
{51	,6390	,98 },
{52	,6292	,97 },
{53	,6195	,98 },
{54	,6195	,99 },
{55	,6195	,100},
{56	,6195	,101},
{57	,6195	,101},
{58	,6195	,102},
{59	,6195	,103},
{60	,6195	,104},
{61	,6195	,104},
{62	,6195	,103},
{63	,6195	,103},
{64	,6195	,102},
{65	,6195	,102},
{66	,6195	,102},
{67	,6195	,101},
{68	,6195	,101},
{69	,6195	,100},
{70	,4469	,99 },
{71	,4371	,98 },
{72	,4277	,94 },
{73	,4184	,93 },
{74	,4092	,92 },
{75	,4002	,90 },
{76	,3916	,86 },
{77	,3832	,84 },
{78	,3750	,82 },
{79	,3669	,81 },
{80	,3589	,80 },
{81	,3511	,78 },
{82	,3434	,77 },
{83	,3358	,76 },
{84	,3283	,75 },
{85	,3209	,74 },
{86	,3136	,73 },
{87	,3064	,72 },
{88	,2993	,71 },
{89	,2923	,70 },
{90	,2854	,69 },
{91	,2786	,68 },
{92	,2719	,67 },
{93	,2653	,66 },
{94	,2588	,65 },
{95	,2524	,64 },
{96	,2461	,63 },
{97	,2399	,62 },
{98	,2338	,61 },
{99	,2278	,60 },
{100	,2219	,59 },
{101	,2158	,60 },
{102	,2098	,59 },
{103	,2039	,57 },
{104	,1982	,57 },
{105	,1925	,56 },
{106	,1869	,56 },
{107	,1813	,55 },
{108	,1758	,54 },
{109	,1704	,54 },
{110	,1650	,53 },
{111	,1597	,53 },
{112	,1544	,52 },
{113	,1492	,52 },
{114	,1440	,51 },
{115	,1389	,51 },
{116	,1338	,50 },
{117	,1288	,49 },
{118	,1238	,48 },
{119	,1189	,47 },
{120	,1140	,46 },

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

