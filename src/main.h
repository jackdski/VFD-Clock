/**
  ******************************************************************************
  * @file    GPIO/GPIO_IOToggle/Inc/main.h
  * @author  MCD Application Team
  * @version V1.6.0
  * @date    27-May-2016
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"
#include "stm32f0xx_nucleo.h"

/*	P R O F I L E R   */
#define PROFILER_TIMER			TIM3

/*	I 2 C   */
//#define	USE_I2C1
#ifdef	USE_I2C1
#define I2CX					I2C1
#define I2C_PORT				GPIOF
#define I2C_SDA_PIN				0		// PF0 - AF1
#define I2C_SCL					1		// PF1 - AF1
#define I2C_AF					1
#else
#define	SENSOR_I2C				I2C2
#define I2C_GPIO_PORT			GPIOA
#define I2C_SDA					12		// PA12 - AF5
#define I2C_SCL					11		// PA11 - AF5
#define I2C_AF					5
#endif

/* TMP36  */
#define	TMP36_OUT_PORT			GPIOB
#define TMP36_OUT_PIN			1
#define	TMP36_SHDW_PORT			GPIOB
#define TMP36_SHDW_PIN			2

/*	U S A R T   */
#define BLE_USART				USART1
#define BLE_TX_PORT				GPIOA
#define BLE_TX_PIN				9
#define BLE_RX_PORT				GPIOA
#define BLE_RX_PIN				10
#define BLE_STATUS_PORT			GPIOA
#define BLE_STATUS_PIN			8

/*	P W M   */
#define PWM_PORT				GPIOA
#define PWM_PIN					4
#define PWM_TIMER				TIM14
#define PWM_SOURCE_FREQUENCY	1000000		// 8MHz with prescalar of 8
#define	PWM_FREQUENCY			1000  //Hz	//400	// 400us or 2.5kHz
#define CALC_PWM_DUTY_CYCLE(X)	((X * PWM_SOURCE_FREQUENCY) / PWM_FREQUENCY)
#define BRIGHTNESS_LOW			10
#define BRIGHTNESS_MEDIUM		45
#define BRIGHTNESS_HIGH			70
#define BRIGHTNESS_MAX			99

/*	L O W   P O W E R   */
#define LP_TIMER				TIM1

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
