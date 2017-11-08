/*!
    \file  gd32f4xx_it.c
    \brief interrupt service routines
*/

/*
    Copyright (C) 2016 GigaDevice

    2016-10-19, V1.0.0, demo for GD32F4xx
*/

#include "gd32f4xx.h"
#include "gd32f4xx_it.h"
#include "main.h"

//#include "FreeRTOS.h"
//#include "task.h"
//#include "queue.h"
#include "lwip/sys.h"

#include "los_hw.h"

//extern xSemaphoreHandle g_rx_semaphore;
extern sys_sem_t g_rx_semaphore;

int tcp_data_in = 0;

/*!
    \brief      this function handles NMI exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void NMI_Handler(void)
{
}

/*!
    \brief      this function handles HardFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void HardFault_Handler(void)
{
    /* if Hard Fault exception occurs, go to infinite loop */
    while (1){
    }
}

/*!
    \brief      this function handles MemManage exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void MemManage_Handler(void)
{
    /* if Memory Manage exception occurs, go to infinite loop */
    while (1){
    }
}

/*!
    \brief      this function handles BusFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void BusFault_Handler(void)
{
    /* if Bus Fault exception occurs, go to infinite loop */
    while (1){
    }
}

/*!
    \brief      this function handles UsageFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void UsageFault_Handler(void)
{
    /* if Usage Fault exception occurs, go to infinite loop */
    while (1){
    }
}

/*!
    \brief      this function handles ethernet interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/
void ENET_IRQHandler(void)
{
    //portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
		int needschedule = 0;
    /* frame received */
    if(SET == enet_interrupt_flag_get(ENET_DMA_INT_FLAG_RS)){ 
        /* give the semaphore to wakeup LwIP task */
        //xSemaphoreGiveFromISR(g_rx_semaphore, &xHigherPriorityTaskWoken);
				sys_sem_signal(&g_rx_semaphore);
				needschedule = 1;
				//tcp_data_in = 1;
    }

    /* clear the enet DMA Rx interrupt pending bits */
    enet_interrupt_flag_clear(ENET_DMA_INT_FLAG_RS_CLR);
    enet_interrupt_flag_clear(ENET_DMA_INT_FLAG_NI_CLR);

    /* switch tasks if necessary */
		//if(pdFALSE != xHigherPriorityTaskWoken){
    if(needschedule)
		{
        //portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
				needschedule = 0;
				LOS_Schedule();
				
    }
}
