#include "wifip.h" 
//#include "ee.h"
//#include "eeConfig.h"
//#include <string.h>
#ifdef Wifi 
#include "wifi.h"
#include "mcu_api.h" 
#else
#include "zigbee.h"
#include "mcu_api.h" 
#endif

#define TP1 							HAL_GPIO_ReadPin(TOUCH_PAD1_GPIO_Port,TOUCH_PAD1_Pin)
#define TP2 							HAL_GPIO_ReadPin(TOUCH_PAD2_GPIO_Port,TOUCH_PAD2_Pin)
#define TP3 							HAL_GPIO_ReadPin(TOUCH_PAD3_GPIO_Port,TOUCH_PAD3_Pin)
#define SENSOR 				    HAL_GPIO_ReadPin(SENSOR_GPIO_Port,SENSOR_Pin)
#define SW1_BLUE					SW1_LED_BLUE_GPIO_Port,SW1_LED_BLUE_Pin
#define SW1_RED						SW1_LED_RED_GPIO_Port,SW1_LED_RED_Pin
#define SW2_BLUE					SW2_LED_BLUE_GPIO_Port,SW2_LED_BLUE_Pin
#define SW2_RED						SW2_LED_RED_GPIO_Port,SW2_LED_RED_Pin
#define SW3_BLUE					SW3_LED_BLUE_GPIO_Port,SW3_LED_BLUE_Pin
#define SW3_RED						SW3_LED_RED_GPIO_Port,SW3_LED_RED_Pin
#define SENSOR_MD			    LED_RED_GPIO_Port, LED_RED_Pin
#define TOUCH_MD					LED_BLUE_GPIO_Port,LED_BLUE_Pin
#define TOUCH_DK 					TOUCH_DK_GPIO_Port,TOUCH_DK_Pin
#define TB1								RELAY1_GPIO_Port,RELAY1_Pin
#define TB2								RELAY2_GPIO_Port,RELAY2_Pin
#define TB3								RELAY3_GPIO_Port,RELAY3_Pin
#define TB4								RELAY4_GPIO_Port,RELAY4_Pin

extern UART_HandleTypeDef huart1;
uint8_t Nhanbuff=0,state_receive = 0;
extern unsigned char out_dl;
void Uart_PutChar(unsigned char value)
{
	HAL_UART_AbortReceive_IT(&huart1);
	HAL_UART_Transmit(&huart1,&value,1,100);
	HAL_UART_Receive_IT(&huart1,&Nhanbuff,1);
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance==huart1.Instance)//nhan du lieu uart1
	{
		uart_receive_input(Nhanbuff);
		HAL_UART_Receive_IT(&huart1,&Nhanbuff,1);
	}
}

unsigned char wifi_state;

volatile unsigned char State_switch_1;
volatile unsigned char State_switch_2;
volatile unsigned char State_switch_3;
volatile unsigned char State_switch_4;
volatile unsigned char chon_cd=0;
volatile unsigned char State_sensor,run_countdown1,run_countdown2;
volatile unsigned char time_sensor,light_led_ss=0,use_ss,dk_app;
unsigned char use_touch,use_sensor,led_sensor_blink;
extern unsigned long countdown_1;
extern unsigned long countdown_2;
extern unsigned long countdown_3;
extern unsigned long countdown_4;
volatile uint16_t count_1s = 0;
unsigned long State_countdown_1;
unsigned long State_countdown_2;
unsigned long State_countdown_3;
unsigned long State_countdown_4;
unsigned long State_thong_so1; // thong so add vao cong suat toi thieu 10 wh
unsigned long State_thong_so2; // thong so dong dien don vi mA
unsigned long State_thong_so3; // Thong so cong suat don vi chia 10 ra W
unsigned long State_thong_so4; // thong so dien ap chia 10 ra vol
unsigned long State_thong_so1_count; // thong so add vao cong suat toi thieu 10 wh
unsigned long State_thong_so2_count; // thong so dong dien don vi mA
unsigned long State_thong_so3_count; // Thong so cong suat don vi chia 10 ra W
unsigned long State_thong_so4_count; // thong so dien ap chia 10 ra vol
float diennang = 0;
unsigned char dodienap = 0,nead_update_dienanng = 0;
uint16_t count_nhay = 0,count_update = 0,count_setup = 0,time_count_setup = 0,count_reset_heart = 0;
volatile uint16_t count_wifi_status = 0,count_blink_1s = 0,modeconfig = 0,timeout_config = 0,count_wifi_status_blink = 0,
									old_pad1 = 0,old_pad2 = 0,old_pad3 = 0,old_pad4 = 0,count_config_wifi = 0,count_config_wifi1 =0,count_config_wifi2 =0,state_config = 0,old_state1 = 0,
									old_state2 = 0,old_state3 = 0,old_state4 = 0,timeout_update_rf = 0,count_reset_touch = 0,time_count_reset_touch = 0,flag_reset_touch = 0,
									cycle_count_reset_touch = 0,time_update_all=0;
	static uint8_t has_change_touchpad = 0,old_button = 0;
