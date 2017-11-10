/*******************************************************************************
 * (c) Copyright 2016-2017 Microsemi SoC Products Group. All rights reserved.
 * 
 * This SoftConsole example project demonstrates how to use configure and use the
 * CoreRISCV_AXI4 system timer.
 *
 * Please refer README.TXT in the root folder of this project for more details.
 */
#include "riscv_hal.h"
#include "hw_platform.h"
#include "core_uart_apb.h"
#include "core_gpio.h"

//Constants

const char SSID[] = "Microsemi_Guest";
const char password[] = "";

const char port[] = "8080";
const char msg_hdr[] = "+IPD";

// responses to parse
const uint8_t OK = 1;
const uint8_t ERROR = 2;
const uint8_t NO_CHANGE = 3;
const uint8_t FAIL = 4;
const uint8_t READY = 5;

uint8_t CR = 0x0D;
uint8_t LF = 0x0A;
uint8_t SQ = 0x22;
uint8_t SC = 0x2C;
#define NEW_CONFIGURATION

uint8_t rx_buff[3000];
uint8_t data_ready;
uint8_t tmp;
uint16_t data_len = 0;
uint8_t received_data[16], ip_address[16];

const char * g_hello_msg =
"\r\n PolarFire WiFi3 Click Ping Example Project. \n\r";

/*-----------------------------------------------------------------------------
 * UART instance data.
 */
UART_instance_t g_uart;
UART_instance_t wifi_uart;
  
volatile uint8_t cmd_size = 0;
uint8_t tx_data[500] = {0x00};
uint16_t tx_cnt = 0;
/*-----------------------------------------------------------------------------
 * GPIO instance data.
 */

gpio_instance_t g_gpio_in;
gpio_instance_t g_gpio_out;

uint8_t uart_rx_isr( void );
/*-----------------------------------------------------------------------------
 * Global state counter.
 */
uint32_t g_state = 1;

void delay_ms(uint32_t temp)
{
	volatile uint32_t count = 0;
	for(count = 0; count < (temp*2200); count++);
}

static uint8_t i = 0;
uint8_t External_29_IRQHandler(void)
{
	uint8_t status = 0;

    status = uart_rx_isr();
    if(i == 0)
    {
    	i++;
    	return (EXT_IRQ_DISABLE);
    }
    else
    {
    	return status;
    }
}

/******************************************************************************
 * UART receiver interrupt service routine.
 **********************************
 *****************************************************************************/
uint8_t linefreed = 0;
uint8_t tmp = 0;
uint8_t rx_data[64] = {0x00};
volatile uint16_t rx_cnt = 0;
char state;
char response_rcvd;
char responseID, response = 0;

uint8_t uart_rx_isr( void )
{
    volatile uint8_t rx_size = 0;
	uint8_t status = 0;

	/**********************************************************************
	 * Read data received by the UART.
	 *********************************************************************/
	rx_size = UART_get_rx( &wifi_uart, &rx_data[0], 1);
	if(rx_size != 0)
	{
		switch(rx_data[0])
		{
		  case 0x0A:
			 if(data_len > cmd_size)
			 {
			   data_ready = 1;
			   status = EXT_IRQ_DISABLE;
			 }
			 break;

		  case 0x0D:
			 rx_buff[data_len] = 0;
			 break;

		  default:
			  {
				  rx_buff[data_len] = rx_data[0];
				  data_len++;
				  rx_size--;
			  }
		}
	}
}


void calculate_cmd_size(uint8_t *CMD)
{
	uint8_t *ptr = CMD;

	while(*ptr)
	{
		ptr++;
		cmd_size++;
	}
}

void UART_Write_AT(uint8_t *CMD)
{
   uint8_t data = 0;

   data_len = 0;
   calculate_cmd_size(CMD);
   UART_polled_tx_string(&wifi_uart, CMD);
   data = 0x0D;
   UART_send(&wifi_uart, &data, 1);
   data = 0x0A;
   UART_send(&wifi_uart, &data, 1);
}

void reset_buff()
{
    memset(rx_buff,0,sizeof(rx_buff));
    data_ready = 0;
    data_len = 0;
    cmd_size = 0;
}

/* Read the response from WiFi3 Click using UART polled method. */
uint8_t read_response(void)
{
    uint8_t res = 0;
    uint8_t rx_byte = 0x00;
    uint8_t rx_size = 0x00;

	do
	{
		rx_size = UART_get_rx( &wifi_uart, &rx_byte, 1);
		while ( rx_size > 0 )
		{
			rx_buff[data_len] = rx_byte;
			if(rx_buff[data_len - 1] == 'O')
			{
				if(rx_buff[data_len] == 'K')
				{
				   res = 1;
				}
			}

			data_len++;
			rx_size = 0;
		}

		if(res == 1)
		{
			res = 0;
			UART_polled_tx_string(&g_uart,&rx_buff);
			break;
		}
	}while(data_len < sizeof(rx_buff));

	return res;
}

