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

#define BUF_SIZE (1024)

    
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
TaskHandle_t xTask11Handle = NULL;
TaskHandle_t xTask12Handle = NULL;

TaskHandle_t xTask13Handle = NULL;
TaskHandle_t xTask14Handle = NULL;
TaskHandle_t xTask15Handle = NULL;

const gpio_num_t relay_pins[5] = {
    GPIO_NUM_23, GPIO_NUM_32, GPIO_NUM_25, GPIO_NUM_27, GPIO_NUM_18
};

// Пины для светодиодов
const gpio_num_t led_pins[5] = {
    GPIO_NUM_22, GPIO_NUM_33, GPIO_NUM_26, GPIO_NUM_2, GPIO_NUM_13
};

uint8_t byte;
int idx = 0;
uint8_t received_data[10]; // Блок для хранения 10 байт данных
uint8_t data[10];  // Буфер для принятия 10 байт

bool header_found = false;
int data_index = 0; //переменные для поиска нужного заголовка массива data

uint16_t unlock_time; // принятое значение времени открытия реле
uint16_t flash_time;  // принятое значение времени моргания светодиода
uint8_t id;// принятое значение ID, которое я определил как номер открываемого канала
uint8_t command;// принятое значение command, где 0х02 обозначает открытие канала
 
// Параметры управления
uint32_t relay_open_time; //переменная в которой сохраняется полученное значение время открытия реле в миллисекундах
uint32_t relay_1_open_time, relay_2_open_time, relay_3_open_time, relay_4_open_time, relay_5_open_time; //переменные времени открытия реле в миллисекундах для каждого канала
uint32_t led_blink_time; //переменная в которой сохраняется полученное значение времени мигания светодиода в миллисекундах
uint32_t led_1_blink_time, led_2_blink_time, led_3_blink_time, led_4_blink_time, led_5_blink_time;    //переменные времени мигания светодиодов в миллисекундах для каждого канала

int flag_1 = 0;
int flag_2 = 0;// вспомогательный флаг, для одноразового выполнения цикла преобразования принятых данных в значения для упрвления каналами
int can = 0;//переменная в которой сохраняется полученное значение ID, как выбор открываемого канала


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
void gpio_init()
{
    gpio_set_direction(relay_pins[0], GPIO_MODE_OUTPUT);
    gpio_set_direction(relay_pins[1], GPIO_MODE_OUTPUT);
    gpio_set_direction(relay_pins[2], GPIO_MODE_OUTPUT);
    gpio_set_direction(relay_pins[3], GPIO_MODE_OUTPUT);
    gpio_set_direction(relay_pins[4], GPIO_MODE_OUTPUT); 
    gpio_set_direction(led_pins[0], GPIO_MODE_OUTPUT);
    gpio_set_direction(led_pins[1], GPIO_MODE_OUTPUT);
    gpio_set_direction(led_pins[2], GPIO_MODE_OUTPUT);
    gpio_set_direction(led_pins[3], GPIO_MODE_OUTPUT);
    gpio_set_direction(led_pins[4], GPIO_MODE_OUTPUT);//настройка пинов для управления реле и светодиодами
    gpio_set_level(led_pins[0], 0); 
    gpio_set_level(led_pins[1], 0);  
    gpio_set_level(led_pins[2], 0); 
    gpio_set_level(led_pins[3], 0); 
    gpio_set_level(led_pins[4], 0); // Выключаем светодиоды 
    gpio_set_level(relay_pins[0], 0);
    gpio_set_level(relay_pins[1], 0); 
    gpio_set_level(relay_pins[2], 0); 
    gpio_set_level(relay_pins[3], 0); 
    gpio_set_level(relay_pins[4], 0); // Выключаем реле	
}


