#include <stdio.h>
#include<string.h>
#include "packet.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/FreeRTOS.h"

static const char *TAG = "UART TEST";

#define TEST_TXD 1 //(CONFIG_EXAMPLE_UART_TXD)
#define TEST_RXD 3 //(CONFIG_EXAMPLE_UART_RXD)
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)

#define UART_PORT_NUM                0 //(CONFIG_EXAMPLE_UART_PORT_NUM)
#define ECHO_UART_BAUD_RATE     115200 //(CONFIG_EXAMPLE_UART_BAUD_RATE)
#define ECHO_TASK_STACK_SIZE    2048   //(CONFIG_EXAMPLE_TASK_STACK_SIZE)

#define SSerialReceive 16          // GPIO4 = D2 pin <-> RS485 RO (Receive Out)
#define SSerialTransmit 17         // GPIO0 = D3 pin <-> RS485 DI (Data In)
// #define RS485DirectionControl 14  // GPI14 = D5 pin <-> RS485 DE (Data Enable) & RE (Receive Enable)

// #define BUF_SIZE 1024
#define RS485Transmit HIGH
#define RS485Receive LOW
#define BLINK_GPIO GPIO_NUM_2

Packet packet;  // last received pentair packet.

void uart_init(int uart_num, int tx_pin, int rx_pin)
{
    uart_config_t uart_config = 
    {
        .baud_rate = ECHO_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    ESP_ERROR_CHECK(uart_driver_install(uart_num, ECHO_TASK_STACK_SIZE*2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(uart_num, tx_pin, rx_pin, ECHO_TEST_RTS, ECHO_TEST_CTS));
}

void read_bytes()
{
    Packet in;
    char c = '\0';
    size_t availableBytes;
    bool send_ready = false;
    gpio_set_level(BLINK_GPIO,1);
    while(in.buffer_pos_ <= 251)
    {
        uart_get_buffered_data_len(UART_NUM_2, &availableBytes);
        uart_read_bytes(UART_NUM_2, &c, 1 ,portMAX_DELAY);
        // printf("%d\n",availableBytes);
        if(packet.Push(c))
        {
            // printf("Pakcet Pushed\n");
            availableBytes=0;
            if(packet.IsValid())
            {
                // printf("Packet Is Valid\n");
                send_ready = (packet.cid_ == COMMAND_PUMP_STATUS);
                packet.PrintTo();
            }
            else
            {
                printf("   ***   INVALID PACKET ***\n");
                packet.PrintTo();
            }
            packet.Reset();
            in.buffer_pos_ = 252;
        }  
    }
    gpio_set_level(BLINK_GPIO,0);
}

void print_cid()
{
    printf("To send a request to the Pool Controller, press any one of the following characters given below through your keyboard:\n"
            "\t\t\tPOOL_ON        = a,\n"
            "\t\t\tPOOL_OFF       = b,\n"
            "\t\t\tPOOL_LIGHT_ON  = c,\n"
            "\t\t\tPOOL_LIGHT_OFF = d,\n"
            "\t\t\tCLEANER_ON     = e,\n"
            "\t\t\tCLEANER_OFF    = f,\n"
            "\t\t\tWATER_FALL_ON  = g,\n"
            "\t\t\tWATER_FALL_OFF = h,\n"
            "\t\t\tSALT_PCT_QUERY = i,\n"
            "\t\t\tCHLOR_STEP_1   = j,\n"
            "\t\t\tCHLOR_STEP_2   = k,\n"
            "\t\t\tCHLOR_STEP_3   = l,\n"
            "\t\t\tCHLOR_STEP_4   = m,\n"
            "\t\t\tCHLOR_STEP_5   = n,\n"
            "\t\t\tCHLOR_STEP_6   = o,\n"
            "\t\t\tCHLOR_STEP_7   = p,\n"
            "\t\t\tCHLOR_STEP_8   = q,\n"
            "NOTE:THE CHARACTER COMMANDS ARE CASE-SENSITIVE.\n"
            "Enter your desired character now:\n");
}
void write_bytes()
{  
    gpio_set_level(BLINK_GPIO,1);
    char input = '\0';
    print_cid();
    int len = uart_read_bytes(UART_PORT_NUM, &input, 1, portMAX_DELAY);//100 / portTICK_RATE_MS
    if (len) 
    {
        ESP_LOGI(TAG, "%c\t", input);
    }

    switch((CIDNum)input)
    {    
        /*Enums*/
        // POOL_ON        = 'a',
        // POOL_OFF       = 'b',
        // POOL_LIGHT_ON  = 'c',
        // POOL_LIGHT_OFF = 'd',
        // CLEANER_ON     = 'e',
        // CLEANER_OFF    = 'f',
        // WATER_FALL_ON  = 'g',
        // WATER_FALL_OFF = 'h',
        // SALT_PCT_QUERY = 'i',
        // CHLOR_STEP_1   = 'j',
        // CHLOR_STEP_2   = 'k',
        // CHLOR_STEP_3   = 'l',
        // CHLOR_STEP_4   = 'm',
        // CHLOR_STEP_5   = 'n',
        // CHLOR_STEP_6   = 'o',
        // CHLOR_STEP_7   = 'p',
        // CHLOR_STEP_8   = 'q',
        
        case POOL_ON:
        {
            Packet out;
            // printf("\nI'm Here\n");
            out.Make(0x86,"06 01"); //Command ID, Data
            out.WriteTo();
            uart_flush(UART_NUM_2);
            printf("\n");
            break;
        }
        case POOL_OFF:
        {
            Packet out;
            out.Make(0x86,"06 00"); //Command ID, Data
            out.WriteTo();
            uart_flush(UART_NUM_2);
            printf("\n");
            break;
        }
        case POOL_LIGHT_ON:
        {
            Packet out;
            out.Make(0x86,"02 01"); //Command ID, Data
            out.WriteTo();
            uart_flush(UART_NUM_2);
            printf("\n");
            break;
        }
        case POOL_LIGHT_OFF:
        {
            Packet out;
            out.Make(0x86,"02 00"); //Command ID, Data
            out.WriteTo();
            uart_flush(UART_NUM_2);
            printf("\n");
            break;
        }
        case CLEANER_ON:
        {
            Packet out;
            out.Make(0x86,"01 01"); //Command ID, Data
            out.WriteTo();
            uart_flush(UART_NUM_2);
            printf("\n");
            break;
        }
        case CLEANER_OFF:
        {
            Packet out;
            out.Make(0x86,"01 00"); //Command ID, Data
            out.WriteTo();
            uart_flush(UART_NUM_2);
            printf("\n");
            break;
        }
        case WATER_FALL_ON:
        {
            Packet out;
            out.Make(0x86,"03 01"); //Command ID, Data
            out.WriteTo();
            uart_flush(UART_NUM_2);
            printf("\n");
            break;
        }
        case WATER_FALL_OFF:
        {
            Packet out;
            out.Make(0x86,"03 00"); //Command ID, Data
            out.WriteTo();
            uart_flush(UART_NUM_2);
            printf("\n");
            break;
        }
        case SALT_PCT_QUERY:
        {
            Packet out;
            out.Make(0xD9,"00"); //Command ID, Data
            out.WriteTo();
            uart_flush(UART_NUM_2);
            printf("\n");
            break;
        }
        case CHLOR_STEP_1:
        {
            Packet out;
            out.Make(0x99,"01 02 00 00"); //Command ID, Data
            out.WriteTo();
            uart_flush(UART_NUM_2);
            printf("\n");
            break;
        }
        case CHLOR_STEP_2:
        {
            Packet out;
            out.Make(0x99,"01 05 00 00"); //Command ID, Data
            out.WriteTo();
            uart_flush(UART_NUM_2);
            printf("\n");
            break;
        }
        case CHLOR_STEP_3:
        {
            Packet out;
            out.Make(0x99,"01 0A 00 00"); //Command ID, Data
            out.WriteTo();
            uart_flush(UART_NUM_2);
            printf("\n");
            break;
        }
        case CHLOR_STEP_4:
        {
            Packet out;
            out.Make(0x99,"01 0F 00 00"); //Command ID, Data
            out.WriteTo();
            uart_flush(UART_NUM_2);
            printf("\n");
            break;
        }
        case CHLOR_STEP_5:
        {
            Packet out;
            out.Make(0x99,"01 14 00 00"); //Command ID, Data
            out.WriteTo();
            uart_flush(UART_NUM_2);
            printf("\n");
            break;
        }
        case CHLOR_STEP_6:
        {
            Packet out;
            out.Make(0x99,"01 19 00 00"); //Command ID, Data
            out.WriteTo();
            uart_flush(UART_NUM_2);
            printf("\n");
            break;
        }
        case CHLOR_STEP_7:
        {
            Packet out;
            out.Make(0x99,"01 21 00 00"); //Command ID, Data
            out.WriteTo();
            uart_flush(UART_NUM_2);
            printf("\n");
            break;
        }
        case CHLOR_STEP_8:
        {
            Packet out;
            out.Make(0x99,"01 32 00 00"); //Command ID, Data
            out.WriteTo();
            uart_flush(UART_NUM_2);
            printf("\n");
            break;
        }
    }
    gpio_set_level(BLINK_GPIO,0);
    ESP_LOGI(TAG,"Reading");
    read_bytes();
}  

static void echo_task(void *arg)
{
    while(1)
    {
        ESP_LOGI(TAG,"Writing");
        write_bytes();
    }
}

extern "C" {void app_main();}

void app_main(void)
{
    /*GPIO config*/
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    // gpio_set_direction(RS485DirectionControl, GPIO_MODE_OUTPUT);
    // gpio_set_level(RS485DirectionControl,RS485Receive);
    uart_init(UART_PORT_NUM,TEST_TXD,TEST_RXD);
    uart_init(UART_NUM_2,SSerialTransmit,SSerialReceive);
    xTaskCreate(echo_task, "uart_echo_task", ECHO_TASK_STACK_SIZE*2, NULL, 10, NULL);
}