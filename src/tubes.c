/*
 * tubes.c
 *
 *  Created on: Jun 14, 2019
 *      Author: jack
 */

/*	D E V I C E   I N C L U D E S   */
#include "stm32f091xc.h"
#include <stdint.h>

/*	F R E E R T O S   I N C L U D E S   */
#include "FreeRTOS.h"
#include "task.h"

/*	A P P L I C A T I O N   I N C L U D E S   */
#include "gpio.h"
#include "vfd_typedefs.h"
#include "tubes.h"

/*	G L O B A L   V A R I A B L E S   */
extern System_State_E system_state;
extern uint8_t hour;			/* 0-23 */
extern uint8_t minutes;			/* 0-59 */
extern uint8_t seconds;			/* 0-59 */
extern int8_t temperature;		/* -128 - 127 */

/*	D E F I N E S   */
//#define	TUBE_TESTING		// only outputs 0xAA on tubes
#define DISPLAY_BINARY	1		// display BCD values instead of VFD-Tube mapped
//#define DEMO


/* Binary values that will display the corresponding
 * numbers on the seven segment displays after being
 * loaded to the shift register  */
#define 	ZERO        0b00111111; // 0x3F
#define 	ONE         0b00110000; // 0x30
#define 	TWO         0b01011011; // 0x5B
#define 	THREE       0b01001111; // 0x4F
#define 	FOUR        0b01100110; // 0x66
#define 	FIVE        0b01101101; // 0x6D
#define 	SIX         0b01111101; // 0x7D
#define 	SEVEN       0b00000111; // 0x07
#define 	EIGHT       0b01111111; // 0x7F
#define 	NINE        0b01101111; // 0x6F
#define 	LETTER_F    0b01101011; // 0x##
#define 	DEGREES     0b01101011; // 0x6B

#define ALL_TUBES	7

/* pinout */
//#define NOE			4	// PWM on Pin PA4
//#define NSRCLR		11	// PA11
//#define SRCLK		9	// PC9
//#define RCLK		8	// PC8

#ifdef DEMO
#define NOE			4	// PWM on Pin PA4
#define NSRCLR		11	// PA11
#define SRCLK		9	// PC9
#define RCLK		8	// PC8

#define SERIAL1		7	// PC7
#define SERIAL2		6	// PC6
#define SERIAL3		15	// PB15
#define SERIAL4		14	// PB14
#define SERIAL5		13	// PB13
#define SERIAL6		12	// PB12
#else
#define NOE			4	// PWM on Pin PA4
#define NSRCLR		7	// PA7 - Latch
#define SRCLK		6	// PA6
#define RCLK		4	// PC4

#define SERIAL1		3	// PA3
#define SERIAL2		5	// PA5
#define SERIAL3		5	// PC5
#define SERIAL4		2	// PB2
#define SERIAL5		10	// PB10
#define SERIAL6		11	// PB11
#endif

