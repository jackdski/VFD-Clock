/*	D E V I C E   I N C L U D E S   */
#include "main.h"
#include "stm32f091xc.h"
#include <stdint.h>

/*	F R E E R T O S   I N C L U D E S   */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/*	A P P L I C A T I O N   I N C L U D E S   */
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

/*	F R E E R T O S   H O O K S   */
void vApplicationMallocFailedHook( void );
void vApplicationIdleHook( void );
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );
void vPreSleepProcessing( uint32_t ulExpectedIdleTime );
void vApplicationTickHook( void);
void *malloc( size_t xSize );

/*	G L O B A L   V A R I A B L E S   */
static uint8_t hours = 0;
static uint8_t minutes = 0;
static uint8_t seconds = 0;

static uint8_t temperature;

volatile uint32_t light_value = 0;

volatile uint8_t usart_msg = 0;

/*	Q U E U E S   */
QueueHandle_t BLE_Queue = NULL;		// cannot be static if used across multiple files


/*	M A I N   */
int main(void)
{
	init_sysclock();
	init_led();
	init_buttons();
	init_i2c();
	init_usart();
	configure_shift_pins();
	init_pwm();
	init_adc();

	/* create all queues */
	BLE_Queue = xQueueCreate(10, 8);	// 10 items, each 4-bytes

	/* create all tasks */
	BaseType_t tempReturned = xTaskCreate( prvTemperature_Task, "TempSensor", configMINIMAL_STACK_SIZE, NULL, 3, NULL);
    BaseType_t blinkyReturned = xTaskCreate( prvBlink_LED, "Blinky", configMINIMAL_STACK_SIZE, (void *)NULL, 3, NULL);
    BaseType_t tubesReturned = xTaskCreate( prvUpdateTubes, "UpdateTubes", configMINIMAL_STACK_SIZE, (void *)NULL, 5, NULL);
    BaseType_t BLEreturned = xTaskCreate( prvBLE_Update_Task, "BLE", configMINIMAL_STACK_SIZE, (void *)NULL, 3, NULL);
    BaseType_t PWMreturned = xTaskCreate( prvChangePWM, "PWM", configMINIMAL_STACK_SIZE, (void *)NULL, 5, NULL);
    BaseType_t Lightreturned = xTaskCreate( prvLight_Task, "LightSensor", configMINIMAL_STACK_SIZE, (void *)NULL, 4, NULL);


    if(tempReturned != pdPASS) {
    	toggle_led();
    	while(1);
    }

    if(blinkyReturned != pdPASS) {
    	toggle_led();
    	while(1);
    }

    if(tubesReturned != pdPASS) {
    	toggle_led();
    	while(1);
    }

    if(BLEreturned != pdPASS) {
    	toggle_led();
    	while(1);
    }

    if(PWMreturned != pdPASS) {
    	toggle_led();
    	while(1);
    }

    if(Lightreturned != pdPASS) {
    	toggle_led();
    	while(1);
    }


	SysTick_Config(60000);		// initialize SysTick timer to 10ms ticks

    /* start scheduler */
    vTaskStartScheduler();
    for( ;; );
}

void vApplicationMallocFailedHook( void )
{
    /* vApplicationMallocFailedHook() will only be called if
    configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
    function that will get called if a call to pvPortMalloc() fails.
    pvPortMalloc() is called internally by the kernel whenever a task, queue,
    timer or semaphore is created.  It is also called by various parts of the
    demo application.  If heap_1.c or heap_2.c are used, then the size of the
    heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
    FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
    to query the size of free heap space that remains (although it does not
    provide information on how the remaining heap might be fragmented). */
    taskDISABLE_INTERRUPTS();
    for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
    /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
    to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
    task.  It is essential that code added to this hook function never attempts
    to block in any way (for example, call xQueueReceive() with a block time
    specified, or call vTaskDelay()).  If the application makes use of the
    vTaskDelete() API function (as this demo application does) then it is also
    important that vApplicationIdleHook() is permitted to return to its calling
    function, because it is the responsibility of the idle task to clean up
    memory allocated by the kernel to any task that has since been deleted. */
    for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    ( void ) pcTaskName;
    ( void ) pxTask;

    /* Run time stack overflow checking is performed if
    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected. */
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
    /* There should not be a heap defined, so trap any attempts to call
    malloc. */
//    Interrupt_disableMaster();
    for( ;; );
}
