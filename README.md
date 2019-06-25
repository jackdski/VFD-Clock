This project aims to create a unique display for time, date, and temperature data on 
    seven-segment IV-11 Vacuum Fluorescent Display (VFD) tubes. To accomplish this, an STM32F0 
    microcontroller, shift-registers, logic-level shifters, boost- and buck-converters, a digital 
    temperature sensor, capacative-touch, and a photoresistor are used. 
    
To accomplish this, several goals were outlined:
- Use FreeRTOS to gain experience with RTOS's
- Use Bluetooth Low-Energy to wirelessly communicate instructions and auto-sync time and
    date data from an iPhone
    - Manually select the time
    - Turn off auto-brightness adjustments
    - Select whether to display temperature or the date temporarily
    - Turn the VFD display on or off and put into low-power mode
- Adapt brightness on displays using PWM and sampling ambient brightness using 
            ADC and two photoresistors, located on opposite corners of the PCB
    - Use a timer to avoid dimming or brightening the display due to a passing event
- Retain RTC operation using a coin-cell battery without wall power for minimum 1 year
- Use I2C to collect temperature data from a digital temperature sensor and display it 
            on the tubes
- Use capacative touch on the corners of the PCB to display the ambient temperature or 
            date while the capacative "button" is touched