/*
 * mcp9808.h
 *
 *  Created on: Jun 15, 2019
 *      Author: jack
 */

#ifndef MCP9808_H_
#define MCP9808_H_

void config_temperature_sensor();

void read_config_register();

void enable_shutdown_mode();

void disable_shutdown_mode();

void write_mcp9808(uint32_t data);

uint8_t sample_temperature();

#endif /* MCP9808_H_ */
