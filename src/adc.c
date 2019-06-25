/*
 * adc.c
 *
 *  Created on: Jun 18, 2019
 *      Author: jack
 */

/*	D E V I C E   I N C L U D E S   */
#include "stm32f091xc.h"
#include <stdint.h>

/*	F R E E R T O S   I N C L U D E S   */
#include "FreeRTOS.h"
#include "task.h"

/*	A P P L I C A T I O N   I N C L U D E S   */
#include "adc.h"

void init_adc(void) {
	/* configure GPIO pin - PC3 AF0 */
	/* select alternate function mode */
	GPIOC->MODER = (GPIOC->MODER & ~(GPIO_MODER_MODER3)) | GPIO_MODER_MODER3_1;

	// select AF0 on PC3
	GPIOC->AFR[0] |=  (0x00 << GPIO_AFRL_AFRL3_Pos); // AFRL (Ports 0-7)

	calibrate_adc();

	/* set data resolution to 12-bit */
	while ((ADC1->CR & ADC_CR_ADEN) != 0);	// wait to write to data resolution
	ADC1->CFGR1 &= ~(ADC_CFGR1_RES);		// set data resolution to 12 bits

	/* set ADC clock mode to ADCCLK */
	ADC1->CFGR2 &= ~(ADC_CFGR2_CKMODE);

	/* enable interrupts */
	ADC1->IER |=	( ADC_IER_AWD1IE		// analog watchdog
					| ADC_IER_EOCIE			// end of conversion
			);

	enable_adc();

	// turn analog watchdog on for all channels
	// set_analog_watchdog_adc(0xFF);
}

void calibrate_adc(void) {
	/* from A.7.1 in Reference Manual */
	/* (1) Ensure that ADEN = 0 */
	/* (2) Clear ADEN by setting ADDIS*/
	/* (3) Clear DMAEN */
	/* (4) Launch the calibration by setting ADCAL */
	/* (5) Wait until ADCAL=0 */
	if ((ADC1->CR & ADC_CR_ADEN) != 0) { /* (1) */
		ADC1->CR |= ADC_CR_ADDIS; /* (2) */
	}
	while ((ADC1->CR & ADC_CR_ADEN) != 0) {
	/* For robust implementation, add here time-out management */
	}
		ADC1->CFGR1 &= ~ADC_CFGR1_DMAEN; /* (3) */
		ADC1->CR |= ADC_CR_ADCAL; /* (4) */
		while ((ADC1->CR & ADC_CR_ADCAL) != 0) { /* (5) */
		/* For robust implementation, add here time-out management */
		}
}

void enable_adc(void) {
	/* from A.7.2 in Reference Manual */
	/* (1) Ensure that ADRDY = 0 */
	/* (2) Clear ADRDY */
	/* (3) Enable the ADC */
	/* (4) Wait until ADC ready */
	if ((ADC1->ISR & ADC_ISR_ADRDY) != 0) { /* (1) */
		ADC1->ISR |= ADC_ISR_ADRDY; /* (2) */
	}
	ADC1->CR |= ADC_CR_ADEN; /* (3) */
	while ((ADC1->ISR & ADC_ISR_ADRDY) == 0) { /* (4) */
	/* For robust implementation, add here time-out management */
	}
}

void disable_adc(void) {
	/* from A.7.3 in Reference Manual */
	/* (1) Stop any ongoing conversion */
	/* (2) Wait until ADSTP is reset by hardware i.e. conversion is stopped */
	/* (3) Disable the ADC */
	/* (4) Wait until the ADC is fully disabled */
	ADC1->CR |= ADC_CR_ADSTP; /* (1) */
	while ((ADC1->CR & ADC_CR_ADSTP) != 0) { /* (2) */
		/* For robust implementation, add here time-out management */
	}
	ADC1->CR |= ADC_CR_ADDIS; /* (3) */
	while ((ADC1->CR & ADC_CR_ADEN) != 0) { /* (4) */
		/* For robust implementation, add here time-out management */
	}
}