void relay_control_task(void *pvParameters) {
    int channel = (int)pvParameters;
   	switch (channel) 
 		{
        case 0:
		    while (1) 
		    {
				relay_1_open_time=relay_open_time;
		        // Включаем реле 1
		        gpio_set_level(relay_pins[channel], 1);
		        vTaskDelay(pdMS_TO_TICKS(relay_1_open_time));       
		        // Выключаем реле 1
		        gpio_set_level(relay_pins[channel], 0);
		
		    }
		break;
		case 1:
		    while (1) 
		    {
		        relay_2_open_time=relay_open_time;
		        // Включаем реле 2
		        gpio_set_level(relay_pins[channel], 1);
		        vTaskDelay(pdMS_TO_TICKS(relay_2_open_time));       
		        // Выключаем реле 2
		        gpio_set_level(relay_pins[channel], 0);
		    }
		break;
		case 2:
		    while (1) 
		    {
				relay_3_open_time=relay_open_time;
		        // Включаем реле 3
		        gpio_set_level(relay_pins[channel], 1);
		        vTaskDelay(pdMS_TO_TICKS(relay_3_open_time));       
		        // Выключаем реле 3
		        gpio_set_level(relay_pins[channel], 0);
		    }
		break;
		case 3:
		    while (1) 
		    {
				relay_4_open_time=relay_open_time;
		        // Включаем реле 4
		        gpio_set_level(relay_pins[channel], 1);
		        vTaskDelay(pdMS_TO_TICKS(relay_4_open_time));       
		        // Выключаем реле 4
		        gpio_set_level(relay_pins[channel], 0);
		    }
		break;
		case 4:
		    while (1) 
		    {
				relay_5_open_time=relay_open_time;
		        // Включаем реле 5
		        gpio_set_level(relay_pins[channel], 1);
		        vTaskDelay(pdMS_TO_TICKS(relay_5_open_time));       
		        // Выключаем реле 5
		        gpio_set_level(relay_pins[channel], 0);
		    }
		break;						
		}    
}

void led_blink_task(void *pvParameters) 
{
   int channel = (int)pvParameters;
   switch (channel) 
 	{
        case 0:
		    while (1) 
		    {
		        led_1_blink_time=led_blink_time;
				gpio_set_level(led_pins[channel], 1);
		        vTaskDelay(pdMS_TO_TICKS(led_1_blink_time));
		        // Выключаем светодиод      
		        gpio_set_level(led_pins[channel], 0);
		        vTaskDelay(pdMS_TO_TICKS(led_1_blink_time));
		    }
		break;
		case 1:
		    while (1) 
		    {
				led_2_blink_time=led_blink_time;
		        // Включаем светодиод
				gpio_set_level(led_pins[channel], 1);
		        vTaskDelay(pdMS_TO_TICKS(led_2_blink_time));
		        // Выключаем светодиод      
		        gpio_set_level(led_pins[channel], 0);
		        vTaskDelay(pdMS_TO_TICKS(led_2_blink_time));
		    }
		break;
		case 2:
		while (1) 
		    {
				led_3_blink_time=led_blink_time;
		        // Включаем светодиод
				gpio_set_level(led_pins[channel], 1);
		        vTaskDelay(pdMS_TO_TICKS(led_3_blink_time));
		        // Выключаем светодиод      
		        gpio_set_level(led_pins[channel], 0);
		        vTaskDelay(pdMS_TO_TICKS(led_3_blink_time));
		    }
		break;
		case 3:
		while (1) 
		    {
				led_4_blink_time=led_blink_time;
		        // Включаем светодиод
				gpio_set_level(led_pins[channel], 1);
		        vTaskDelay(pdMS_TO_TICKS(led_4_blink_time));
		        // Выключаем светодиод      
		        gpio_set_level(led_pins[channel], 0);
		        vTaskDelay(pdMS_TO_TICKS(led_4_blink_time));
		    }
		break;
		case 4:
		while (1) 
		    {
				led_5_blink_time=led_blink_time;
		        // Включаем светодиод
				gpio_set_level(led_pins[channel], 1);
		        vTaskDelay(pdMS_TO_TICKS(led_5_blink_time));
		        // Выключаем светодиод      
		        gpio_set_level(led_pins[channel], 0);
		        vTaskDelay(pdMS_TO_TICKS(led_5_blink_time));
		    }
		break;         
	}		    
}

