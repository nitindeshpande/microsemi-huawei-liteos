/*!
    \file  netconf.c
    \brief network connection configuration 
*/

/*
    Copyright (C) 2016 GigaDevice

    2016-10-19, V1.0.0, demo for GD32F4xx
*/

#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/dhcp.h"
#include "ethernetif.h"
#include "main.h"
#include "netconf.h"
#include "tcpip.h"
#include <stdio.h>

//change from Keil
#include "cmsis_os.h"

#define MAX_DHCP_TRIES       4

typedef enum 
{ 
    DHCP_START = 0,
    DHCP_WAIT_ADDRESS,
    DHCP_ADDRESS_ASSIGNED,
    DHCP_TIMEOUT
}dhcp_state_enum;

#ifdef USE_DHCP
dhcp_state_enum dhcp_state = DHCP_START;
#endif /* USE_DHCP */

struct netif g_mynetif;

/*!
    \brief      initializes the LwIP stack
    \param[in]  none
    \param[out] none
    \retval     none
*/
void lwip_stack_init(void)
{
    struct ip_addr ipaddr;
    struct ip_addr netmask;
    struct ip_addr gw;

    /* create tcp_ip stack thread */
    tcpip_init( NULL, NULL );

    /* IP address setting */
#ifdef USE_DHCP
    ipaddr.addr = 0;
    netmask.addr = 0;
    gw.addr = 0;
#else
    IP4_ADDR(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
    IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1 , NETMASK_ADDR2, NETMASK_ADDR3);
    IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
  
#endif /* USE_DHCP */

    netif_add(&g_mynetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);

    /* registers the default network interface */
    netif_set_default(&g_mynetif);

    /* when the netif is fully configured this function must be called */
    netif_set_up(&g_mynetif);
}

#ifdef USE_DHCP
/*!
    \brief      dhcp_task
    \param[in]  none
    \param[out] none
    \retval     none
*/
void dhcp_task(void * pvParameters)
{
    struct ip_addr ipaddr;
    struct ip_addr netmask;
    struct ip_addr gw;
    uint32_t ip_address = 0;
  
    for(;;){
        switch(dhcp_state){
        case DHCP_START:
            dhcp_start(&g_mynetif);
            /* IP address should be set to 0 every time we want to assign a new DHCP address*/
            ip_address = 0;
            dhcp_state = DHCP_WAIT_ADDRESS;
            break;

        case DHCP_WAIT_ADDRESS:    
            /* read the new IP address */
            ip_address = g_mynetif.ip_addr.addr;

            if(0 != ip_address){ 
                dhcp_state = DHCP_ADDRESS_ASSIGNED;
                /* stop DHCP */
                dhcp_stop(&g_mynetif);
                //printf("\r\nDHCP -- eval board ip address: %d.%d.%d.%d \r\n", ip4_addr1_16(&ip_address), \
                //        ip4_addr2_16(&ip_address), ip4_addr3_16(&ip_address), ip4_addr4_16(&ip_address));
            }else{
                /* DHCP timeout */
                if(g_mynetif.dhcp->tries > MAX_DHCP_TRIES){
                    dhcp_state = DHCP_TIMEOUT;
                    /* stop DHCP */
                    dhcp_stop(&g_mynetif);

                    /* static address used */
                    IP4_ADDR(&ipaddr, IP_ADDR0 ,IP_ADDR1 , IP_ADDR2 , IP_ADDR3 );
                    IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
                    IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
                    netif_set_addr(&g_mynetif, &ipaddr , &netmask, &gw);
                }
            }
            break;

        default: 
            break;
        }
    	//change from Keil
        /* wait 250 ms */
        //vTaskDelay(250);
		osDelay(500);
    }   
}
#endif /* USE_DHCP */
