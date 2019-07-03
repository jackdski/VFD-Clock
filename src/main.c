/*	D E V I C E   I N C L U D E S   */
#include "main.h"
#include "stm32f091xc.h"
#include <stdint.h>

/*	F R E E R T O S   I N C L U D E S   */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/*	A P P L I C A T I O N   I N C L U D E S   */
#include "vfd_typedefs.h"
#include "gpio.h"
#include "clocks.h"
#include "usart.h"
#include "i2c.h"
#include "tubes.h"
#include "pwm.h"
#include "adc.h"
#include "rtc.h"

/*	T A S K S   */
#include "sensor_tasks.h"

/*	Q U E U E S   */
volatile QueueHandle_t BLE_Queue = NULL;		// cannot be static if used across multiple files

/*	S E M A P H O R E S   */

/*	T A S K   N O T I F I C A T I O N S   */
TaskHandle_t thRTC = NULL;
TaskHandle_t thTubes = NULL;

/*	F R E E R T O S   H O O K S   */
void vApplicationMallocFailedHook( void );
void vApplicationIdleHook( void );
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );
void vPreSleepProcessing( uint32_t ulExpectedIdleTime );
void vApplicationTickHook( void);
void *malloc( size_t xSize );

/*	G L O B A L   V A R I A B L E S   */
volatile System_State_E system_state = Clock;

volatile uint8_t hours = 0;			/* 0-23 */
volatile uint8_t minutes = 0;		/* 0-59 */
volatile uint8_t seconds = 0;		/* 0-59 */
volatile int8_t temperature = 0;	/* -128 - 127 */

volatile uint32_t light_value = 200;
volatile uint16_t display_brightness = 50;
volatile uint8_t usart_msg = 0;

/*	M A I N   */
int main(void) {
	/* create queues */
	BLE_Queue = xQueueCreate(20, 1);	// 20 items, each 1-bytes, enough space for 3 received messages

	/* initialize peripherals */
	init_sysclock();
	init_rtc();
	init_led();
	init_buttons();
	init_i2c();
	init_usart();
	configure_shift_pins();
	init_dimming_timer();
	init_pwm();
	init_adc();

	/* create tasks */
    /* Priority 5 Tasks */
	BaseType_t rtcReturned = xTaskCreate(prvRTC_Task, "RTC", configMINIMAL_STACK_SIZE, NULL, 5, &thRTC);
	// tubes task isn't needed, update_tubes() can be called from RTC task
//	BaseType_t tubesReturned = xTaskCreate( prvUpdateTubes, "UpdateTubes", configMINIMAL_STACK_SIZE, (void *)NULL, 4, NULL);

	/* Priority 4 Tasks */
//	BaseType_t PWMreturned = xTaskCreate( prvChangePWM, "PWM Change", configMINIMAL_STACK_SIZE, (void *)NULL, 5, NULL);

	/* Priority 3 Tasks */
	BaseType_t tempReturned = xTaskCreate( prvTemperature_Task, "TempSensor", configMINIMAL_STACK_SIZE, NULL, 3, NULL);
	BaseType_t Lightreturned = xTaskCreate( prvLight_Task, "LightSensor", configMINIMAL_STACK_SIZE, (void *)NULL, 3, NULL);
	BaseType_t BLERXreturned = xTaskCreate( prvBLE_Receive_Task, "BLE RX", configMINIMAL_STACK_SIZE, (void *)NULL, 3, NULL);
	BaseType_t BLETXreturned = xTaskCreate( prvBLE_Send_Task, "BLE TX", configMINIMAL_STACK_SIZE, (void *)NULL, 3, NULL);

	/* Priority 2 Tasks */

	/* Priority 1 Tasks*/
	BaseType_t blinkyReturned = xTaskCreate( prvBlink_LED, "Blinky", configMINIMAL_STACK_SIZE, (void *)NULL, 1, NULL);


	/* check that tasks were created successfully */
	if(rtcReturned != pdPASS) {
		toggle_error_led();
		while(1);
	}

    if(tempReturned != pdPASS) {
		toggle_error_led();
    	while(1);
    }

    if(blinkyReturned != pdPASS) {
		toggle_error_led();
    	while(1);
    }

    if(BLERXreturned != pdPASS) {
		toggle_error_led();
    	while(1);
    }

    if(BLETXreturned != pdPASS) {
		toggle_error_led();
    	while(1);
    }

    if(Lightreturned != pdPASS) {
		toggle_error_led();
    	while(1);
    }


    /* initialize SysTick timer to 10ms ticks */
    SysTick_Config(60000);

    /* TODO: check on/off switch position before starting scheduler */

    /* start scheduler */
    vTaskStartScheduler();
    for( ;; );
}

void vApplicationMallocFailedHook( void )
{
    taskDISABLE_INTERRUPTS();
    for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
    for( ;; ) {
    	// TODO: check that everything is in order, then put into low-power mode
//    	__WFI();
    }
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    ( void ) pcTaskName;
    ( void ) pxTask;
    taskDISABLE_INTERRUPTS();
    for( ;; );
}

void vPreSleepProcessing( uint32_t ulExpectedIdleTime ) {

}

void vApplicationTickHook( void) {
//    for( ;; );
}

void *malloc( size_t xSize )
{
    for( ;; );
}