void control_chanal_task(void *pvParameters) 
{
        int channel = (int)pvParameters;
        switch (channel) 
        {
        case 0:
        xTaskCreate(relay_control_task, "relay_1_control_task", 2048, (void *)channel, 5, &xTask1Handle);
        xTaskCreate(led_blink_task, "led_blink_1_task", 2048, (void *)channel, 5, &xTask2Handle);
        vTaskDelay(pdMS_TO_TICKS(relay_1_open_time));
        if (xTask2Handle != NULL) 
        {
        vTaskDelete(xTask2Handle);
        xTask2Handle = NULL; // Сбрасываем дескриптор функции управления светодиодом канала 1
        }
        if (xTask1Handle != NULL) 
        {
        vTaskDelete(xTask1Handle);
        xTask1Handle = NULL; // Сбрасываем дескриптор функции управления реле канала 1
        }
        gpio_set_level(relay_pins[channel], 0);
        gpio_set_level(led_pins[channel], 0);
        if (xTask3Handle != NULL) 
        {
        vTaskDelete(xTask3Handle);
        xTask3Handle = NULL; //  Сбрасываем дескриптор ункции управления канала 1
        } 
        break;  
        case 1:
        xTaskCreate(relay_control_task, "relay_2_control_task", 2048, (void *)channel, 5, &xTask4Handle);
        xTaskCreate(led_blink_task, "led_blink_2_task", 2048, (void *)channel, 5, &xTask5Handle);
        vTaskDelay(pdMS_TO_TICKS(relay_2_open_time));
        if (xTask5Handle != NULL) 
        {
        vTaskDelete(xTask5Handle);
        xTask5Handle = NULL; // Сбрасываем дескриптор функции управления светодиодом канала 2
        }
        if (xTask4Handle != NULL) 
        {
        vTaskDelete(xTask4Handle);
        xTask4Handle = NULL; // Сбрасываем дескриптор функции управления реле канала 2
        }
         gpio_set_level(relay_pins[channel], 0);
         gpio_set_level(led_pins[channel], 0);
        if (xTask6Handle != NULL) 
        {
        vTaskDelete(xTask6Handle);
        xTask6Handle = NULL; //  Сбрасываем дескриптор ункции управления канала 2 
        } 
        break;
        case 2:
        xTaskCreate(relay_control_task, "relay_3_control_task", 2048, (void *)channel, 5, &xTask7Handle);
        xTaskCreate(led_blink_task, "led_blink_3_task", 2048, (void *)channel, 5, &xTask8Handle);
        vTaskDelay(pdMS_TO_TICKS(relay_3_open_time));
        if (xTask8Handle != NULL) 
        {
        vTaskDelete(xTask8Handle);
        xTask8Handle = NULL; // Сбрасываем дескриптор функции управления светодиодом канала 3
        }
        if (xTask7Handle != NULL) 
        {
        vTaskDelete(xTask7Handle);
        xTask7Handle = NULL; // Сбрасываем дескриптор функции управления реле канала 3
        }
         gpio_set_level(relay_pins[channel], 0);
         gpio_set_level(led_pins[channel], 0);
        if (xTask9Handle != NULL) 
        {
        vTaskDelete(xTask9Handle);
        xTask9Handle = NULL; //  Сбрасываем дескриптор ункции управления канала 3
        }
        break;
        case 3:
        xTaskCreate(relay_control_task, "relay_4_control_task", 2048, (void *)channel, 5, &xTask10Handle);
        xTaskCreate(led_blink_task, "led_blink_4_task", 2048, (void *)channel, 5, &xTask11Handle);
        vTaskDelay(pdMS_TO_TICKS(relay_4_open_time));
         if (xTask11Handle != NULL) 
        {
        vTaskDelete(xTask11Handle);
        xTask11Handle = NULL; // Сбрасываем дескриптор функции управления светодиодом канала 4
        }
        if (xTask10Handle != NULL) 
        {
        vTaskDelete(xTask10Handle);
        xTask10Handle = NULL; // Сбрасываем дескриптор функции управления реле канала 4
        }
         gpio_set_level(relay_pins[channel], 0);
         gpio_set_level(led_pins[channel], 0);
        if (xTask12Handle != NULL) 
        {
        vTaskDelete(xTask12Handle);
        xTask12Handle = NULL; //  Сбрасываем дескриптор ункции управления канала 4
        }  
        
        break;     
        case 4:
        xTaskCreate(relay_control_task, "relay_5_control_task", 2048, (void *)channel, 5, &xTask13Handle);
        xTaskCreate(led_blink_task, "led_blink_5_task", 2048, (void *)channel, 5, &xTask14Handle);
        vTaskDelay(pdMS_TO_TICKS(relay_5_open_time));      
        if (xTask14Handle != NULL) 
        {
        vTaskDelete(xTask14Handle);
        xTask14Handle = NULL; // Сбрасываем дескриптор функции управления светодиодом канала 5
        }
        if (xTask13Handle != NULL) 
        {
        vTaskDelete(xTask13Handle);
        xTask13Handle = NULL; // Сбрасываем дескриптор функции управления реле канала 5
        gpio_set_level(relay_pins[channel], 0);
        gpio_set_level(led_pins[channel], 0);
        }
        if (xTask15Handle != NULL) 
        {
        vTaskDelete(xTask15Handle);
        xTask15Handle = NULL; // Сбрасываем дескриптор ункции управления канала 5
        } 
        break;     
        }
                        
}

