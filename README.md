# VFD-Clock

This project aims to create a unique display for time, date, and temperature data on 
seven-segment IV-11 Vacuum Fluorescent Display (VFD) tubes. To accomplish this, an STM32F0 
microcontroller, shift-registers, logic-level shifters, boost- and buck-converters, a digital 
temperature sensor, buttons, switches, and photoresistors are used. 
    
To accomplish this, several goals were outlined:
- Use FreeRTOS to gain experience with RTOS's
- Use the Hc-10 Bluetooth Low-Energy module to wirelessly communicate instructions time and date data from an iPhone
    - Update the time
    - Turn off auto-brightness adjustments
    - Select whether to display temperature or the date temporarily
    - Turn the VFD display on or off and put into low-power mode
- Adapt brightness on displays using PWM and sampling ambient brightness using ADC and two photoresistors, located on opposite corners of the PCB
- Retain RTC operation using a coin-cell battery without wall power for minimum 1 year
- Use I2C to collect temperature data from a digital temperature sensor and display it on the tubes

Progress:
- [x] Auto-Brightness
	- [x] Adapt brightness based off of photoresistor ADC measurements
	- [x] Configure timer to change display brightness smoothly
	- [x] PWM to !Output-Enable pin on shift register using TIM14 timer
- [ ] Temperature Sensor
	- [x] I2C Driver
	- [x] Communicate with temp. sensor over I2C
	- [ ] Adapt to multiple temperature sensors
- [x] Control the display and use time from RTC
	- [x] Write to shift registers
	- [x] Implement task notification from RTC Alarm A to update tubes
	- [x] Configure RTC Alarm A for 1Hz interrupt
- [ ] Configure time/date using buttons and switches
	- [x] Interrupts for buttons and switches
	- [x] Go into Config mode after holding down +/- buttons for 3s
	- [x] Exit Config mode after 10s of inactivity
	- [x] Flash display at 2Hz while in config mode
	- [x] Add indicator LED that flashes quickly to indicate successful user input
	- [ ] Update RTC registers with new values
- [x] USART driver
- [ ] Communicate with iPhone over BLE
	- [x] Control Brightness
		- [x] Turn autobrightness on and off
		- [x] Choose a brightness value (0-99)
	- [x] Change date
	- [x] Change time
	- [x] Create 5s software timer
	- [x] Display date for 5 seconds
	- [x] Display temperature for 5 seconds
	- [ ] Deep Sleep mode
- [ ] Low-Power
	- [x] Sleep mode in Idle Task
	- [ ] Deep Sleep mode (turn off tubes, disable everything except for RTC) when power switch is in off position

