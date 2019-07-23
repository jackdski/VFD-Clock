/*
 * tubes.h
 *
 *  Created on: Jun 14, 2019
 *      Author: jack
 */

#ifndef TUBES_H_
#define TUBES_H_

/* sets the SRCLK (Serial Clock) pin high */
void srclk_high(void);

/* sets the SRCLK (Serial Clock) pin low */
void srclk_low(void);

/* sets the !SRCLR (!Serial Clear) pin high */
void srclr_latch_high(void);

/* sets the !SRCLR (!Serial Clear) pin low */
void srclr_latch_low(void);

/* sets the RCLK (Register Clock) pin high */
void rclk_high(void);

/* sets the RCLK (Register Clock) pin high */
void rclk_low(void);

/* Pulse clock pin to shift a bit in the shift registers  */
void pulse_clock();

/* Configures the shift registers to be used */
void configure_shift_pins();

/* returns a value that will show the input value
 * in the uint8_t on 7-seg display
 */
uint8_t dec_to_sev_seg(uint8_t value);

/* sets a pin for an assigned tube high or low
 *  @param uint8_t tube: 1-6
 *  @param uint8_t val: 0 (off) or 1 (on)
 * */
void assign_pin(uint8_t tube, uint8_t val);

/* updates only the two tubes related to seconds */
void update_seconds(uint8_t decSecs);

/* updates only the two tubes related to minutes */
void update_minutes(uint8_t decMins, uint8_t decSecs);

/* places a value in the shift register
 *  @param uint8_t tubeNumber: 1-6
 *  @param uint8_t val: 0 (off) or 1 (on)
 * */
void shift_out(uint8_t tubeNumber, uint8_t val);

/* updates hours, minutes, and seconds */
void update_time(uint8_t decHrs, uint8_t decMins, uint8_t decSecs);

/* displays the temperature '  ##oF  ' */
void display_temperature(uint8_t temperature);

/* displays the date 'MM  DD' */
void display_date(void);

#endif /* TUBES_H_ */