void app_main(void) 
{
	uart_init();
    gpio_init();
   
    while (1) 
    {
		
		   while (flag_1==0)
		   {
           // Ожидание принятия данных
           int len  = uart_read_bytes(ECHO_UART_PORT_NUM, &byte, 1, 20 / portTICK_PERIOD_MS);
                
                if(byte != 0x02 && byte != 0x33)  continue;         
				     // Проверка на экранирующий символ
	            else if (byte == 0x02) 
	            {
	                // Чтение следующего байта
	        /*        int len1 = uart_read_bytes(ECHO_UART_PORT_NUM, &byte, 1, 20 / portTICK_PERIOD_MS);
	                if (len1 > 0) 
	                {
	                    // Проверка на заголовок
	                    if (byte == 0x33) 
	                    {
	                        header_found = true;
	                        idx = 0;
	                    }
	                }
	                len=0;*/
	             continue;  
	            } 
	            else if (byte == 0x33) 
	                    {
							flag_1=1;
	                        header_found = true;
	                        idx = 0;
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
						for(int i=0; i<10; i++) received_data[i]= 0;
						}
                }
     header_found = false;     

	        
        //vTaskDelay(pdMS_TO_TICKS(1));
    length=0;
 	while(flag_2==1) 
  	{
 	if(id==0x01) can=0;
 	if(id==0x02) can=1; 
 	if(id==0x03) can=2; 
 	if(id==0x04) can=3;   
 	if(id==0x05) can=4; //преобразование полученного значения в канал, для управления которым будут предназначены данные из принятой посылки
  	relay_open_time=(uint32_t)unlock_time;//
  	led_blink_time=(uint32_t)flash_time;//необязательные преобразования 

	    
		if  (can<5)
		{
			//printf("%" PRIu32"\n", relay_open_time);
	        // printf("%" PRIu32"\n", led_blink_time);   
	
	     switch (can) 
	        {
	        case 0:
	        xTaskCreate(control_chanal_task, "control_chanal_1_task", 2048, (void *)0, 5, &xTask3Handle);
	        relay_1_open_time=relay_open_time;
	        led_1_blink_time=led_blink_time; //запуск функции управления каналом 1
	        break;  
	        case 1:
	        xTaskCreate(control_chanal_task, "control_chanal_2_task", 2048, (void *)1, 5, &xTask6Handle);
	        relay_2_open_time=relay_open_time; 
	        led_2_blink_time=led_blink_time;//запуск функции управления каналом 2
	        break;
	        case 2:
	        xTaskCreate(control_chanal_task, "control_chanal_3_task", 2048, (void *)2, 5, &xTask9Handle);
	        relay_3_open_time=relay_open_time;
	        led_3_blink_time=led_blink_time; //запуск функции управления каналом 3
	        break;
	        case 3:
	        xTaskCreate(control_chanal_task, "control_chanal_4_task", 2048, (void *)3, 5, &xTask12Handle);
	        relay_4_open_time=relay_open_time;
	        led_4_blink_time=led_blink_time; //запуск функции управления каналом 4
	        break;     
	        case 4:
	        xTaskCreate(control_chanal_task, "control_chanal_5_task", 2048, (void *)4, 5, &xTask15Handle);
	        relay_5_open_time=relay_open_time;
	        led_5_blink_time=led_blink_time; //запуск функции управления каналом 5
	        break;     
	        }
	    
	    }
	    can=5;
	    	for(int i=0; i<10; i++)
	   	 	{
			data[i]= 0;
	    	}
	      flag_2=0;
	   	}
 	}
}