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
#include "FreeRTOS.h"
#include "task.h"

/*	A P P L I C A T I O N   I N C L U D E S   */
#include "pwm.h"
#include "gpio.h"
#include "tubes.h"

void init_tsc(void) {
	TSC->CR = TSC_CR_PGPSC_2 | TSC_CR_PGPSC_0 | TSC_CR_CTPH_0 | TSC_CR_CTPL_0
	| TSC_CR_MCV_2 | TSC_CR_MCV_1 | TSC_CR_TSCE; /* (1) */
	TSC->IOHCR &= (uint32_t)(~(TSC_IOHCR_G2_IO4 | TSC_IOHCR_G2_IO3)); /* (2) */
	TSC->IER = TSC_IER_EOAIE; /* (3) */
	TSC->IOSCR = TSC_IOSCR_G2_IO4; /* (4) */
	TSC->IOCCR = TSC_IOCCR_G2_IO3; /* (5) */
	TSC->IOGCSR |= TSC_IOGCSR_G2E; /* (5) */

	NVIC_EnableIRQ(TSC_IRQn);
	NVIC_SetPriority(TSC_IRQn, 0);
}

void TSC_IRQHandler(void) {
	/* End of acquisition flag */
	if ((TSC->ISR & TSC_ISR_EOAF) == TSC_ISR_EOAF) {
		TSC->ICR = TSC_ICR_EOAIC; /* Clear flag */
		uint32_t AcquisitionValue = TSC->IOGXCR[1]; /* Get G2 counter value */
	}
}
