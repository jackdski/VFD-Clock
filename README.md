This project aims to create a unique display for time, date, and temperature data on 
    seven-segment IV-11 Vacuum Fluorescent Display (VFD) tubes. To accomplish this, an STM32F0 
    microcontroller, shift-registers, logic-level shifters, boost- and buck-converters, a digital 
    temperature sensor, capacative-touch, and a photoresistor are used. 
    
To accomplish this, several goals were outlined:
- Use FreeRTOS to gain experience with RTOS's
- Use Bluetooth Low-Energy to wirelessly communicate instructions and auto-sync time and date data from an iPhone
    - Manually select the time
    - Turn off auto-brightness adjustments
    - Select whether to display temperature or the date temporarily
    - Turn the VFD display on or off and put into low-power mode
- Adapt brightness on displays using PWM and sampling ambient brightness using ADC and two photoresistors, located on opposite corners of the PCB
- Retain RTC operation using a coin-cell battery without wall power for minimum 1 year
- Use I2C to collect temperature data from a digital temperature sensor and display it on the tubes
- Use capacative touch on the corners of the PCB to display the ambient temperature or date while the capacative "button" is touched

Progress:
- [x] I2C Driver
- [x] Communicate with temp. sensor over I2C
- [x] Configure time and interrupt to change display brightness smoothly
- [x] Adapt brightness based off of photoresistor ADC measurements
- [x] PWM to !Output-Enable pin on shift register using TIM14 timer
- [x] Write to shift registers
- [x] Interrupts for buttons and switches
- [ ] Configure time/date using buttons and switches
- [ ] Implement semaphores for RTC Alarm A
- [x] Configure RTC Alarm A for 1Hz interrupt
- [x] Enable Low-Power mode in Idle Task
- [ ] Deep Sleep (turn off tubes, disable everything except for RTC)
- [x] USART driver
- [ ] Communicate to iPhone over BLE
- [ ] iPhone app
