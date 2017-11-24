#include "los_sys.h"
#include "los_tick.h"
#include "los_task.ph"
#include "los_config.h"

#include "los_bsp_led.h"
#include "los_bsp_key.h"
#include "los_bsp_uart.h"

#include <string.h>

#include "gd32f4xx.h"
#include "netconf.h"
#include "main.h"
#include "tcpip.h"
#include "gd32f450i_eval.h"
#include "hello_gigadevice.h"
#include "tcp_client.h"
#include "udp_echo.h"

#include "lwip/tcp.h"
#include "lwip/opt.h"
#include "lwip/tcp.h"
#include "lwip/sys.h"
#include "lwip/api.h"
#include "lwip/sockets.h"

#define INIT_TASK_PRIO   ( tskIDLE_PRIORITY + 1 )
#define DHCP_TASK_PRIO   ( tskIDLE_PRIORITY + 4 )
#define LED_TASK_PRIO    ( tskIDLE_PRIORITY + 2 )

#define configMINIMAL_STACK_SIZE                      ( ( unsigned short ) 160 )
	

void led_task(void * pvParameters)
{  
    for( ;; )
    {
        /* toggle LED3 each 500ms */
        gd_eval_led_toggle(LED3);
		osDelay(500);
    }
}


#define MAX_RISCV_DATA_LEN 16
#define RISCV_DATA_REQUEST 0x10
static UINT32 g_riscv_test;
static char rwbuf[MAX_RISCV_DATA_LEN] = {0};
LITE_OS_SEC_TEXT VOID test_riscv_request(void)
{
    int ret = 0;
    ip_addr_t testip;
    ip_addr_t testmask;
    ip_addr_t testgw;
    int  runtime2 = 0;
    int sxx = -1;
    struct timeval val;
	struct sockaddr_in toAddr;
	struct sockaddr_in fromAddr;
    struct sockaddr_in localaddr;
    int n = 0;
    int i = 0;
//    char tx[2] = {0x31,0x32};
#if 0// display by max7219 led matrix
	LOS_TaskLock();
	LOS_MAX7219_Init();
	LOS_TaskUnlock();
#endif

    sxx = lwip_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	memset(&toAddr, 0, sizeof(toAddr));
	toAddr.sin_family =  AF_INET;

	memset(&localaddr, 0, sizeof(localaddr));
	localaddr.sin_family = AF_INET;
	localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localaddr.sin_port = htons(6001);

	lwip_bind(sxx, (struct sockaddr *)&localaddr, sizeof(localaddr));
    toAddr.sin_addr.s_addr = inet_addr("192.168.1.101");
	toAddr.sin_port = htons(5000);

    val.tv_sec = 1;
    val.tv_usec = 0;
    lwip_setsockopt(sxx, SOL_SOCKET, SO_RCVTIMEO, &val, sizeof(val));

    while(1)
    {
        rwbuf[0] = RISCV_DATA_REQUEST;
        n = lwip_sendto(sxx,rwbuf, 1, 0,(struct sockaddr *)&toAddr,sizeof(struct sockaddr_in));
        n = lwip_recv(sxx, rwbuf, MAX_RISCV_DATA_LEN, 0);
#if 0// display by max7219 led matrix
        for (i = 0; i < n; i++)
        {
            LOS_TaskLock();
            LOS_MAX7219_show(rwbuf[i]);
            LOS_TaskUnlock();
            LOS_TaskDelay(500);
        }
#endif
    }
}

void los_start_riscv_test(void)
{
	UINT32 uwRet;
	TSK_INIT_PARAM_S stTaskInitParam;

	(VOID)memset((void *)(&stTaskInitParam), 0, sizeof(TSK_INIT_PARAM_S));
	stTaskInitParam.pfnTaskEntry = (TSK_ENTRY_FUNC)test_riscv_request;
	stTaskInitParam.uwStackSize = LOSCFG_BASE_CORE_TSK_IDLE_STACK_SIZE;
	stTaskInitParam.pcName = "Riscvdemo";
	stTaskInitParam.usTaskPrio = 20;
	uwRet = LOS_TaskCreate(&g_riscv_test, &stTaskInitParam);

	if (uwRet != LOS_OK)
	{
		return ;
	}
	return ;
}


/*!
    \brief      init task
    \param[in]  pvParameters not used
    \param[out] none
    \retval     none
*/
void init_task(void * pvParameters)
{
	osThreadDef_t thread_def;
	
	LOS_TaskLock();
//    /* initilaize the LwIP stack */
    lwip_stack_init();

    /* initilaize the tcp server: telnet 8000 */
   // hello_gigadevice_init();
    /* initilaize the tcp client: echo 1026 */
    //tcp_client_init();
    /* initilaize the udp: echo 1025 */
    //udp_echo_init();

#ifdef USE_DHCP
    /* start DHCP client */
    //xTaskCreate(dhcp_task, "DHCP", configMINIMAL_STACK_SIZE * 2, NULL, DHCP_TASK_PRIO, NULL);
		thread_def.name = "DHCP";
		thread_def.stacksize = configMINIMAL_STACK_SIZE * 2;
		thread_def.tpriority = osPriorityNormal;
		thread_def.pthread = (os_pthread)dhcp_task;
		osThreadCreate(&thread_def, NULL);
#endif /* USE_DHCP */

    /* start toogle LED task every 250ms */
    //xTaskCreate(led_task, "LED", configMINIMAL_STACK_SIZE, NULL, LED_TASK_PRIO, NULL);
//		thread_def.name = "LED";
//		thread_def.stacksize = 3072/*configMINIMAL_STACK_SIZE * 2*/;
//		thread_def.tpriority = osPriorityLow;
//		thread_def.pthread = (os_pthread)led_task;
//		osThreadCreate(&thread_def, NULL);
		
		LOS_TaskUnlock();

		udp_echo_init();
		//los_start_riscv_test();
}

/*!
    \brief      led task
    \param[in]  pvParameters not used
    \param[out] none
    \retval     none
*/


/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(EVAL_COM1, (uint8_t) ch);
    while (RESET == usart_flag_get(EVAL_COM1, USART_FLAG_TBE));
    return ch;
}

extern void LOS_EvbSetup(void);

/*****************************************************************************
 Function    : main
 Description : Main function entry
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT
int main(void)
   {
    UINT32 uwRet; 
	osThreadDef_t thread_def;

	/*Init LiteOS kernel */
    uwRet = LOS_KernelInit();
    if (uwRet != LOS_OK) {
        return LOS_NOK;
    }
	/* Enable LiteOS system tick interrupt */
    LOS_EnableTick();

    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);

//	thread_def.name = "INIT";
//	thread_def.stacksize = 3072;
//	thread_def.tpriority = osPriorityLow;
//	thread_def.pthread = (os_pthread)init_task;
//	osThreadCreate(&thread_def, NULL);

//    LOS_TaskLock();//lock task schedule
    LOS_EvbSetup();
    /* initilaize the LwIP stack */
//	lwip_stack_init();
//
//    udp_echo_init();

//    LOS_TaskUnlock();

    /* Kernel start to run */
    LOS_Start();
    for (;;);
    /* Replace the dots (...) with your own code.  */
}
