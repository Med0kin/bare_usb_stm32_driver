/** @file clock.c
 * 
 * @brief Clock configuration.
 */

#include "clock.h"

#define THIS_FILE__ "clock.c"

void
clock_init(void)
{

    // Change flash latency to 2 wait states
    // and wait for it to be set
    //
    FLASH->ACR |= FLASH_ACR_LATENCY_2WS;
    while(!(FLASH->ACR & FLASH_ACR_LATENCY_2WS));

    // Set regulator voltage scaling to max freq before enabling PLL
    //
    PWR->CR |= PWR_CR_VOS;

    // Enable HSE and wait for it to be ready
    //
    RCC->CR |= RCC_CR_HSEON;
    while(!(RCC->CR & RCC_CR_HSERDY));

    // Zero out PLLCFGR register, set values and select HSE as PLL source
    // /M = 25, *N = 144, /P = 2, /Q = 3
    // Keeping PLLN at 0b00 as it indicates /P = 2
    //
    RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLM |
                      RCC_PLLCFGR_PLLN |
                      RCC_PLLCFGR_PLLP |
                      RCC_PLLCFGR_PLLQ);
    RCC->PLLCFGR |= ((25  << RCC_PLLCFGR_PLLM_Pos) |
                     (144 << RCC_PLLCFGR_PLLN_Pos) |
                     (3   << RCC_PLLCFGR_PLLQ_Pos));
    RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSE;

    // Set HSE as PLL source
    //
    RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSE;

    // Enable PLL and wait for it to be ready
    //
    RCC->CR |= RCC_CR_PLLON;
    while(!(RCC->CR & RCC_CR_PLLRDY));

    // Set prescalers
    // AHB = 1, APB1 = 2, APB2 = 1
    //
    RCC->CFGR &= ~(RCC_CFGR_HPRE |
                   RCC_CFGR_PPRE1 |
                   RCC_CFGR_PPRE2);
    RCC->CFGR |= (RCC_CFGR_HPRE_DIV1 |
                  RCC_CFGR_PPRE1_DIV2 |
                  RCC_CFGR_PPRE2_DIV1);    

    // Set PLL as system clock source and wait for it to be set
    //
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while((!(RCC->CFGR & RCC_CFGR_SWS_PLL)));

    // Enable Clock Security System
    //
    RCC->CR |= RCC_CR_CSSON;

    SysTick_Config(72000);
}


/*** end of file ***/