/*
 * low_power.c
 *
 *  Created on: Jul 25, 2019
 *      Author: jack
 */

#include "stm32f091xc.h"
#include <stdint.h>

/*	A P P L I C A T I O N   I N C L U D E S   */
//#include "low_power.h"
#include "main.h"
#include "usart.h"
#include "rtc.h"
#include "tubes.h"
#include "i2c.h"
#include "adc.h"
#include "gpio.h"

/*	F R E E R T O S   I N C L U D E S   */
#include "FreeRTOS.h"
#include "task.h"


/*	L O W - P O W E R   */
void configure_pwr_for_sleep(void) {
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	PWR->CSR |= PWR_CSR_EWUP2;		// enable WKUP2 on PC13
	PWR->CR |= PWR_CR_CWUF; 		// clear the WUF flag
	PWR->CR |= PWR_CR_PDDS;
}

void configure_rtc_for_sleep(void) {
	RCC->BDCR |=	( RCC_BDCR_LSEON		// use LSE oscillator for RTC
					| RCC_BDCR_LSEDRV_0	);	// not-bypass mode medium-high drive LSE
	RCC->BDCR &= ~RCC_BDCR_RTCSEL_LSI; 	// disable LSI use LSE instead

	rtc_disable_alarm();	// make sure that standby mode is not exited by RTC Alarm A
}

/* Configures peripherals for entry to the DeepSleep Low-Power Mode,
 * 	ends FreeRTOS scheduler, and then sets the SCB->SCR bit to enter
 * 	the DeepSleep Low-Power Mode */
void configure_for_deepsleep(void) {
	set_sleep_mode_hc_10(); 	// put HC-10 in sleep mode
	uart_disable_peripheral();
	i2c_disable_peripheral();
	disable_adc();
	efuse_disable();			// cut power to tubes, boost converter
	update_time(0, 0, 0);		/* do not display anything on tubes */
	configure_rtc_for_sleep();  // configure RTC for DeepSleep/Standby mode
	vTaskEndScheduler();
	configure_pwr_for_sleep();
}


void configure_for_stopmode(void) {
	set_sleep_mode_hc_10(); 	// put HC-10 in sleep mode
	uart_disable_peripheral();
	i2c_disable_peripheral();
	disable_adc();
	efuse_disable();			// cut power to tubes, boost converter
	update_time(0, 0, 0);		/* do not display anything on tubes */
	configure_rtc_for_sleep();  // configure RTC for DeepSleep/Standby mode
	vTaskEndScheduler();
	configure_pwr_for_sleep();

	// clear all EXTI Line Pending bits
	EXTI->PR |= 0xFFFFFFFF;

	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
	PWR->CR &= ~PWR_CR_PDDS;
	PWR->CR |= PWR_CR_LPDS;
}


/*	T I C K L E S S   I D L E   */
void vApplicationSleep( TickType_t xExpectedIdleTime ) {
	unsigned long ulLowPowerTimeBeforeSleep, ulLowPowerTimeAfterSleep;
	eSleepModeStatus eSleepStatus;
    ulLowPowerTimeBeforeSleep = ulGetExternalTime();

    /* Stop the timer that is generating the tick interrupt. */
    prvStopTickInterruptTimer();

    /* Enter a critical section that will not effect interrupts bringing the MCU
        out of sleep mode. */
    __disable_irq();

    /* Ensure it is still ok to enter the sleep mode. */
	eSleepStatus = eTaskConfirmSleepModeStatus();

	 if( eSleepStatus == eAbortSleep ) {
		/* A task has been moved out of the Blocked state since this macro was
		executed, or a context swith is being held pending.  Do not enter a
		sleep state.  Restart the tick and exit the critical section. */
		prvStartTickInterruptTimer();
		__enable_irq();
	}
	else {
		if( eSleepStatus == eNoTasksWaitingTimeout ) {
			/* It is not necessary to configure an interrupt to bring the
			microcontroller out of its low power state at a fixed time in the
			future. */
			prvSleep();
		}
		else {
			/* Configure an interrupt to bring the microcontroller out of its low
			power state at the time the kernel next needs to execute.  The
			interrupt must be generated from a source that remains operational
			when the microcontroller is in a low power state. */
			vSetWakeTimeInterrupt( xExpectedIdleTime );

			/* Enter the low power state. */
			prvSleep();

			/* Determine how long the microcontroller was actually in a low power
			state for, which will be less than xExpectedIdleTime if the
			microcontroller was brought out of low power mode by an interrupt
			other than that configured by the vSetWakeTimeInterrupt() call.
			Note that the scheduler is suspended before
			portSUPPRESS_TICKS_AND_SLEEP() is called, and resumed when
			portSUPPRESS_TICKS_AND_SLEEP() returns.  Therefore no other tasks will
			execute until this function completes. */
			ulLowPowerTimeAfterSleep = ulGetExternalTime();

			/* Correct the kernels tick count to account for the time the
			microcontroller spent in its low power state. */
			vTaskStepTick( ulLowPowerTimeAfterSleep - ulLowPowerTimeBeforeSleep );
		}

		/* Exit the critical section - it might be possible to do this immediately
		after the prvSleep() calls. */
		__enable_irq();

		/* Restart the timer that is generating the tick interrupt. */
		prvStartTickInterruptTimer();
	}
}

unsigned long ulGetExternalTime(void) {
	return TIM1->CNT; // TIM1->CNT is in ms
}

void vSetWakeTimeInterrupt(TickType_t xExpectedIdleTime) {
	/* config TIM1 registers */
	TIM1->PSC = 7999; 	// 8MHz / (7999+1) = 1kHz
	TIM1->ARR = xExpectedIdleTime;
	TIM1->EGR |= TIM_EGR_UG;	// Force update generation (UG = 1)
	TIM1->DIER |= TIM_DIER_UIE;	// enable Update Interrupt Flag
	TIM1->CNT = 0;
	TIM1->CR1 |= TIM_CR1_CEN;	// enable counter
}


// handle Tickless-Idle Wake-Up Interrupt
void TIM1_IRQHandler(void) {
	if(TIM1->SMCR & TIM_SR_UIF) {
	    TIM2->SR &= ~(TIM_SR_UIF);
	    TIM1->CR1 &= ~(TIM_CR1_CEN); // disable timer
	}
}

void prvSleep(void) {
	__WFI();	// sleep (stop the CPU clock) when the opportunity is given
}

// start systick
void prvStartTickInterruptTimer(void) {
	/* initialize SysTick timer to 10ms ticks */
	SysTick_Config(60000);
}

// stop SysTick
void prvStopTickInterruptTimer(void) {
	/* Disable SysTick IRQ and SysTick Timer */
	SysTick->CTRL  &= ~(SysTick_CTRL_TICKINT_Msk   |
	                   SysTick_CTRL_ENABLE_Msk);
}
