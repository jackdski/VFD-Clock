/*
 * FreeRTOSConfig.h
 *
 *  Created on: Jun 14, 2019
 *      Author: jack
 */

#ifndef FREERTOSCONFIG_H_
#define FREERTOSCONFIG_H_

#include <stdint.h>
#include "stm32f0xx_it.h"
#include "stm32f091xc.h"

#include "../inc/low_power.h"

volatile unsigned long ulHighFrequencyTimerTicks; 	// for profiling Tasks

/* redirect FreeRTOS port interrupts */
#define vPortSVCHandler 		SVC_Handler
#define xPortPendSVHandler 		PendSV_Handler
#define xPortSysTickHandler 	SysTick_Handler

#define configUSE_PREEMPTION			1
#define configUSE_IDLE_HOOK				0
#define configUSE_TICK_HOOK				0
#define configCPU_CLOCK_HZ				( 8000000 )
#define configTICK_RATE_HZ				( ( TickType_t ) 100 )
#define configMAX_PRIORITIES			( 5 )
#define configMINIMAL_STACK_SIZE		( ( unsigned short ) 60 )
#define configTOTAL_HEAP_SIZE			( ( size_t ) ( 6500 ) )
#define configMAX_TASK_NAME_LEN			( 16 )
#define configUSE_TRACE_FACILITY		1
#define configUSE_16_BIT_TICKS			0
#define configIDLE_SHOULD_YIELD			1
#define configUSE_MUTEXES				1
#define configQUEUE_REGISTRY_SIZE		8
#define configCHECK_FOR_STACK_OVERFLOW	2
#define configUSE_RECURSIVE_MUTEXES		1
#define configUSE_MALLOC_FAILED_HOOK	1
#define configUSE_APPLICATION_TASK_TAG	0
#define configUSE_COUNTING_SEMAPHORES	1
#define configUSE_TASK_NOTIFICATIONS	1
#define configUSE_TICKLESS_IDLE			2  // use user-defined tickless idle
#define configMESSAGE_BUFFER_LENGTH_TYPE			size_t//uint8_t

/* run time stats */
extern void init_timing_stats_timer(void);
#define configGENERATE_RUN_TIME_STATS				1
//#define configUSE_STATS_FORMATTING_FUNCTIONS		1
#define configUSE_TRACE_FACILITY					1
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()	init_timing_stats_timer()
#define portGET_RUN_TIME_COUNTER_VALUE()			ulHighFrequencyTimerTicks

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 			0
#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )

/* Software timer definitions. */
#define configUSE_TIMERS				1
#define configTIMER_TASK_PRIORITY		( 3 )
#define configTIMER_QUEUE_LENGTH		5
#define configTIMER_TASK_STACK_DEPTH	( 80 )

/* Low Power Definitions */
#define configEXPECTED_IDLE_TIME_BEFORE_SLEEP 	5
#define portSUPPRESS_TICKS_AND_SLEEP( xIdleTime )	vApplicationSleep( xIdleTime )

/* Normal assert() semantics without relying on the provision of an assert.h
header file. */
#define configASSERT( x ) if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for( ;; ); }

/* Optional functions - most linkers will remove unused functions anyway. */
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskCleanUpResources			1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_xResumeFromISR                  1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_uxTaskGetStackHighWaterMark     0
#define INCLUDE_xTaskGetIdleTaskHandle          0
#define INCLUDE_xTaskGetCurrentTaskHandle		1
#define INCLUDE_eTaskGetState                   1
#define INCLUDE_xEventGroupSetBitFromISR        1
#define INCLUDE_xTimerPendFunctionCall          1
#define INCLUDE_xTaskAbortDelay                 0
#define INCLUDE_xTaskGetHandle                  0
#define INCLUDE_xTaskResumeFromISR              1
#define INCLUDE_xSemaphoreGetMutexHolder		0
#define INCLUDE_eTaskGetState					1


/* A header file that defines trace macro can be included here. */

#endif /* FREERTOSCONFIG_H_ */