uint16_t chophep_up=0;
void coundown_process(void)
{
	if(run_countdown2==1 )
	{
		if(use_ss==1)
		{
	if(count_1s >= 1000)
	{
		//count_1s =0;
		if(State_countdown_3 > 0 )
		{	
			if(State_countdown_3 >1)
			{
				State_countdown_3 --;
			}
			else //neu dung bang 1 thi togle thiet bi
			{
				State_switch_3 = 0;
				mcu_dp_bool_update(DPID_SWITCH_3,State_switch_3);
				State_countdown_3 = countdown_3;
				State_sensor=0;
				run_countdown1=0;
				run_countdown2=0;
				use_ss=0;	
			}		
	}
		count_1s =0;
}
	else
	{
		count_1s ++;
	}
}
		}
	}


void wifiprocess(void)
{
	#ifdef Wifi
		wifi_uart_service();
		wifi_state = mcu_get_wifi_work_state();
	#else
		zigbee_uart_service();
	#endif
		/////count cho update data;
	if(time_update_all>=5000)
	{
		all_data_update();
		time_update_all=0;
	}
	else
	{
	time_update_all++;
	}
		HAL_UART_Receive_IT(&huart1,&Nhanbuff,1);
//		//het timeout cua count setup
//		
		if(time_count_setup> 120)	//xoa gia tri sau 100 lan dem
		{
			count_setup = 0;
			time_count_setup = 0;
		}
		else
		{
			time_count_setup++;
		}
		
		//count cho blink cac che do
		if(count_blink_1s >= 40) // nhan giu du 200 nhung doi blink lon hon "count_blink_1s" moi nhay led
			// thoi gian dao trang thai led
		{
				count_blink_1s = 0;
			if(modeconfig == 1)  // che do config 
			{		
				HAL_GPIO_WritePin(SW1_RED,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(SW2_RED,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(SW3_RED,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(SENSOR_MD,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(TOUCH_MD,GPIO_PIN_RESET);
				//neu o cho do config thi nhay cac led len
				if(timeout_config > 30)  // nhay trong 30 lan dem se thoat nhay led
				{
					modeconfig = 0;
					timeout_config = 0;
				}
				else
				{
					timeout_config++;
				}
				HAL_GPIO_TogglePin(SW1_BLUE);
				HAL_GPIO_TogglePin(SW2_BLUE);
				HAL_GPIO_TogglePin(SW3_BLUE);
			}
			else
			{
				modeconfig = 0;
				timeout_config = 0;
			}
			}
		else
		{
			count_blink_1s ++;
		}
		
			if(TP1 == GPIO_PIN_RESET ) //sw1
		{				
			time_count_setup = 0;  //xoa count_reset sau 100 lan
			cycle_count_reset_touch = 0;  // reset touch
			count_reset_touch=0; //reset touch auto
			if(count_setup == 6)//config mode
			{
				if(count_config_wifi >200)
				{
				count_config_wifi  = 200 ;
					#ifdef Wifi
				mcu_set_wifi_mode(0);
					#else
				mcu_network_start();
					#endif		
				modeconfig = 1; // che do cho nhay led luan phien	
				count_setup=0;
				}
				else
				{
				count_config_wifi ++;
				}
			}	
			if(old_pad1 == 0)
			{
				old_pad1 = 1;		
				if(State_switch_1 == 0)  
				{
					State_switch_1 = 1;
				}
				else
				{
					State_switch_1 = 0;
				}
				mcu_dp_bool_update(DPID_SWITCH_1,State_switch_1); // update trang thai len phan mem		
				count_setup ++;
			}
		}
	else
		{
			old_pad1 = 0;
			count_config_wifi  = 0;
		}

	if(TP2 == GPIO_PIN_RESET ) //sw2
		{				
			time_count_setup = 0;  //xoa count_reset sau 100 lan
			cycle_count_reset_touch = 0;  // reset touch
			count_reset_touch=0; //reset touch auto
			if(count_setup == 6)//config mode
			{
				if(count_config_wifi1 >200)
				{
				count_config_wifi1  = 200 ;
					#ifdef Wifi
				mcu_set_wifi_mode(0);
					#else
				mcu_network_start();
					#endif		
				modeconfig = 1; // che do cho nhay led luan phien	
				count_setup=0;
				}
				else
				{
				count_config_wifi1 ++;
				}
			}	
			if(old_pad2 == 0)
			{
				old_pad2 = 1;		
				if(State_switch_2 == 0)  
				{
					State_switch_2 = 1;
				}
				else
				{
					State_switch_2 = 0;
				}
				mcu_dp_bool_update(DPID_SWITCH_2,State_switch_2); // update trang thai len phan mem		
				count_setup ++;
			}
		}
	else
		{
			old_pad2 = 0;
			count_config_wifi1  = 0;
		}	

		if(TP3 == GPIO_PIN_RESET )   // sw config mode 
		{				
			time_count_setup = 0;  //xoa count_reset sau 100 lan
			cycle_count_reset_touch = 0;  // reset touch
			count_reset_touch=0; //reset touch auto
			
				if(count_setup == 1)
			{
				if(count_config_wifi2>=300)
				{
					count_config_wifi2=300;
					if(chon_cd==0)
					{
						chon_cd=1; // use touch
					}
					else
					{
						chon_cd=0;  // use sensor
					}
					count_setup =0;
			}
					else
				{
				count_config_wifi2++;
				}
			}
				 else if(count_setup == 6)//config mode
			{
				if(count_config_wifi2>200)
				{
				count_config_wifi2 =200 ;
					#ifdef Wifi
				mcu_set_wifi_mode(0);
					#else
				mcu_network_start();
					#endif
				modeconfig = 1; // che do cho nhay led luan phien
				count_setup=0;
				}
				else
				{
				count_config_wifi2++;
				}
			}	
						if(old_pad3 == 0)
			{
				old_pad3 = 1;		
				if(chon_cd==0)
{
				if(State_switch_3 == 0)  
				{
					State_switch_3 = 1;
				}
				else
				{
					State_switch_3 = 0;
				}
}
				mcu_dp_bool_update(DPID_SWITCH_3,State_switch_3); // update trang thai len phan mem		
				count_setup ++;
			}
		}
	else
		{
			old_pad3 = 0;
			count_config_wifi2  = 0;
		}	
		
		//------------------------
		if(chon_cd ==0) //use touch 
		{
			if(modeconfig == 0)
			{
				HAL_GPIO_WritePin(TOUCH_MD,GPIO_PIN_SET);
				HAL_GPIO_WritePin(SENSOR_MD,GPIO_PIN_RESET);
			}
		}
		else   //use sensor
		{
			if(modeconfig == 0)
			{
				if(SENSOR == GPIO_PIN_RESET)
				{
				HAL_GPIO_WritePin(SENSOR_MD ,GPIO_PIN_SET);
				HAL_GPIO_WritePin(TOUCH_MD,GPIO_PIN_RESET);
				}
				HAL_GPIO_WritePin(TOUCH_MD,GPIO_PIN_RESET);
//				HAL_GPIO_WritePin(SW2_RED ,GPIO_PIN_RESET);
//				HAL_GPIO_WritePin(SW2_BLUE,GPIO_PIN_SET);		
		if(SENSOR == GPIO_PIN_SET)
		{	
				State_switch_3 = 1;
				mcu_dp_bool_update(DPID_SWITCH_3,State_switch_3); // update trang thai len phan mem
				use_ss=1;
				if(countdown_3==0 )
			{
			//State_countdown_3=60;
				State_countdown_3=30;
			}
			else
			{
				State_countdown_3=countdown_3;
			}	
				if(time_sensor>=15)
				{
				HAL_GPIO_TogglePin(SENSOR_MD);
				time_sensor=0;
				}
				else
				{
				time_sensor++;	
				}
			}
		else
		{	
				HAL_GPIO_WritePin(SENSOR_MD,GPIO_PIN_SET);	
				run_countdown2=1;	
		}
	}
}
   if( State_switch_1 == 1)   // tb1
		{
			HAL_GPIO_WritePin(TB1,GPIO_PIN_SET);
			if(modeconfig == 0)
			{
				dk_app=0;
				HAL_GPIO_WritePin(SW1_RED,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(SW1_BLUE,GPIO_PIN_SET);
			}
		}
		else
		{
			HAL_GPIO_WritePin(TB1,GPIO_PIN_RESET);
			if(modeconfig == 0)
			{
				dk_app=0;
				HAL_GPIO_WritePin(SW1_RED,GPIO_PIN_SET);
				HAL_GPIO_WritePin(SW1_BLUE,GPIO_PIN_RESET);
			}
		}
		
	   if( State_switch_2 == 1)   // tb2
		{
			HAL_GPIO_WritePin(TB2,GPIO_PIN_SET);
			if(modeconfig == 0)
			{
				HAL_GPIO_WritePin(SW2_RED,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(SW2_BLUE,GPIO_PIN_SET);
			}
		}
		else
		{
			HAL_GPIO_WritePin(TB2,GPIO_PIN_RESET);
			if(modeconfig == 0)
			{
				dk_app=0;
				HAL_GPIO_WritePin(SW2_RED,GPIO_PIN_SET);
				HAL_GPIO_WritePin(SW2_BLUE,GPIO_PIN_RESET);
			}
		}
		
	   if( State_switch_3 == 1)   // tb3
		{
			HAL_GPIO_WritePin(TB3,GPIO_PIN_SET);
			if(modeconfig == 0)
			{
				HAL_GPIO_WritePin(SW3_RED,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(SW3_BLUE,GPIO_PIN_SET);
			}
		}
		else
		{
			HAL_GPIO_WritePin(TB3,GPIO_PIN_RESET);
			if(modeconfig == 0)
			{
				HAL_GPIO_WritePin(SW3_RED,GPIO_PIN_SET);
				HAL_GPIO_WritePin(SW3_BLUE,GPIO_PIN_RESET);
			}
		}
}
		
void wifi_init(void)
{
	HAL_GPIO_WritePin(ESP_RESET_GPIO_Port,ESP_RESET_Pin,GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(ESP_RESET_GPIO_Port,ESP_RESET_Pin,GPIO_PIN_SET);
	HAL_Delay(1000);
	HAL_UART_Receive_IT(&huart1,&Nhanbuff,1);
	#ifdef Wifi
	wifi_protocol_init();
	#else
	zigbee_protocol_init();
	#endif
}