/* Configures the shift registers to be used */
void configure_shift_pins() {
	/* AHB Peripheral Clock Enable Registers */
	RCC->AHBENR |=	( RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN);

#ifdef DEMO
	/* set to LOW */
	GPIOA->ODR &= ~(GPIO_ODR_8 | GPIO_ODR_11);
	GPIOB->ODR &= ~(GPIO_ODR_12 | GPIO_ODR_13 | GPIO_ODR_14 | GPIO_ODR_15);
	GPIOC->ODR &= ~(GPIO_ODR_6 | GPIO_ODR_7 | GPIO_ODR_8 | GPIO_ODR_9);

	/* set to output */
	GPIOA->MODER |= (GPIO_MODER_MODER8_0 | GPIO_MODER_MODER11_0);
	GPIOB->MODER |= (GPIO_MODER_MODER12_0 | GPIO_MODER_MODER13_0 | GPIO_MODER_MODER14_0 | GPIO_MODER_MODER15_0);
	GPIOC->MODER |= (GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0 | GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0);

	/* set to push-pull */
	GPIOA->OTYPER &= ~(GPIO_OTYPER_OT_8 | GPIO_OTYPER_OT_11);
	GPIOB->OTYPER &= ~(GPIO_OTYPER_OT_12 | GPIO_OTYPER_OT_13 | GPIO_OTYPER_OT_14 | GPIO_OTYPER_OT_15);
	GPIOC->OTYPER &= ~(GPIO_OTYPER_OT_6 | GPIO_OTYPER_OT_7 | GPIO_OTYPER_OT_8 | GPIO_OTYPER_OT_9);

	/* set to mid-speed */
	GPIOA->OSPEEDR |= (GPIO_OSPEEDR_OSPEEDR8_0 | GPIO_OSPEEDR_OSPEEDR11_0);
	GPIOB->OSPEEDR |= (GPIO_OSPEEDR_OSPEEDR12_0 | GPIO_OSPEEDR_OSPEEDR13_0
					 | GPIO_OSPEEDR_OSPEEDR14_0 | GPIO_OSPEEDR_OSPEEDR15_0);
	GPIOC->OSPEEDR |= (GPIO_OSPEEDR_OSPEEDR6_0 | GPIO_OSPEEDR_OSPEEDR7_0
					 | GPIO_OSPEEDR_OSPEEDR8_0 | GPIO_OSPEEDR_OSPEEDR9_0);

#else
	/* set to LOW */
	GPIOA->ODR &= ~(GPIO_ODR_3 | GPIO_ODR_4 | GPIO_ODR_5 | GPIO_ODR_6 | GPIO_ODR_7);
	GPIOB->ODR &= ~(GPIO_ODR_2 | GPIO_ODR_10 | GPIO_ODR_11);
	GPIOC->ODR &= ~(GPIO_ODR_4 | GPIO_ODR_5);

	/* set to output */
	GPIOA->MODER |= (GPIO_MODER_MODER3_0 | GPIO_MODER_MODER4_0 | GPIO_MODER_MODER5_0 | GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0);
	GPIOB->MODER |= (GPIO_MODER_MODER2_0 | GPIO_MODER_MODER10_0 | GPIO_MODER_MODER11_0);
	GPIOC->MODER |= (GPIO_MODER_MODER4_0 | GPIO_MODER_MODER5_0);

	/* set to push-pull */
	GPIOA->OTYPER &= ~(GPIO_OTYPER_OT_3 | GPIO_OTYPER_OT_4 | GPIO_OTYPER_OT_5 | GPIO_OTYPER_OT_6 | GPIO_OTYPER_OT_7);
	GPIOB->OTYPER &= ~(GPIO_OTYPER_OT_2 | GPIO_OTYPER_OT_10 | GPIO_OTYPER_OT_11);
	GPIOC->OTYPER &= ~(GPIO_OTYPER_OT_4 | GPIO_OTYPER_OT_5);

	/* set to mid-speed */
	GPIOA->OSPEEDR |= (GPIO_OSPEEDR_OSPEEDR3_0 | GPIO_OSPEEDR_OSPEEDR4_0 | GPIO_OSPEEDR_OSPEEDR5_0
			| GPIO_OSPEEDR_OSPEEDR6_0 | GPIO_OSPEEDR_OSPEEDR7_0);
	GPIOB->OSPEEDR |= (GPIO_OSPEEDR_OSPEEDR2_0 | GPIO_OSPEEDR_OSPEEDR10_0 | GPIO_OSPEEDR_OSPEEDR11_0);
	GPIOC->OSPEEDR |= (GPIO_OSPEEDR_OSPEEDR4_0 | GPIO_OSPEEDR_OSPEEDR5_0);


#endif
}

/* sets the SRCLK (Serial Clock) pin high */
void srclk_high(void) {
#ifdef DEMO
	GPIOC->ODR |= (1 << SRCLK);
#else
	GPIOA->ODR |= (GPIO_ODR_6);
#endif
}

/* sets the SRCLK (Serial Clock) pin low */
void srclk_low(void) {
#ifdef DEMO
	GPIOC->ODR &= ~(1 << SRCLK);
#else
	GPIOA->ODR &= ~(GPIO_ODR_6);
#endif
}

/* sets the !SRCLR (!Serial Clear) pin high */
void srclr_latch_high(void) {
#ifdef DEMO
	GPIOA->ODR |= (1 << NSRCLR);
#else
	GPIOA->ODR |= (GPIO_ODR_7);
#endif
}

/* sets the !SRCLR (!Serial Clear) pin low */
void srclr_latch_low(void) {
#ifdef DEMO
	GPIOA->ODR &= ~(1 << NSRCLR);
#else
	GPIOA->ODR &= ~(GPIO_ODR_7);
#endif
}

/* sets the RCLK (Register Clock) pin high */
void rclk_high(void) {
#ifdef DEMO
	GPIOC->ODR |= (1 << RCLK);
#else
	GPIOC->ODR |= GPIO_ODR_4;
#endif
}

/* sets the RCLK (Register Clock) pin high */
void rclk_low(void) {
#ifdef DEMO
	GPIOC->ODR &= ~(1 << RCLK);
#else
	GPIOC->ODR &= ~GPIO_ODR_4;
#endif
}

/* Pulse clock pin to shift a bit in the shift registers  */
void pulse_clock() {
    srclk_high();
    rclk_high();

    srclk_low();
    rclk_low();
}


