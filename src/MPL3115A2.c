/*
 * MPL3115A2.c
 *
 *  Created on: Jun 21, 2019
 *      Author: jack
 */

/*	D E V I C E   I N C L U D E S   */
#include "stm32f091xc.h"
#include "stm32f0xx_hal.h"
#include <stdint.h>

/*	A P P L I C A T I O N   I N C L U D E S   */
#include "i2c.h"

/* D E F I N E S */
/* MPL3115A2 Macros */
#define MPL3115A2           0x60 // 7-bit I2C address
//#define MPL3115A2           0xC0
#define STATUS              0x00
#define OUT_T_MSB           0x04
#define OUT_T_LSB           0x05
#define OUT_T_DELTA_MSB     0x0A
#define OUT_T_DELTA_LSB     0x0B
#define WHO_AM_I            0x0C
#define CTRL_REG1           0x26
#define CTRL_REG2			0x27
#define PT_DATA_CFG         0x13
#define ACTIVE              0xB9  //value to write


void config_temperature_sensor_mpl() {

}

void read_config_mpl() {
	i2c_read_reg(MPL3115A2, STATUS);
}

void check_whoami_mpl(void) {
	i2c_read_reg(MPL3115A2, WHO_AM_I);
}

void trigger_sample_mpl(void) {
	i2c_change_sadd(MPL3115A2);	// set MPL3115A2 as slave

	// set ST to 1 in CTRL_REG2
	i2c_write_reg(MPL3115A2, CTRL_REG2, 0x01); // set CTRL_REG2
	i2c_read_reg(MPL3115A2, CTRL_REG2);  // read CTRL_REG2

	i2c_write_reg(MPL3115A2, CTRL_REG1, 0x03);	// set CTRL_REG1
	i2c_read_reg(MPL3115A2, CTRL_REG1);	 // read CTRL_REG1
}

uint8_t read_temp_c(void) {
//	trigger_sample_mpl(); // trigger a new temp sample
//    uint8_t status = 0;
//    uint8_t temp_c;
    // wait for new temp data
//    while((status & 0x08) == 0) {
//        status = i2c_read_reg(OUT_T_MSB);
//    }
   return i2c_read_reg(MPL3115A2, OUT_T_MSB);
}

uint8_t convert_to_fahrenheit(uint8_t temp_c) {
    return (temp_c * 9) / 5 + 32;
}


uint8_t read_temp_f() {
    uint8_t final_temp = convert_to_fahrenheit(read_temp_c());
    return final_temp;
}

//void set_mode_standby() {
////    uint8_t setting = i2c_read_reg(CTRL_REG1);  // read setting
//    setting &= ~(1 << 0);                       // set standby bit
//    i2c_write_reg(MPL3115A2, CTRL_REG1, setting);          // write to reg
//}
//
//void set_mode_active() {
////    uint8_t setting = i2c_read_reg(CTRL_REG1);  // read setting
//    setting |= (1 << 0);                        // set active bit
//    i2c_write_reg(MPL3115A2, CTRL_REG1, setting);          // write to reg
//}
//
//
//void set_oversample_rate(uint8_t sample_rate) {
//  if(sample_rate > 7) sample_rate = 7;
//  sample_rate <<= 3;
////  uint8_t setting = i2c_read_reg(CTRL_REG1);    // read setting
//  setting &= 0b11000111;                        // clear out OS bits
//  setting |= sample_rate;                       // use new OS bits
//  i2c_write_reg(MPL3115A2, CTRL_REG1, setting);            // write to reg
//}


void enable_event_flags() {
  i2c_write_reg(MPL3115A2, PT_DATA_CFG, 0x07); // Enable all three pressure and temp event flags
}


