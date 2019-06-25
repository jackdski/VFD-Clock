/*
 * FreeRTOSConfig.h
 *
 *  Created on: Jun 14, 2019
 *      Author: jack
 */

#ifndef FREERTOSCONFIG_H_
#define FREERTOSCONFIG_H_

//#include <stdint.h>
//extern uint32_t SystemCoreClock;

#include "stm32f0xx_it.h"

/* redirect FreeRTOS port interrupts */
//#define vPortSVCHandler							SVC_handler
//#define xPortPendSVHandler						pending_SV_handler
//#define xPortSysTickHandler						SysTick_handler


#define vPortSVCHandler 		SVC_Handler
#define xPortPendSVHandler 		PendSV_Handler
#define xPortSysTickHandler 	SysTick_Handler
//
///* Config */
//#define configUSE_PREEMPTION                    1
//#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
//#define configUSE_TICKLESS_IDLE                 0
//#define configCPU_CLOCK_HZ                      48000000	// 48MHz
//#define configTICK_RATE_HZ						(( TickType_t) 1000 )
//
//#define configMAX_PRIORITIES                    ( 5 )
//#define configMINIMAL_STACK_SIZE                128
//#define configMAX_TASK_NAME_LEN                 16
//
//#define configUSE_16_BIT_TICKS                  0
//#define configIDLE_SHOULD_YIELD                 1
//#define configUSE_TASK_NOTIFICATIONS            1
//
//#define configUSE_MUTEXES                       1
//#define configUSE_RECURSIVE_MUTEXES             1
//#define configUSE_COUNTING_SEMAPHORES           1
//#define configUSE_ALTERNATIVE_API               0 /* Deprecated! */
//#define configQUEUE_REGISTRY_SIZE               8
//#define configUSE_QUEUE_SETS                    0
//#define configUSE_TIME_SLICING                  1
//#define configUSE_NEWLIB_REENTRANT              0
//#define configENABLE_BACKWARD_COMPATIBILITY     0
//#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 5
//#define configSTACK_DEPTH_TYPE                  uint16_t
//#define configMESSAGE_BUFFER_LENGTH_TYPE        size_t
//
///* Memory allocation related definitions. */
//#define configSUPPORT_STATIC_ALLOCATION         0
//#define configSUPPORT_DYNAMIC_ALLOCATION        1
//#define configTOTAL_HEAP_SIZE					( ( size_t ) ( 6500 ) )
//#define configAPPLICATION_ALLOCATED_HEAP        0
//
///* Hook function related definitions. */
//#define configUSE_IDLE_HOOK                     1
//#define configUSE_TICK_HOOK                     1
//#define configCHECK_FOR_STACK_OVERFLOW          2
//#define configUSE_MALLOC_FAILED_HOOK            1
//#define configUSE_DAEMON_TASK_STARTUP_HOOK      0
//
///* Run time and task stats gathering related definitions. */
//#define configGENERATE_RUN_TIME_STATS           0
//#define configUSE_TRACE_FACILITY                0
//#define configUSE_STATS_FORMATTING_FUNCTIONS    1
//
///* Co-routine related definitions. */
//#define configUSE_CO_ROUTINES                   0
//#define configMAX_CO_ROUTINE_PRIORITIES         1
//
///* Software timer related definitions. */
//#define configUSE_TIMERS                        1
//#define configTIMER_TASK_PRIORITY               ( 2 )
//#define configTIMER_QUEUE_LENGTH                5
//#define configTIMER_TASK_STACK_DEPTH            configMINIMAL_STACK_SIZE
//
///* Interrupt nesting behaviour configuration. */
///* Taken from MSP432 FreeRTOS config */
///* The lowest interrupt priority that can be used in a call to a "set priority"
//function. */
//#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY			0x07
//#define configPRIO_BITS							3
//
///* The highest interrupt priority that can be used by any interrupt service
//routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
//INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
//PRIORITY THAN THIS! (higher priorities are lower numeric values. */
//#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY	5
//
///* Interrupt priorities used by the kernel port layer itself.  These are generic
//to all Cortex-M ports, and do not rely on any particular library functions. */
//#define configKERNEL_INTERRUPT_PRIORITY 		( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
///* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
//See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
//#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
///* end taken from MSP432 FreeRTOS config */
//
///* Define to trap errors during development. */
//#define configASSERT( x ) if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for( ;; ); }

#define configUSE_PREEMPTION			1
#define configUSE_IDLE_HOOK				0
#define configUSE_TICK_HOOK				1
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
#define configGENERATE_RUN_TIME_STATS	0

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 			0
#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )

/* Software timer definitions. */
#define configUSE_TIMERS				1
#define configTIMER_TASK_PRIORITY		( 2 )
#define configTIMER_QUEUE_LENGTH		5
#define configTIMER_TASK_STACK_DEPTH	( 80 )


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


/* A header file that defines trace macro can be included here. */

#endif /* FREERTOSCONFIG_H_ */
