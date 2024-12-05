/** @file isr.c
 * 
 * @brief Interrupt service routines.
 */

#include "isr.h"

#define THIS_FILE__ "isr.c"

volatile uint32_t ticks = 0;

void
SysTick_Handler(void)
{
    ticks++;
}

void
HardFault_Handler(void)
{
    //printf("Exception : Hardfault\n");
    ASSERT(0);
    while(1);
}


void
MemManage_Handler(void)
{
    //printf("Exception : MemManage\n");
    while(1);
}

void
BusFault_Handler(void)
{
    //printf("Exception : BusFault\n");
    while(1);
}

void
OTG_FS_IRQHandler(void)
{
    usb_irq_handler();
}

/*** end of file ***/