/* returns a value that will show the input value
 * in the uint8_t on 7-seg display
 */
uint8_t dec_to_sev_seg(uint8_t value) {
    switch(value) {
        case 0: return ZERO;
        case 1: return ONE;
        case 2: return TWO;
        case 3: return THREE;
        case 4: return FOUR;
        case 5: return FIVE;
        case 6: return SIX;
        case 7: return SEVEN;
        case 8: return EIGHT;
        case 9: return NINE;
        default: return ZERO;
    }
}

/* sets a pin for an assigned tube high or low
 *  @param uint8_t tube: 1-6
 *  @param uint8_t val: 0 (off) or 1 (on)
 * */
void assign_pin(uint8_t tube, uint8_t val) {
    if(val != 0) {
		switch(tube) {
#ifdef DEMO
			case 1: GPIOC->ODR |= (1 << SERIAL1); break;
			case 2: GPIOC->ODR |= (1 << SERIAL2); break;
			case 3: GPIOB->ODR |= (1 << SERIAL3); break;
			case 4: GPIOB->ODR |= (1 << SERIAL4); break;
			case 5: GPIOB->ODR |= (1 << SERIAL5); break;
			case 6: GPIOB->ODR |= (1 << SERIAL6); break;
#else
			case 1: GPIOA->ODR &= ~(1 << SERIAL1); break;
			case 2: GPIOA->ODR &= ~(1 << SERIAL2); break;
			case 3: GPIOC->ODR &= ~(1 << SERIAL3); break;
			case 4: GPIOB->ODR &= ~(1 << SERIAL4); break;
			case 5: GPIOB->ODR &= ~(1 << SERIAL5); break;
			case 6: GPIOB->ODR &= ~(1 << SERIAL6); break;
#endif
		}
    }
    else {
		switch(tube) {
#ifdef	DEMO
			case 1: GPIOC->ODR &= ~(1 << SERIAL1); break;
			case 2: GPIOC->ODR &= ~(1 << SERIAL2); break;
			case 3: GPIOB->ODR &= ~(1 << SERIAL3); break;
			case 4: GPIOB->ODR &= ~(1 << SERIAL4); break;
			case 5: GPIOB->ODR &= ~(1 << SERIAL5); break;
			case 6: GPIOB->ODR &= ~(1 << SERIAL6); break;
#else
			case 1: GPIOA->ODR &= ~(1 << SERIAL1); break;
			case 2: GPIOA->ODR &= ~(1 << SERIAL2); break;
			case 3: GPIOC->ODR &= ~(1 << SERIAL3); break;
			case 4: GPIOB->ODR &= ~(1 << SERIAL4); break;
			case 5: GPIOB->ODR &= ~(1 << SERIAL5); break;
			case 6: GPIOB->ODR &= ~(1 << SERIAL6); break;
#endif
		}
    }
}

/* updates only the two tubes related to seconds */
void update_seconds(uint8_t decSecs) {
    uint8_t segSecsOne = dec_to_sev_seg(decSecs / 10);
    uint8_t segSecsTwo = dec_to_sev_seg(decSecs % 10);

    srclr_latch_high(); 	// set latch (!SRCLR)  low

    shift_out(5, segSecsOne);
    shift_out(6, segSecsTwo);

    srclr_latch_low();	// set latch (!SRCLR) high
}

/* updates only the two tubes related to minutes */
void update_minutes(uint8_t decMins, uint8_t decSecs) {
    uint8_t segMinsOne = dec_to_sev_seg(decMins / 10);
    uint8_t segMinsTwo = dec_to_sev_seg(decMins % 10);
    uint8_t segSecsOne = dec_to_sev_seg(decSecs / 10);
    uint8_t segSecsTwo = dec_to_sev_seg(decSecs % 10);

    srclr_latch_high();	// latch (!SRCLR), set high

    // write the values to the tubes
    shift_out(3, segMinsOne);
    shift_out(4, segMinsTwo);
    shift_out(5, segSecsOne);
    shift_out(6, segSecsTwo);

    srclr_latch_low();	 // set latch (!SRCLR) low again
}

/* places a value in the shift register
 *  @param uint8_t tubeNumber: 1-6
 *  @param uint8_t val: 0 (off) or 1 (on)
 * */
void shift_out(uint8_t tubeNumber, uint8_t val) {
    uint8_t i;
    for(i = 0; i < 8; i++) {
        assign_pin(tubeNumber, (val & (1 << i))); // (SER) for tube 1
        pulse_clock();
    }
    rclk_low();
}

