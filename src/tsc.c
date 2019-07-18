/*
 * tsc.c
 *
 *  Created on: Jun 26, 2019
 *      Author: jack
 */

/*	D E V I C E   I N C L U D E S   */
#include "stm32f091xc.h"
#include <stdint.h>

/*	F R E E R T O S   I N C L U D E S   */
//#include "FreeRTOS.h"
//#include "task.h"

/*	A P P L I C A T I O N   I N C L U D E S   */
#include "tsc.h"
#include "stm32f0xx_hal.h"
#include "pwm.h"
#include "gpio.h"
#include "tubes.h"

#define TSC_SAMPLING_CAP	3	// sampling capacitor G5IO1 - PB3
#define TSC_CHANNEL			4	// channel pin G5IO2 - PB4

/* Configure TCS */
void init_tsc(void) {
	/* configure capacitor pin */
//	GPIOB->MODER |= (GPIO_MODER_MODER3_1);		// alternate function mode
//	GPIOB->OTYPER |= GPIO_OTYPER_OT_3;			//
//
//	/* config TSC_G5_IO1 - PB3 to AF3 */
//	GPIOB->MODER |= (GPIOB->MODER & ~(GPIO_MODER_MODER3)) | GPIO_MODER_MODER3_1;
//	GPIOB->AFR[0] |=  (0x03 << GPIO_AFRL_AFRL3_Pos); // select AF3 on PB3
//
//	/* configure sensing pin */
//	GPIOB->MODER |=		(GPIO_MODER_MODER4_1);		// alternate funciton mode
//	GPIOB->OTYPER &= ~(GPIO_OTYPER_OT_4);			// configured as push-pull
//
//	/* config TSC_G5_IO2 - PB4 to AF3 */
//	GPIOB->MODER = (GPIOB->MODER & ~(GPIO_MODER_MODER4)) | GPIO_MODER_MODER4_1;
//	GPIOB->AFR[0] |=  (0x03 << GPIO_AFRL_AFRL4_Pos); // select AF4 on PB3
//
//	TSC->CR &= ~(TSC_CR_IODEF);
//
//	// discharge for ~1ms
//	int i;
//	for(i = 0; i < 10000; i++);
//
//	/* (1) Select fPGCLK = fHCLK/32,
//	Set pulse high = 2xtPGCLK,Master
//	Set pulse low = 2xtPGCLK
//	Set Max count value = 16383 pulses
//	Enable TSC */
//	/* (2) Disable hysteresis */
//	/* (3) Enable end of acquisition IT */
//	/* (4) Sampling enabled, G5IO1 */
//	/* (5) Channel enabled, G5IO2 */
//	/* (6) Enable group, G5 */
//	TSC->CR = TSC_CR_PGPSC_2 | TSC_CR_PGPSC_0 | TSC_CR_CTPL_0 | TSC_CR_CTPH_0
//			| TSC_CR_MCV_2 | TSC_CR_MCV_1 | TSC_CR_TSCE; /* (1) */
//	TSC->IOHCR &= (uint32_t)(~(TSC_IOHCR_G5_IO1 | TSC_IOHCR_G5_IO2)); /* (2) */
//	TSC->IER = TSC_IER_EOAIE; /* (3) */
//	TSC->IOSCR = TSC_IOSCR_G5_IO1; /* (4) */
//	TSC->IOCCR = TSC_IOCCR_G5_IO2; /* (5) */
//	TSC->IOGCSR |= TSC_IOGCSR_G5E; /* (5) */
//
//	TSC->CR |= TSC_CR_AM | TSC_CR_START;
//
//	NVIC_EnableIRQ(TSC_IRQn);
//	NVIC_SetPriority(TSC_IRQn, 0);

//	GPIOB->MODER  |= GPIO_MODER_MODER3_1 |GPIO_MODER_MODER4_1;										// Alt -mode
//	GPIOB->AFR[0]	|=(3<<GPIO_AFRL_AFRL3_Pos) |(3<<GPIO_AFRL_AFRL4_Pos);						// TSC
//
//	RCC->AHBENR |=RCC_AHBENR_TSCEN;
//	TSC->CR |= (0b10<<TSC_CR_CTPH_Pos) | (1<<TSC_CR_CTPL_Pos) | (0b11<<TSC_CR_PGPSC_Pos) | (0b110<<TSC_CR_MCV_Pos) | TSC_CR_TSCE;
//	TSC->IER |=TSC_IER_MCEIE | TSC_IER_EOAIE;
//	TSC->IOHCR &=(~TSC_IOHCR_G5_IO1) & (~TSC_IOHCR_G5_IO2);  //Hysteresis - off
//	TSC->IOSCR |=TSC_IOSCR_G5_IO1;															// Condensor
//	TSC->IOCCR |=TSC_IOSCR_G5_IO2; 															// Sensor
//	TSC->IOGCSR |=TSC_IOGCSR_G5E;

	TSC_HandleTypeDef htsc;
	htsc.Instance = TSC;
	htsc.Init.CTPulseHighLength = TSC_CTPH_2CYCLES;
	htsc.Init.CTPulseLowLength = TSC_CTPL_2CYCLES;
	htsc.Init.SpreadSpectrum = DISABLE;
	htsc.Init.SpreadSpectrumDeviation = 1;
	htsc.Init.SpreadSpectrumPrescaler = TSC_SS_PRESC_DIV1;
	htsc.Init.PulseGeneratorPrescaler = TSC_PG_PRESC_DIV4;
	htsc.Init.MaxCountValue = TSC_MCV_16383;
	htsc.Init.IODefaultMode = TSC_IODEF_OUT_PP_LOW;
	htsc.Init.SynchroPinPolarity = TSC_SYNC_POLARITY_FALLING;
	htsc.Init.AcquisitionMode = TSC_ACQ_MODE_NORMAL;
	htsc.Init.MaxCountInterrupt = DISABLE;
	htsc.Init.ChannelIOs = TSC_GROUP4_IO2|TSC_GROUP5_IO3;
	htsc.Init.ShieldIOs = 0;
	htsc.Init.SamplingIOs = TSC_GROUP4_IO1|TSC_GROUP5_IO4;

	HAL_TSC_Init(&htsc);

	NVIC_EnableIRQ (TSC_IRQn);
	NVIC_SetPriority(TSC_IRQn, 0);
}

void TSC_IRQHandler(void) {
	/* End of acquisition flag */
	if ((TSC->ISR & TSC_ISR_EOAF) == TSC_ISR_EOAF) {
		toggle_error_led();
		TSC->ICR = TSC_ICR_EOAIC; /* Clear flag */
		uint32_t acquisition_value = TSC->IOGXCR[5]; /* Get G5 counter value */
	}
}
