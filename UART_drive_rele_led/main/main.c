#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <inttypes.h>

#define ECHO_TEST_TXD (CONFIG_EXAMPLE_UART_TXD)
#define ECHO_TEST_RXD (CONFIG_EXAMPLE_UART_RXD)
#define ECHO_UART_PORT_NUM      (CONFIG_EXAMPLE_UART_PORT_NUM)
#define ECHO_UART_BAUD_RATE     (CONFIG_EXAMPLE_UART_BAUD_RATE)
#define ECHO_TASK_STACK_SIZE    (CONFIG_EXAMPLE_TASK_STACK_SIZE)

#define BUF_SIZE (4096)

#define LED1 22
#define LED2 32
#define LED3 25
#define LED4 27
#define LED5 18

#define RELAY1 23
#define RELAY2 33
#define RELAY3 26
#define RELAY4 2
#define RELAY5 23

TaskHandle_t xTask1Handle = NULL;
TaskHandle_t xTask2Handle = NULL;
TaskHandle_t xTask3Handle = NULL;
TaskHandle_t xTask4Handle = NULL;
TaskHandle_t xTask5Handle = NULL;
TaskHandle_t xTask6Handle = NULL;
TaskHandle_t xTask7Handle = NULL;
TaskHandle_t xTask8Handle = NULL;
TaskHandle_t xTask9Handle = NULL;
TaskHandle_t xTask10Handle = NULL;


uint8_t byte;
int idx = 0;
uint8_t received_data[11]; // Блок для хранения 10 байт данных
uint8_t data[11];  // Буфер для принятия 10 байт

bool header_found = false;
int data_index = 0; //переменные для поиска нужного заголовка массива data

uint16_t unlock_time; // принятое значение времени открытия реле
uint16_t flash_time;  // принятое значение времени моргания светодиода
uint8_t id;// принятое значение ID, которое я определил как номер открываемого канала
uint8_t command;// принятое значение command, где 0х02 обозначает открытие канала
 

uint32_t relay_open_time; //переменная в которой сохраняется полученное значение время открытия реле в миллисекундах
uint32_t relay_1_open_time=0, relay_2_open_time=0, relay_3_open_time=0, relay_4_open_time=0, relay_5_open_time=0; //переменные времени открытия реле в миллисекундах для каждого канала
uint32_t led_blink_time; //переменная в которой сохраняется полученное значение времени мигания светодиода в миллисекундах
uint32_t led_1_blink_time=0, led_2_blink_time=0, led_3_blink_time=0, led_4_blink_time=0, led_5_blink_time=0;    //переменные времени мигания светодиодов в миллисекундах для каждого канала
int flag_1 = 0, flag_2 = 0;   

//int flag_1 = 0;
//int flag_2 = 0;// вспомогательный флаг, для одноразового выполнения цикла преобразования принятых данных в значения для упрвления каналами
int can = 0;//переменная в которой сохраняется полученное значение ID, как выбор открываемого канала


void gpio_init()
{
    gpio_set_direction(LED1, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED2, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED3, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED4, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED5, GPIO_MODE_OUTPUT); 
    gpio_set_direction(RELAY1, GPIO_MODE_OUTPUT);
    gpio_set_direction(RELAY2, GPIO_MODE_OUTPUT);
    gpio_set_direction(RELAY3, GPIO_MODE_OUTPUT);
    gpio_set_direction(RELAY4, GPIO_MODE_OUTPUT);
    gpio_set_direction(RELAY5, GPIO_MODE_OUTPUT);// Настройка пинов      
    gpio_set_level(LED1, 0); 
    gpio_set_level(LED2, 0);  
    gpio_set_level(LED3, 0); 
    gpio_set_level(LED4, 0); 
    gpio_set_level(LED5, 0); // Выключаем светодиоды 
    gpio_set_level(RELAY1, 0);
    gpio_set_level(RELAY2, 0); 
    gpio_set_level(RELAY3, 0); 
    gpio_set_level(RELAY4, 0); 
    gpio_set_level(RELAY5, 0); // Выключаем реле
}