uint32_t sample_adc(void) {
	ADC1->CFGR1 &= ~(ADC_CFGR1_CONT);			// set to single conversion mode
	while(!(ADC1->ISR & ADC_ISR_ADRDY));		// wait for ADC ready flag
	ADC1->CR |= ADC_CR_ADSTART;					// start conversion
	while((ADC1->ISR & ADC_ISR_EOC) == 0);			// wait for EOC (End of Conversion) flag
	return read_adc();
}

/* enables the VBAT channel
 * 	VBAT = 2 * conversion_value
 */
void enable_vbat_adc(void) {
	while(ADC1->CR & ADC_CR_ADSTART);		// cannot write while ADSTART = 1 (conversion ongoing)
	ADC1_COMMON->CCR |= ADC_CCR_VBATEN;		// enable VBAT channel
}

/* disables the VBAT channel */
void disable_vbat_adc(void) {
	while(ADC1->CR & ADC_CR_ADSTART);		// cannot write while ADSTART = 1 (conversion ongoing)
	ADC1_COMMON->CCR &= ~ADC_CCR_VBATEN;	// disable VBAT channel
}

/* will take a sample of the VBAT channel */
uint16_t read_vbat_adc(void) {
	// save the old config to write back later
	uint32_t original_config = (ADC1->CFGR1 & (ADC_CFGR1_EXTEN | ADC_CFGR1_CONT));

	// switch to single conversion mode
	ADC1->CFGR1 &= ~(ADC_CFGR1_EXTEN | ADC_CFGR1_CONT);
	select_adc_channel(18);				// ADC input channel 18
	ADC1->CR |= ADC_CR_ADSTART;			// trigger conversion
	uint16_t r = read_adc();			// save the VBAT voltage sample
	ADC1->CFGR1 |= original_config;		// rewrite the original config
	return r * 2;						// double since VBAT reading is /2
}

/* enables the internal Voltage Reference */
void enable_vrefint_adc(void) {
	while(ADC1->CR & ADC_CR_ADSTART);		// cannot write while ADSTART = 1 (conversion ongoing)
	ADC1_COMMON->CCR |= ADC_CCR_VREFEN;		// enable VBAT channel
}

/* disables the internal Voltage Reference */
void disable_vrefint_adc(void) {
	while(ADC1->CR & ADC_CR_ADSTART);		// cannot write while ADSTART = 1 (conversion ongoing)
	ADC1_COMMON->CCR &= ~ADC_CCR_VREFEN;	// disable VBAT channel
}

void set_analog_watchdog_adc(uint8_t channel) {
	/* (1) Select the continuous mode and configure */
	if(channel == 0xFF) { 	// enable analog watchdog on all channels
		ADC1->CFGR1 &= ~ADC_CFGR1_AWDSGL;
	}
	else {
		/* the Analog watchdog to monitor only @param:channel */
		ADC1->CFGR1 |= 	( ADC_CFGR1_CONT | (channel << 26) | ADC_CFGR1_AWDEN | ADC_CFGR1_AWDSGL);
	}

	/* (2) Define analog watchdog range : 16b-MSW is the high limit
	and 16b-LSW is the low limit */
	// TODO: work on finding the average untouched values
	ADC1->TR = (0xFF00 << 16) + 0x00FF; /* (2)*/

	/* (3) Enable interrupt on Analog Watchdog */
	ADC1->IER = ADC_IER_AWDIE; /* (3) */
}

/*
 * Input channel specified is selected for conversion
 *  @param uint8_t channel: 0-18
 */
void select_adc_channel(uint8_t channel) {
	while(ADC1->CR & ADC_CR_ADSTART);	// cannot write while ADSTART = 1 (conversion ongoing)
	ADC1->CHSELR = (0x1U << channel);	// select new input channel for conversion
}

/*
 * inline function that returns the 16 bit value currently in the ADC Data Register (ADC_DR)
 */
uint32_t inline read_adc(void) {
	return (ADC1->DR & ADC_DR_DATA);
}
