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

#define INIT_TASK_PRIO   ( tskIDLE_PRIORITY + 1 )
#define DHCP_TASK_PRIO   ( tskIDLE_PRIORITY + 4 )
#define LED_TASK_PRIO    ( tskIDLE_PRIORITY + 2 )

#define configMINIMAL_STACK_SIZE                      ( ( unsigned short ) 160 )
	
extern int simpleclientstart(void);
extern int simpleserverstart(void);

void led_task(void * pvParameters)
{  
    for( ;; ){
        /* toggle LED3 each 250ms */
        gd_eval_led_toggle(LED3);
        //vTaskDelay(250);
				osDelay(500);
    }
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
	
    gd_eval_com_init(EVAL_COM1);
    gd_eval_led_init(LED3);
	
    LOS_TaskLock();//lock task schedule
    
    /* configure ethernet (GPIOs, clocks, MAC, DMA) */ 
    enet_system_setup();
	#if 1
    /* initilaize the LwIP stack */
    lwip_stack_init();
#endif
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
		thread_def.name = "LED";
		thread_def.stacksize = 3072/*configMINIMAL_STACK_SIZE * 2*/;
		thread_def.tpriority = osPriorityLow;
		thread_def.pthread = (os_pthread)led_task;
		//osThreadCreate(&thread_def, NULL);
		
		LOS_TaskUnlock();
		//simpleclientstart();
		simpleserverstart();
    //for( ;; ){
				//uwRet = LOS_TaskDelete(g_TestTaskID01);
        //vTaskDelete(NULL);
			//osDelay(500);
    //}
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

static UINT32 g_uwboadTaskID;
LITE_OS_SEC_TEXT VOID LOS_BoadExampleTskfunc(VOID)
{
    while (1)
    {
        LOS_EvbLedControl(LOS_LED2, LED_ON);
        LOS_EvbUartWriteStr("Board Test\n");
        LOS_TaskDelay(500);
        LOS_EvbLedControl(LOS_LED2, LED_OFF);
        LOS_TaskDelay(500);
    }
}
void LOS_BoadExampleEntry(void)
{
    UINT32 uwRet;
    TSK_INIT_PARAM_S stTaskInitParam;

    (VOID)memset((void *)(&stTaskInitParam), 0, sizeof(TSK_INIT_PARAM_S));
    stTaskInitParam.pfnTaskEntry = (TSK_ENTRY_FUNC)LOS_BoadExampleTskfunc;
    stTaskInitParam.uwStackSize = LOSCFG_BASE_CORE_TSK_IDLE_STACK_SIZE;
    stTaskInitParam.pcName = "BoardDemo";
    stTaskInitParam.usTaskPrio = 10;
    uwRet = LOS_TaskCreate(&g_uwboadTaskID, &stTaskInitParam);

    if (uwRet != LOS_OK)
    {
        return ;
    }
    return ;
}

//extern void LOS_Demo_Entry(void);

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

    /*
		add you hardware init code here
		for example flash, i2c , system clock ....
    */
	//HAL_init();....
	
	/*Init LiteOS kernel */
    uwRet = LOS_KernelInit();
    if (uwRet != LOS_OK) {
        return LOS_NOK;
    }
	/* Enable LiteOS system tick interrupt */
    LOS_EnableTick();

    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);

    /* init task */
    //xTaskCreate(init_task, "INIT", configMINIMAL_STACK_SIZE * 2, NULL, INIT_TASK_PRIO, NULL);
		thread_def.name = "INIT";
		thread_def.stacksize = 3072;
		thread_def.tpriority = osPriorityLow;
		thread_def.pthread = (os_pthread)init_task;
		osThreadCreate(&thread_def, NULL);
    /* start scheduler */
    //vTaskStartScheduler();

		//LOS_Demo_Entry();
		
    /* 
        Notice: add your code here
        here you can create task for your function 
        do some hw init that need after systemtick init
    */
    //LOS_EvbSetup();
    //LOS_BoadExampleEntry();
		
    /* Kernel start to run */
    LOS_Start();
    for (;;);
    /* Replace the dots (...) with your own code.  */
}