uint8_t response_success(void)
{
    uint8_t result;
    uint8_t *resp = rx_buff;

    while(!data_ready);

    resp += cmd_size;

    if((strstr(resp,"OK")) || (strstr(resp,"no change")) || (strstr(resp,"ready")))
    {
       result = 1;
    }
    else
    {
    	result = 0;
    }

    reset_buff();

    return result;
}

/*-----------------------------------------------------------------------------
 * main
 */
int main(int argc, char **argv)
{
    PLIC_init();

    UART_init(&wifi_uart,
              COREUARTAPB1_BASE_ADDR,
			  BAUD_VALUE_115200,
              (DATA_8_BITS | NO_PARITY));

    UART_init(&g_uart,
             COREUARTAPB0_BASE_ADDR,
			  BAUD_VALUE_115200,
             (DATA_8_BITS | NO_PARITY));

    UART_polled_tx_string(&g_uart, (const uint8_t *)g_hello_msg);

    GPIO_init(&g_gpio_out, COREGPIO_OUT_BASE_ADDR, GPIO_APB_8_BITS_BUS);
    GPIO_set_outputs(&g_gpio_out,0x02);
    delay_ms(6000);

    PLIC_SetPriority(External_29_IRQn, 1);
    PLIC_EnableIRQ(External_29_IRQn);

    UART_rxflush(&wifi_uart);
    HAL_enable_interrupts();

    /* Resetting module. */
    UART_rxflush(&wifi_uart);
    PLIC_EnableIRQ(External_29_IRQn);
	UART_Write_AT("AT+RST");
	while(!response_success());
	UART_polled_tx_string(&g_uart,"\r\n RESET SUCCESSFUL. \r\n ");
	delay_ms(1500);

    /* Set maximum value of RF TX Power. */
	UART_rxflush(&wifi_uart);
	PLIC_EnableIRQ(External_29_IRQn);
	UART_Write_AT("AT+RFPOWER=50");
	while(!response_success());
	UART_polled_tx_string(&g_uart,"\r\n RFPOWER command successful. \r\n ");
	delay_ms(1500);

	/* Change the working mode to 1. */
	UART_rxflush(&wifi_uart);
	PLIC_EnableIRQ(External_29_IRQn);
	UART_Write_AT("AT+CWMODE_CUR=1");
	while(!response_success());
	UART_polled_tx_string(&g_uart,"\r\n CWMODE_CUR command successful. \r\n ");
	delay_ms(1500);

	/* Setting connection mode. */
	UART_rxflush(&wifi_uart);
	PLIC_EnableIRQ(External_29_IRQn);
	UART_Write_AT("AT+CIPMUX=1");
	while(!response_success());
	UART_polled_tx_string(&g_uart,"\r\n CIPMUX command successful. \r\n\n ");
	delay_ms(1500);


    reset_buff();
    /* Connecting to AP. */
    cmd_size = sizeof("AT+CWJAP_CUR=");
    cmd_size += sizeof(SSID);
    //cmd_size += sizeof(password);
    cmd_size += 1;

    UART_polled_tx_string(&wifi_uart,"AT+CWJAP_CUR=");
    UART_send(&wifi_uart,&SQ,1);
    UART_polled_tx_string(&wifi_uart,SSID);
    UART_send(&wifi_uart,&SQ,1);
    UART_send(&wifi_uart,&SC,1);
    //UART_send(&wifi_uart,&SQ,1);
    //UART_polled_tx_string(&wifi_uart,password);
    //UART_send(&wifi_uart,&SQ,1);
    UART_send(&wifi_uart,&CR,1);
    UART_send(&wifi_uart,&LF,1);
    response  = read_response();
    if(!response)
    {
    	UART_polled_tx_string(&g_uart,"\r\n CWJAP_CUR command successful. \r\n\r\n");
    }
    else
    {
    	UART_polled_tx_string(&g_uart,"\r\n CWJAP_CUR command failed. \r\n\r\n");
    }
    delay_ms(1500);

    /* Check the assigned IP value. */
    reset_buff();
	UART_rxflush(&wifi_uart);
	//UART_Write_AT("AT+CIFSR");AT+CWJAP=?
	UART_Write_AT("AT+CWJAP_CUR?");
	response  = read_response();
	if(!response)
	{
		UART_polled_tx_string(&g_uart,"\r\n CWJAP_CUR command successful. \r\n\n\n");
	}
    else
    {
    	UART_polled_tx_string(&g_uart,"\r\n CWJAP_CUR command failed. \r\n\r\n");
    }
	delay_ms(1500);

    /* Check the assigned IP value. */
    reset_buff();
	UART_rxflush(&wifi_uart);
	UART_Write_AT("AT+CIFSR");
	response = read_response();
	if(!response)
	{
		UART_polled_tx_string(&g_uart,"\r\n CIFSR command successful. \r\n\n\n");
	}
    else
    {
    	UART_polled_tx_string(&g_uart,"\r\n CIFSR command fail. \r\n\r\n");
    }
	delay_ms(1500);

    reset_buff();
	UART_rxflush(&wifi_uart);
	response = read_response();



    while(1)
    {
    	;
    }

    return 0;
}