/* updates hours, minutes, and seconds */
void update_time(uint8_t decHrs, uint8_t decMins, uint8_t decSecs) {
#ifdef TUBE_TESTING
    uint8_t segHrsOne = 0xAA;
    uint8_t segHrsTwo = 0xAA;
    uint8_t segMinsOne = 0xAA;
    uint8_t segMinsTwo = 0xAA;
    uint8_t segSecsOne = 0xAA;
    uint8_t segSecsTwo = 0xAA;

#elif DISPLAY_BINARY
    uint8_t segHrsOne = (decHrs / 10);
    uint8_t segHrsTwo = (decHrs % 10);
    uint8_t segMinsOne = (decMins / 10);
    uint8_t segMinsTwo = (decMins % 10);
    uint8_t segSecsOne = (decSecs / 10);
    uint8_t segSecsTwo = (decSecs % 10);
#else
    uint8_t segHrsOne = dec_to_sev_seg(decHrs / 10);
    uint8_t segHrsTwo = dec_to_sev_seg(decHrs % 10);
    uint8_t segMinsOne = dec_to_sev_seg(decMins / 10);
    uint8_t segMinsTwo = dec_to_sev_seg(decMins % 10);
    uint8_t segSecsOne = dec_to_sev_seg(decSecs / 10);
    uint8_t segSecsTwo = dec_to_sev_seg(decSecs % 10);
#endif

    srclr_latch_high();	// latch (!SRCLR), set high

    // write the values to the tubes
    uint8_t i;
    for(i = 0; i < 8; i++) {
    	/* set pins high or low to set segment high or low */
        assign_pin(1, (segHrsOne & (1 << i))); // (SER) for tube 1
        assign_pin(2, (segHrsTwo & (1 << i))); // (SER) for tube 2
        assign_pin(3, (segMinsOne & (1 << i))); // (SER) for tube 3
        assign_pin(4, (segMinsTwo & (1 << i))); // (SER) for tube 4
        assign_pin(5, (segSecsOne & (1 << i))); // (SER) for tube 5
        assign_pin(6, (segSecsTwo & (1 << i))); // (SER) for tube 6

        /* force shift */
        pulse_clock();
    }

    rclk_low();
    srclr_latch_low();	// set latch (!SRCLR0) low again
}

/* updates the temperature to display '  ##oF  ' */
void display_temperature(uint8_t temperature) {
    uint8_t temperatureOne = dec_to_sev_seg(temperature / 10);
    uint8_t temperatureTwo = dec_to_sev_seg(temperature % 10);
    uint8_t degrees = DEGREES;
    uint8_t letter_f = LETTER_F;

    srclr_latch_high();	// latch (!SRCLR), set high

    // write the values to the tubes
    uint8_t i;
    for(i = 0; i < 8; i++) {
    	/* set pins high or low to set segment high or low */
        assign_pin(1, (0 & (1 << i))); // (SER) for tube 1
        assign_pin(2, (0 & (1 << i))); // (SER) for tube 2
        assign_pin(3, (temperatureOne & (1 << i))); // (SER) for tube 3
        assign_pin(4, (temperatureTwo & (1 << i))); // (SER) for tube 4
        assign_pin(5, (degrees & (1 << i))); // (SER) for tube 5
        assign_pin(6, (letter_f & (1 << i))); // (SER) for tube 6

        /* force shift */
        pulse_clock();
    }

    rclk_low();
    srclr_latch_low();	// set latch (!SRCLR0) low again
}

void display_date(void) {
	uint8_t m_one = dec_to_sev_seg((RTC->DR & RTC_DR_MT));
	uint8_t m_two = dec_to_sev_seg((RTC->DR & RTC_DR_MU));
	uint8_t d_one = dec_to_sev_seg((RTC->DR & RTC_DR_DT));
	uint8_t d_two = dec_to_sev_seg((RTC->DR & RTC_DR_DU));

    srclr_latch_high();	// latch (!SRCLR), set high

    // write the values to the tubes
    uint8_t i;
    for(i = 0; i < 8; i++) {
    	/* set pins high or low to set segment high or low */
        assign_pin(1, (m_one & (1 << i)));	// (SER) for tube 1
        assign_pin(2, (m_two & (1 << i)));	// (SER) for tube 2
        assign_pin(3, (0 & (1 << i))); 		// (SER) for tube 3
        assign_pin(4, (0 & (1 << i))); 		// (SER) for tube 4
        assign_pin(5, (d_one & (1 << i)));	// (SER) for tube 5
        assign_pin(6, (d_two & (1 << i)));	// (SER) for tube 6

        /* force shift */
        pulse_clock();
    }

    rclk_low();
    srclr_latch_low();	// set latch (!SRCLR0) low again
}


