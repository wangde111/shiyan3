/**
  ******************************************************************************
  * @file    stm32f1xx_it.c
  * @brief   中断服务函数
  ******************************************************************************
  */

#include "main.h"
#include "stm32f1xx_it.h"

/* Cortex-M3 内核异常 --------------------------------------------------------*/

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
    while (1)
    {
    }
}

void MemManage_Handler(void)
{
    while (1)
    {
    }
}

void BusFault_Handler(void)
{
    while (1)
    {
    }
}

void UsageFault_Handler(void)
{
    while (1)
    {
    }
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

/**
  * @brief  SysTick 中断, 每 1 ms 一次, 为 HAL_Delay()/HAL_GetTick() 提供时基
  */
void SysTick_Handler(void)
{
    HAL_IncTick();
}
