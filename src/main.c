/*	D E V I C E   I N C L U D E S   */
#include "main.h"
#include "stm32f091xc.h"
#include <stdint.h>

/*	F R E E R T O S   I N C L U D E S   */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "message_buffer.h"

/*	A P P L I C A T I O N   I N C L U D E S   */
#include "vfd_typedefs.h"
#include "circular_buffer.h"
#include "gpio.h"
#include "clocks.h"
#include "usart.h"
#include "i2c.h"
#include "tubes.h"
#include "pwm.h"
#include "adc.h"
#include "rtc.h"
#include "tsc.h"

/*	T A S K S   */
#include "sensor_tasks.h"

/*	F R E E R T O S   H O O K S   */
void vApplicationMallocFailedHook( void );
void vApplicationIdleHook( void );
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );
void vPreSleepProcessing( uint32_t ulExpectedIdleTime );
void vApplicationTickHook( void);
void *malloc( size_t xSize );

/* C I R C U L A R   B U F F E R S   */
CircBuf_t * TX_Buffer;
CircBuf_t * RX_Buffer;

/*	T A S K   N O T I F I C A T I O N S   */
TaskHandle_t thRTC = NULL;
TaskHandle_t thBrightness_Adj = NULL;
TaskHandle_t thAutoBrightAdj = NULL;

/* S O F T W A R E   T I M E R S   */
TimerHandle_t five_sec_timer = NULL;

/*	G L O B A L   V A R I A B L E S   */
volatile System_State_E system_state = Clock;

// initialize time to 12:00:00pm
volatile uint8_t hours = 12;		/* 1-12*/
volatile uint8_t minutes = 0;		/* 0-59 */
volatile uint8_t seconds = 0;		/* 0-59 */

volatile int8_t temperature = 1;	/* -128 - 127 */

volatile uint32_t light_value = 200;
volatile uint16_t display_brightness = 50;
volatile uint8_t usart_msg = 0;

/*	M A I N   */
int main(void) {
	/* create circular buffers for BLE mesages */
	TX_Buffer = create_CircBuf(50);
	RX_Buffer = create_CircBuf(50);

	/* initialize peripherals */
	init_sysclock();
	init_rtc();
	change_rtc_time(hours, minutes, seconds, 1);	// init to 12:00:00pm
	change_rtc_date(7, 27);		// init to 7/27
	init_led();
	init_buttons();
	init_i2c();
	init_usart();
	configure_shift_pins();
	init_pwm();
	init_adc();
	init_tsc();

    /* Priority 5 Tasks */
	BaseType_t rtcReturned = xTaskCreate(prvRTC_Task, "RTC", configMINIMAL_STACK_SIZE, NULL, 5, &thRTC);

	/* Priority 4 Tasks */

	/* Priority 3 Tasks */
//	BaseType_t tempReturned = xTaskCreate( prvTemperature_Task, "TempSensor", configMINIMAL_STACK_SIZE, NULL, 3, NULL);
	BaseType_t Lightreturned = xTaskCreate( prvLight_Task, "LightSensor", configMINIMAL_STACK_SIZE, (void *)NULL, 3, &thAutoBrightAdj);
	BaseType_t BLERXreturned = xTaskCreate( prvBLE_Receive_Task, "BLE RX", 300, (void *)NULL, 3, NULL);
	BaseType_t BLETXreturned = xTaskCreate( prvBLE_Send_Task, "BLE TX", 300, (void *)NULL, 3, NULL);

	/* Priority 2 Tasks */

	/* Priority 1 Tasks*/
	BaseType_t brightnessReturned = xTaskCreate( prvChange_Brightness_Task, "BrightnessAdj", configMINIMAL_STACK_SIZE, (void *)NULL, 1, &thBrightness_Adj);
	BaseType_t blinkyReturned = xTaskCreate( prvBlink_LED, "Blinky", configMINIMAL_STACK_SIZE, (void *)NULL, 1, NULL);

	/* Suspend the brightness adjustment task so it is only altered by prvLight_Task */
    vTaskSuspend( thBrightness_Adj );

	/* check that tasks were created successfully */
	if(rtcReturned != pdPASS) {
		toggle_error_led();
		while(1);
	}

//    if(tempReturned != pdPASS) {
//		toggle_error_led();
//    	while(1);
//    }

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

    if(brightnessReturned != pdPASS) {
		toggle_error_led();
    	while(1);
    }

    if(blinkyReturned != pdPASS) {
		toggle_error_led();
    	while(1);
    }

    /* initialize software timer */
	five_sec_timer = xTimerCreate("5s Timer", pdMS_TO_TICKS(5000), pdFALSE, 0, five_sec_timer_callback);

    /* initialize SysTick timer to 10ms ticks */
    SysTick_Config(60000);

    /* TODO: check on/off switch position before starting scheduler */

    size_t free_heap_size = xPortGetFreeHeapSize();		// used to debugging how big the heap needs to be

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
    	if(system_state == Switch_Sleep || system_state == BLE_Sleep) {
    		//    	__WFI();
    	}
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
