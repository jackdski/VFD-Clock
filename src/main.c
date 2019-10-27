/*	D E V I C E   I N C L U D E S   */
#include "main.h"
#include "stm32f091xc.h"
#include <stdint.h>

/*	F R E E R T O S   I N C L U D E S   */
#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"

/*	A P P L I C A T I O N   I N C L U D E S   */
#include "vfd_typedefs.h"
#include "sensor_tasks.h"
#include "circular_buffer.h"
#include "gpio.h"
#include "clocks.h"
#include "usart.h"
#include "i2c.h"
#include "tubes.h"
#include "pwm.h"
#include "adc.h"
#include "rtc.h"
#include "callbacks.h"
#include "low_power.h"

/*	D E F I N E S   */
#define STARTING_DISPLAY_BRIGHTNESS 	50	// initialize display to 50% brightness

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

/*	T A S K   H A N D L E S   */
TaskHandle_t thRTC = NULL;
TaskHandle_t thOff = NULL;
TaskHandle_t thConfig = NULL;
TaskHandle_t thBrightness_Adj = NULL;
TaskHandle_t thAutoBrightAdj = NULL;
TaskHandle_t thErrorLED = NULL;
TaskHandle_t thBLErx = NULL;
TaskHandle_t thBLEtx = NULL;
TaskHandle_t thTemperatureButton = NULL;

/* S O F T W A R E   T I M E R S   */
TimerHandle_t three_sec_timer = NULL;
TimerHandle_t five_sec_timer = NULL;
TimerHandle_t ten_sec_timer = NULL;
TimerHandle_t button_timer = NULL;

/*	G L O B A L   V A R I A B L E S   */
volatile System_State_E system_state = Clock;

//volatile int8_t temperature = 1;	/* -128 - 127 */
volatile uint8_t holds = 0;


Button_Status_E plus_button_status = Open;
Button_Status_E minus_button_status = Open;
Light_Flash_E indication_light_status = Off;
Light_Flash_E error_light_status = Off;
Time_Change_Speed_E change_speed = Slow;
Time_Config_Options_E time_config = Reset;
HC_10_Status_E ble_status = Disconnected;
Efuse_Status_E efuse_status = Normal;


/*	M A I N   */
int main(void) {
	init_power_switch();	// initialize first to be able to read switches and buttons

    /* check on/off switch position before initializing and starting scheduler,
     * should only happen the first time the device is powered up */
    if((GPIOC->IDR & (1 << 13)) == 13) {
		configure_for_deepsleep();
		__WFI();  // enter DeepSleep/Standby Mode
    }

    /* check if device was set to Standby mode prior to running this code again
     	 if it was, use the time in the RTC registers */

	RCC->APB1ENR |= RCC_APB1ENR_PWREN;  // enable PWR peripheral clock

	/* device was woken up from Standby mode */
    if(PWR->CSR & PWR_CSR_WUF) {
    	init_indication_led();
    	time_config = Standby_Wakeup;
    }

	/* create circular buffers for BLE messages */
	TX_Buffer = create_CircBuf(50);
	RX_Buffer = create_CircBuf(50);

	/* initialize peripherals */
	init_wwdg();
	init_sysclock();
	init_rtc();
	init_error_led();
	init_indication_led();
	init_buttons();
	init_i2c(SENSOR_I2C);
	init_usart(BLE_USART);
	configure_shift_pins();
	init_pwm();
	init_adc();
	wake_up_hc_10();

	/*   C R E A T E   T A S K S   */

    /* Priority 3 Tasks */
	BaseType_t rtcReturned = xTaskCreate(prvRTC_Task, "RTC", configMINIMAL_STACK_SIZE, (void *) time_config, 3, &thRTC);
	BaseType_t sleepReturned = xTaskCreate(prvTurnOffTask, "Turn Off", configMINIMAL_STACK_SIZE, NULL, 3, &thOff);
    BaseType_t configReturned = xTaskCreate(prvConfig_Task, "Config", configMINIMAL_STACK_SIZE, NULL, 3, &thConfig);

	/* Priority 2 Tasks */
//	BaseType_t tempReturned = xTaskCreate( prvTemperature_Task, "TempSensor", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
	BaseType_t Lightreturned = xTaskCreate( prvLight_Task, "LightSensor", configMINIMAL_STACK_SIZE, (void *)NULL, 2, &thAutoBrightAdj);
	BaseType_t BLERXreturned = xTaskCreate( prvBLE_Receive_Task, "BLE RX", 300, (void *)NULL, 2, &thBLErx);
	BaseType_t BLETXreturned = xTaskCreate( prvBLE_Send_Task, "BLE TX", 300, (void *)NULL, 2, &thBLEtx);
	BaseType_t TempButtonreturned = xTaskCreate( prvTemperature_Task, "Temp Button", configMINIMAL_STACK_SIZE, (void *)NULL, 2, &thTemperatureButton);

	/* Priority 1 Tasks*/
	BaseType_t brightnessReturned = xTaskCreate( prvChange_Brightness_Task, "BrightnessAdj", configMINIMAL_STACK_SIZE, (void *)STARTING_DISPLAY_BRIGHTNESS, 1, &thBrightness_Adj);
	BaseType_t errorLightReturned = xTaskCreate( prvError_LED, "ErrorLED", configMINIMAL_STACK_SIZE, (void *)NULL, 1, NULL);
	BaseType_t blinkyReturned = xTaskCreate( prvIndication_LED, "Blinky", configMINIMAL_STACK_SIZE, (void *)NULL, 1, &thErrorLED);

	/* Suspended Tasks */
    vTaskSuspend( thBrightness_Adj );	/* suspend the brightness adjustment task so it is only altered by prvLight_Task */
    vTaskSuspend( thErrorLED );			/* suspend the error LED task so it only flashes when an error occurrs */

	/* check that tasks were created successfully */
	configASSERT(rtcReturned == pdPASS);
	configASSERT(sleepReturned == pdPASS);
	configASSERT(configReturned = pdPASS);
	configASSERT(TempButtonreturned == pdPASS);
	configASSERT(BLERXreturned == pdPASS);
	configASSERT(BLETXreturned == pdPASS);
	configASSERT(Lightreturned == pdPASS);
	configASSERT(brightnessReturned == pdPASS);
	configASSERT(errorLightReturned == pdPASS);
	configASSERT(blinkyReturned == pdPASS);

    /* initialize software timers */
    three_sec_timer = xTimerCreate("3s Timer", pdMS_TO_TICKS(100), pdTRUE, 0, three_sec_timer_callback);
	five_sec_timer = xTimerCreate("5s Timer", pdMS_TO_TICKS(5000), pdFALSE, 0, five_sec_timer_callback);
    ten_sec_timer = xTimerCreate("10s Timer", pdMS_TO_TICKS(10000), pdFALSE, 0, ten_sec_timer_callback);
	button_timer = xTimerCreate("Button Timer", pdMS_TO_TICKS(50), pdTRUE, 0, button_timer_callback);

    /* initialize SysTick timer to 10ms ticks */
    SysTick_Config(60000);

    // size_t free_heap_size = xPortGetFreeHeapSize();		// used to debug how big the heap needs to be


    // check HC-10 status to see if it has already connected to a device
    get_hc_10_status();
    if( ble_status != Connected ) {
    	vTaskSuspend(thBLEtx);
    }

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
//    for( ;; ) {	}
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

void vApplicationTickHook( void ) {
//    for( ;; );
}

void *malloc( size_t xSize ) {
    for( ;; );
}