void uart_init()
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_driver_install(ECHO_UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(ECHO_UART_PORT_NUM, &uart_config);
    uart_set_pin(ECHO_UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
//настройка uart
}


void led_task_1(void *arg) 
{
int period=(int)arg;  
  
    while (1) 
    {
        gpio_set_level(LED1, 1);
        vTaskDelay(period / portTICK_PERIOD_MS);
        gpio_set_level(LED1, 0);
        vTaskDelay(period / portTICK_PERIOD_MS);   //моргаем светодиодом в канале 1          
    }
}

void relay_task_1(void *arg) 
{
 int period = (int)arg;
 xTaskCreate(led_task_1, "led_task_1", 2048, (void *)led_1_blink_time, 1, &xTask1Handle);
 //создаем задачу на моргание светодиодом в канале 1 
    while (1) 
      {
	        gpio_set_level(RELAY1, 1);
	        vTaskDelay(period / portTICK_PERIOD_MS);
	        gpio_set_level(RELAY1, 0); //Переключаем реле в канале 1
	        if (xTask1Handle != NULL) 
	        {
			gpio_set_level(LED1, 0); 	
	        vTaskDelete(xTask1Handle);
	        xTask1Handle = NULL; // Сбрасываем дескриптор функции управления светодиодом канала 1
	        }
	        if (xTask2Handle != NULL) 
	        {
	        gpio_set_level(RELAY1, 0);	
	        vTaskDelete(xTask2Handle);
	        xTask2Handle = NULL; // Сбрасываем дескриптор функции управления реле канала 1
	        }
       }
}

void led_task_2(void *arg) 
{
int period=(int)arg;  
  
    while (1) 
    {
        gpio_set_level(LED2, 1);
        vTaskDelay(period / portTICK_PERIOD_MS);
        gpio_set_level(LED2, 0);
        vTaskDelay(period / portTICK_PERIOD_MS);  //моргаем светодиодом в канале 2             
     }
}

void relay_task_2(void *arg) 
{
 int period = (int)arg;
 xTaskCreate(led_task_2, "led_task_2", 2048, (void *)led_2_blink_time, 1, &xTask3Handle);

    while (1) 
      {
	        gpio_set_level(RELAY2, 1);
	        vTaskDelay(period / portTICK_PERIOD_MS);
	        gpio_set_level(RELAY2, 0); //Переключаем реле в канале 2
	        if (xTask3Handle != NULL) 
	        {
			gpio_set_level(LED2, 0); 	
	        vTaskDelete(xTask3Handle);
	        xTask3Handle = NULL; // Сбрасываем дескриптор функции управления светодиодом канала 2
	        }
	        if (xTask4Handle != NULL) 
	        {
	        gpio_set_level(RELAY2, 0);	
	        vTaskDelete(xTask4Handle);
	        xTask4Handle = NULL; // Сбрасываем дескриптор функции управления реле канала 2
        	}
       }
}

void led_task_3(void *arg) 
{
int period=(int)arg;  
  
    while (1) 
    {
        gpio_set_level(LED3, 1);
        vTaskDelay(period / portTICK_PERIOD_MS);
        gpio_set_level(LED3, 0);
        vTaskDelay(period / portTICK_PERIOD_MS);   //моргаем светодиодом в канале 3            
     }
}

void relay_task_3(void *arg) 
{
 int period = (int)arg;
 xTaskCreate(led_task_3, "led_task_3", 2048, (void *)led_3_blink_time, 1, &xTask5Handle);

    while (1) 
      {
	        gpio_set_level(RELAY3, 1);
	        vTaskDelay(period / portTICK_PERIOD_MS);
	        gpio_set_level(RELAY3, 0); //Переключаем реле в канале 3
	        if (xTask5Handle != NULL) 
	        {
			gpio_set_level(LED3, 0); 	
	        vTaskDelete(xTask5Handle);
	        xTask5Handle = NULL; // Сбрасываем дескриптор функции управления светодиодом канала 3
	        }
	        if (xTask6Handle != NULL) 
	        {
	        gpio_set_level(RELAY3, 0);	
	        vTaskDelete(xTask6Handle);
	        xTask6Handle = NULL; // Сбрасываем дескриптор функции управления реле канала 3
        	}
       }
}

void led_task_4(void *arg) 
{
int period=(int)arg;  
  
    while (1) 
    {
        gpio_set_level(LED4, 1);
        vTaskDelay(period / portTICK_PERIOD_MS);
        gpio_set_level(LED4, 0);
        vTaskDelay(period / portTICK_PERIOD_MS); //моргаем светодиодом в канале 4              
     }
}

void relay_task_4(void *arg) 
{
 int period = (int)arg;
 xTaskCreate(led_task_4, "led_task_4", 2048, (void *)led_4_blink_time, 1, &xTask7Handle);

    while (1) 
      {
	        gpio_set_level(RELAY4, 1);
	        vTaskDelay(period / portTICK_PERIOD_MS);
	        gpio_set_level(RELAY4, 0); //Переключаем реле в канале 4
	        if (xTask7Handle != NULL) 
	        {
			gpio_set_level(LED4, 0); 	
	        vTaskDelete(xTask7Handle);
	        xTask7Handle = NULL; // Сбрасываем дескриптор функции управления светодиодом канала 4
	        }
	        if (xTask8Handle != NULL) 
	        {
	        gpio_set_level(RELAY4, 0);	
	        vTaskDelete(xTask8Handle);
	        xTask8Handle = NULL; // Сбрасываем дескриптор функции управления реле канала 4
        	}
       }
}

void led_task_5(void *arg) 
{
int period=(int)arg;  
  
    while (1) 
    {
        gpio_set_level(LED5, 1);
        vTaskDelay(period / portTICK_PERIOD_MS);
        gpio_set_level(LED5, 0);
        vTaskDelay(period / portTICK_PERIOD_MS); //моргаем светодиодом в канале 5               
     }
}

void relay_task_5(void *arg) 
{
 int period = (int)arg;
 xTaskCreate(led_task_5, "led_task_5", 2048, (void *)led_5_blink_time, 1, &xTask9Handle);

    while (1) 
      {
	        gpio_set_level(RELAY5, 1);
	        vTaskDelay(period / portTICK_PERIOD_MS);
	        gpio_set_level(RELAY5, 0); //Переключаем реле в канале 5
	        if (xTask9Handle != NULL) 
	        {
			gpio_set_level(LED5, 0); 	
	        vTaskDelete(xTask9Handle);
	        xTask9Handle = NULL; // Сбрасываем дескриптор функции управления светодиодом канала 5
	        }
	        if (xTask10Handle != NULL) 
	        {
	        gpio_set_level(RELAY5, 0);	
	        vTaskDelete(xTask10Handle);
	        xTask10Handle = NULL; // Сбрасываем дескриптор функции управления реле канала 5
        	}
       }
}

void app_main() 
{
 gpio_init();
 uart_init();
 
 
    while (1) 
    {
		
		while (flag_1==0)
		   {
           // Ожидание принятия данных
           int len  = uart_read_bytes(ECHO_UART_PORT_NUM, &byte, 1, 20 / portTICK_PERIOD_MS);
           if(len>0)
  				{     
                if(byte != 0x02 && byte != 0x33)  continue;         
				     // Проверка на экранирующий символ
	            if (byte == 0x02) continue; 
	            if (byte == 0x33) 
	                    {
							flag_1=1;
	                        header_found = true;
	                        idx = 0;
	                        len=0;
	                    }
	            }
		   }
		
		flag_1=0;
		
                 int length  = uart_read_bytes(ECHO_UART_PORT_NUM, received_data, sizeof(received_data), 20 / portTICK_PERIOD_MS);
                 if(length>0&&header_found == true)
                 {
					    if (received_data[0] == 0x01 && received_data[2] == 0x02 && received_data[7] == 0x33 && received_data[8] == 0x03) 
						{
						id = received_data[1];
						command = received_data[2];
						unlock_time = (received_data[3] << 8) | received_data[4]; // время открытия замка
						flash_time = (received_data[5] << 8) | received_data[6];  // время моргания светодиода
						uart_write_bytes(ECHO_UART_PORT_NUM, (const char *)&id, sizeof(id));
						//printf("%02x OK\n", id);  // Вывод принятого байта
						flag_2=1;
						}
						else
					    {
						printf("ERROR\n"); //отправка сообщения об ошибке в случае принятия неверной последовательности байт
						for(int i=0; i<9; i++) received_data[i]= 0;
						}
                }
     header_found = false;     
     length=0;
 	while(flag_2==1) 
  	{
 	if(id==0x01) can=1;
 	if(id==0x02) can=2; 
 	if(id==0x03) can=3; 
 	if(id==0x04) can=4;   
 	if(id==0x05) can=5; //преобразование полученного значения в канал, для управления которым будут предназначены данные из принятой посылки
  	
  	relay_open_time=(uint32_t)unlock_time;
  	led_blink_time=(uint32_t)flash_time;
	
	if  (can<5)
		{
			//printf("%" PRIu32"\n", relay_open_time);
	        //printf("%" PRIu32"\n", led_blink_time);   
	
	     switch (can) 
	        {
	        case 1:
	        
	        relay_1_open_time=relay_open_time;
	        led_1_blink_time=led_blink_time; //запуск функции управления каналом 1
	        xTaskCreate(relay_task_1, "relay_task_1", 2048, (void *)relay_1_open_time, 2, &xTask2Handle);
	        break;
	          
	        case 2:
	        
	        relay_2_open_time=relay_open_time; 
	        led_2_blink_time=led_blink_time;//запуск функции управления каналом 2
	        xTaskCreate(relay_task_2, "relay_task_2", 2048, (void *)relay_2_open_time, 2, &xTask4Handle);
	        break;
	        case 3:
	        
	        relay_3_open_time=relay_open_time;
	        led_3_blink_time=led_blink_time; //запуск функции управления каналом 3
	        xTaskCreate(relay_task_3, "relay_task_3", 2048, (void *)relay_3_open_time, 2, &xTask6Handle);
	        break;
	        case 4:
	        
	        relay_4_open_time=relay_open_time;
	        led_4_blink_time=led_blink_time; //запуск функции управления каналом 4
	        xTaskCreate(relay_task_4, "relay_task_4", 2048, (void *)relay_4_open_time, 2, &xTask8Handle);
	        break;     
	        case 5:
	        
	        relay_5_open_time=relay_open_time;
	        led_5_blink_time=led_blink_time; //запуск функции управления каналом 5
	        xTaskCreate(relay_task_5, "relay_task_5", 2048, (void *)relay_5_open_time, 2, &xTask10Handle);
	        break;     
	        }
	    
	    }
	    can=5;
	    	for(int i=0; i<11; i++)
	   	 	{
			data[i]= 0;
	    	}
	      flag_2=0;
	   	}	
   }
}
