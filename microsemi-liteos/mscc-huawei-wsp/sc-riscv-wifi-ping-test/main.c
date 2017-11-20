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

const uint8_t pheadbuffer[]="Welcome to Microsemi*/*\r\n\r\n";

const char SSID[] = "Connectify-me";
const char password[] = "fewepom4";

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

uint8_t updbuf[16] = {0};

const char * g_hello_msg =
"\r\n PolarFire WiFi3 Click Ping Example Project. \n\r";

const uint8_t opt_msg[] =
"\r\n\r\n\
******************************************************************************\r\n\
************* PolarFire WiFi3 Click UDP Example Project **************\r\n\
******************************************************************************\r\n\
  Press '1' to send data.\r\n";

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

		if((res == 1) && (data_len > cmd_size))
		{
			res = 0;
			UART_polled_tx_string(&g_uart,&rx_buff);
			break;
		}
	}while(data_len < sizeof(rx_buff));

	return res;
}


void UDPsend(uint8_t* buffer)
{
    uint8_t res = 0;
    uint8_t i = 0;
    uint8_t rx_byte = 0x00;
    uint8_t rx_size = 0x00;
    uint8_t data_send = 0;

	UART_rxflush(&wifi_uart);
	UART_polled_tx_string(&wifi_uart,"AT+CIPSEND=20");
    UART_send(&wifi_uart,&CR,1);
    UART_send(&wifi_uart,&LF,1);
    do
	{
        rx_size = UART_get_rx( &wifi_uart, &rx_byte, 1 );
		while ( rx_size > 0 )
		{
			rx_buff[data_len] = rx_byte;
			if(rx_buff[data_len] == '>')
			{
				while(data_send != 1)
				{
					UART_send(&wifi_uart,&buffer[i],1);
					i++;
					if(i == 20)
						data_send = 1;
				}

			}

		    if((rx_buff[data_len - 1] == 'O') && (rx_buff[data_len] == 'K') && (data_send == 1))
		    {
		    	res = 1;
		    }

		 	data_len++;
		 	rx_size = 0;
	   }

	   if(res == 1)
	   {
	       res = 0;
	       data_len = 0;
		   break;
	   }

    } while(data_len < sizeof(rx_buff));
}


/*-----------------------------------------------------------------------------
 * main
 */

int main(int argc, char **argv)
{
	int i = 0;
    uint8_t rx_byte = 0x00;
    size_t rx_size = 0x00;
    uint8_t res = 0;
    uint8_t data_send = 0;
    uint8_t rcv_data[40] = {0};
    uint8_t rcv_len = 0;

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

    /* Resetting module. */
    UART_rxflush(&wifi_uart);
    reset_buff();
	UART_Write_AT("AT+RST");
	response  = read_response();
    if(!response)
    {
    	UART_polled_tx_string(&g_uart,"\r\n RESET command successful. \r\n ");
    }
    else
    {
    	UART_polled_tx_string(&g_uart,"\r\n RESET command failed. \r\n ");
    }

	delay_ms(1500);

    /* Set maximum value of RF TX Power. */
	UART_rxflush(&wifi_uart);
	reset_buff();
	UART_Write_AT("AT+RFPOWER=50");
	response  = read_response();
    if(!response)
    {
    	UART_polled_tx_string(&g_uart,"\r\n RFPOWER command successful. \r\n ");
    }
    else
    {
    	UART_polled_tx_string(&g_uart,"\r\n RFPOWER command failed. \r\n ");
    }
	delay_ms(1500);

	/* Change the working mode to 1. */
	UART_rxflush(&wifi_uart);
	reset_buff();
	UART_Write_AT("AT+CWMODE_CUR=1");
	response  = read_response();
    if(!response)
    {
    	UART_polled_tx_string(&g_uart,"\r\n CWMODE_CUR command successful. \r\n ");
    }
    else
    {
    	UART_polled_tx_string(&g_uart,"\r\n CWMODE_CUR command failed. \r\n ");
    }
	delay_ms(1500);

	/* Setting connection mode. */
	reset_buff();
	UART_rxflush(&wifi_uart);
	UART_Write_AT("AT+CIPMUX=1");
	response  = read_response();
    if(!response)
    {
    	UART_polled_tx_string(&g_uart,"\r\n CIPMUX command successful. \r\n ");
    }
    else
    {
    	UART_polled_tx_string(&g_uart,"\r\n CIPMUX command  FAILED. \r\n ");
    }
	delay_ms(1500);


    reset_buff();
    UART_rxflush(&wifi_uart);
    /* Connecting to AP. */
    cmd_size = sizeof("AT+CWJAP_CUR=");
    cmd_size += sizeof(SSID);
    cmd_size += sizeof(password);
    cmd_size += 2;

    UART_polled_tx_string(&wifi_uart,"AT+CWJAP_CUR=");
    UART_send(&wifi_uart,&SQ,1);
    UART_polled_tx_string(&wifi_uart,SSID);
    UART_send(&wifi_uart,&SQ,1);
    UART_send(&wifi_uart,&SC,1);
    UART_send(&wifi_uart,&SQ,1);
    UART_polled_tx_string(&wifi_uart,password);
    UART_send(&wifi_uart,&SQ,1);
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
	UART_Write_AT("AT+CWJAP_CUR?");
	response  = read_response();
	if(!response)
	{
		UART_polled_tx_string(&g_uart,"\r\n CWJAP_CUR? command successful. \r\n\n\n");
	}
    else
    {
    	UART_polled_tx_string(&g_uart,"\r\n CWJAP_CUR? command failed. \r\n\r\n");
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

	//set single connection model
    reset_buff();
	UART_rxflush(&wifi_uart);
	UART_Write_AT("AT+CIPMUX=0");
	response = read_response();
	if(!response)
	{
		UART_polled_tx_string(&g_uart,"\r\n AT+CIPMUX ok. \r\n\n\n");
	}
    else
    {
    	UART_polled_tx_string(&g_uart,"\r\n AT+CIPMUX fail. \r\n\r\n");
    }
	delay_ms(500);

	//create a udp client
    reset_buff();
	UART_rxflush(&wifi_uart);
	UART_Write_AT("AT+CIPSTART=\"UDP\",\"192.168.137.1\",61773,6000,0");
	response = read_response();
	if(!response)
	{
		UART_polled_tx_string(&g_uart,"\r\n create udp client ok. \r\n\n\n");
	}
    else
    {
    	UART_polled_tx_string(&g_uart,"\r\n create udp client fail. \r\n\r\n");
    }
	delay_ms(500);

	/* Get inputs from user to determine which operation to perform */
	UART_polled_tx_string(&g_uart,opt_msg);
    for(;;)
    {
        /* Start command line interface if any key is pressed. */
        rx_size = UART_get_rx(&g_uart, rx_buff, sizeof(rx_buff));
        if(rx_size > 0)
        {
            switch(rx_buff[0])
            {
                case '1':
                    /* Send Data */
                	UDPsend("Microsemi Hyderabad");
                	UART_polled_tx_string(&g_uart,"Microsemi Hyderabad");
                	UART_polled_tx_string(&g_uart,opt_msg);
                break;

                default:
                	break;
            }
        }
    }
    return 0;
}
