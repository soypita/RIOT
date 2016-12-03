/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     cpu_stm32l1
 * @{
 *
 * @file
 * @brief       Implementation of the kernel's lpm interface
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include "uart_stdio.h"

#include "arch/lpm_arch.h"

#include "stm32l1xx.h"
#include "xtimer.h"

#include "cpu.h"
#include "board.h"
#include "periph_conf.h"

#ifdef MODULE_AUTO_INIT
#include <auto_init.h>
#endif

#define CR_DS_MASK               ((uint32_t)0xFFFFFFFC)

/* Ultra Low Power mode definitions */
#define PWR_OFFSET               (PWR_BASE - PERIPH_BASE)
#define CR_OFFSET                (PWR_OFFSET + 0x00)
#define ULP_BitNumber           0x09
#define CR_ULP_BB               (PERIPH_BB_BASE + (CR_OFFSET * 32) + (ULP_BitNumber * 4))

void lpm_arch_init(void)
{
    /* Disable peripherals in Sleep mode */
	RCC->APB1LPENR &= ~(RCC_APB1LPENR_TIM2LPEN);
	RCC->APB1LPENR &= ~(RCC_APB1LPENR_TIM3LPEN);
	RCC->APB1LPENR &= ~(RCC_APB1LPENR_TIM4LPEN);
	RCC->APB1LPENR &= ~(RCC_APB1LPENR_TIM5LPEN);
	RCC->APB1LPENR &= ~(RCC_APB1LPENR_TIM6LPEN);
	RCC->APB1LPENR &= ~(RCC_APB1LPENR_TIM7LPEN);
	RCC->APB1LPENR &= ~(RCC_APB1LPENR_LCDLPEN);
	RCC->APB1LPENR &= ~(RCC_APB1LPENR_WWDGLPEN);
	RCC->APB1LPENR &= ~(RCC_APB1LPENR_SPI2LPEN);
	RCC->APB1LPENR &= ~(RCC_APB1LPENR_SPI3LPEN);
	RCC->APB1LPENR &= ~(RCC_APB1LPENR_USART2LPEN);
	RCC->APB1LPENR &= ~(RCC_APB1LPENR_USART3LPEN);
	RCC->APB1LPENR &= ~(RCC_APB1LPENR_UART4LPEN);
	RCC->APB1LPENR &= ~(RCC_APB1LPENR_UART5LPEN);
	RCC->APB1LPENR &= ~(RCC_APB1LPENR_I2C1LPEN);
	RCC->APB1LPENR &= ~(RCC_APB1LPENR_I2C2LPEN);
	RCC->APB1LPENR &= ~(RCC_APB1LPENR_USBLPEN);
	RCC->APB1LPENR &= ~(RCC_APB1LPENR_PWRLPEN);
	RCC->APB1LPENR &= ~(RCC_APB1LPENR_DACLPEN);
	RCC->APB1LPENR &= ~(RCC_APB1LPENR_COMPLPEN);

	RCC->APB2LPENR &= ~(RCC_APB2LPENR_SYSCFGLPEN);
	RCC->APB2LPENR &= ~(RCC_APB2LPENR_TIM9LPEN);
	RCC->APB2LPENR &= ~(RCC_APB2LPENR_TIM10LPEN);
	RCC->APB2LPENR &= ~(RCC_APB2LPENR_TIM11LPEN);
	RCC->APB2LPENR &= ~(RCC_APB2LPENR_ADC1LPEN);
	RCC->APB2LPENR &= ~(RCC_APB2LPENR_SDIOLPEN);
	RCC->APB2LPENR &= ~(RCC_APB2LPENR_SPI1LPEN);
	RCC->APB2LPENR &= ~(RCC_APB2LPENR_USART1LPEN);

	/* RCC->AHBLPENR &= ~(RCC_AHBLPENR_GPIOALPEN); */
	/* RCC->AHBLPENR &= ~(RCC_AHBLPENR_GPIOBLPEN); */
	RCC->AHBLPENR &= ~(RCC_AHBLPENR_GPIOCLPEN);
	RCC->AHBLPENR &= ~(RCC_AHBLPENR_GPIODLPEN);
	RCC->AHBLPENR &= ~(RCC_AHBLPENR_GPIOELPEN);
	RCC->AHBLPENR &= ~(RCC_AHBLPENR_GPIOHLPEN);
	RCC->AHBLPENR &= ~(RCC_AHBLPENR_GPIOFLPEN);
	RCC->AHBLPENR &= ~(RCC_AHBLPENR_GPIOGLPEN);
	RCC->AHBLPENR &= ~(RCC_AHBLPENR_CRCLPEN);
	RCC->AHBLPENR &= ~(RCC_AHBLPENR_FLITFLPEN);
	RCC->AHBLPENR &= ~(RCC_AHBLPENR_SRAMLPEN);
	RCC->AHBLPENR &= ~(RCC_AHBLPENR_DMA1LPEN);
	RCC->AHBLPENR &= ~(RCC_AHBLPENR_DMA2LPEN);
	RCC->AHBLPENR &= ~(RCC_AHBLPENR_AESLPEN);
	RCC->AHBLPENR &= ~(RCC_AHBLPENR_FSMCLPEN);
}

enum lpm_mode lpm_arch_set(enum lpm_mode target)
{
    switch (target) {
        case LPM_SLEEP:;                                            /* Sleep mode */
            PWR->CR = (PWR->CR & CR_DS_MASK) | PWR_CR_LPSDSR;
            SCB->SCR &= (uint32_t) ~((uint32_t)SCB_SCR_SLEEPDEEP);  /* Clear SLEEPDEEP bit of Cortex System Control Register */

            /* Request Wait For Interrupt */
            __disable_irq();
            //switch_to_msi_1mhz();

            asm ("DMB");
            __WFI();
            asm ("nop");

           // restore_clocks_hsi();
            __enable_irq();
            break;

        case LPM_POWERDOWN:         /* Low-power sleep mode */
            /* Regulator in LP mode */
            PWR->CR = (PWR->CR & CR_DS_MASK) | PWR_CR_LPSDSR;

            /* Enable Ultra Low Power mode */
            *(__IO uint32_t *) CR_ULP_BB = 1;

            /* Set SLEEPDEEP bit of Cortex System Control Register */
            SCB->SCR |= SCB_SCR_SLEEPDEEP;

            /* Wait in sleep mode until interrupt */
            __disable_irq();
			
            /* Switch to medium-speed clock */
			msi_clock_65khz();

            asm ("DMB");
            __WFI();
            asm ("nop");

            /* Clear SLEEPDEEP bit */
            SCB->SCR &= (uint32_t) ~((uint32_t)SCB_SCR_SLEEPDEEP);

            asm ("nop");
            asm ("nop");

            /* Switch back to full speed */
            cpu_init();

            __enable_irq();

            break;

        case LPM_OFF:               /* Standby mode */
			/* Clear Wakeup flag */	
			PWR->CR |= PWR_CR_CWUF;
		
            /* Select STANDBY mode */
            PWR->CR |= PWR_CR_PDDS;

            /* Set SLEEPDEEP bit of Cortex System Control Register */
            SCB->SCR |= SCB_SCR_SLEEPDEEP;

            /* Enable Ultra Low Power mode */
            *(__IO uint32_t *) CR_ULP_BB = 1;

            /* This option is used to ensure that store operations are completed */
            #if defined (__CC_ARM)
            __force_stores();
            #endif
            /* Request Wait For Interrupt */
            __WFI();
            break;

        /* do nothing here */
        case LPM_UNKNOWN:
        case LPM_ON:
        case LPM_IDLE:
        default:
            break;
    }

    return 0;
}

enum lpm_mode lpm_arch_get(void)
{
    /* TODO */
    return 0;
}

void lpm_arch_awake(void)
{
    /* Disable Ultra Low Power mode */
    *(__IO uint32_t *) CR_ULP_BB = 0;

    PWR->CR &= (uint32_t) ~((uint32_t)PWR_CR_LPRUN);
    PWR->CR &= (uint32_t) ~((uint32_t)PWR_CR_LPSDSR);
}

void lpm_arch_begin_awake(void)
{
    /* TODO */
}

void lpm_arch_end_awake(void)
{
    /* TODO */
}
