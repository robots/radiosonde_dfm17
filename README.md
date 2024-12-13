
# Alternative DFM17 radiosonde firmware

This firmware does not care about GPS and Radio. It implements measurement of temperature and humidity.


## What is implemented:
- radio is setup to output OSC/10 frequency
- MCU runs at 20.48MHz derived from 2.56MHz signal from radio, with fallback to 8MHz internal oscillator.
- I2C Eeprom calibration readout
- Time interval counter to measure the period of oscillator
- Temperature and humidity calculations

- Output to I2C 16x2 display using soft i2c library (routed to xdata pins)

## Where do you have info from?

DFM17 was found with unlocked MCU. The firmware has been dumped and analyzed.
Dont ask for dump, go get your own radiosonde.

Other sources of info are here:
- https://wiki.recessim.com/view/DFM-17_Radiosonde
- https://github.com/gx1400/kicad-dfm17-RE-pcb

Radio driver has been taken from (changed to fit)
- https://github.com/mikaelnousiainen/RS41ng


## How it works?

`TIM1` and `TIM15` are used as ring oscillator together with external analog circuits.

- `TIM1` is used to trigger charging of the capacitor.
- capacitor discharges
- Comparator will notice this and trigger `TIM15`
- after fixed time `TIM15` will trigger `TIM1`

The period of oscillation is controled by:
- selecting discharge element - reference 20k, reference 332k, temp1 and temp2
- selecting capacitor - reference capacitor or humidity sensor.

There are 5options that are supported (and the system is calibrated for):
- ref cap (`PC10 = 0`) and temp 1 (`PC11 = 1`)
- ref cap (`PC10 = 0`) and temp 2 (`PB5 = 1`)
- ref cap (`PC10 = 0`) and ref 20k (`PD2 = 1`)
- ref cap (`PC10 = 0`) and ref 332k (`PB4 = 1`)
- humidity sensor (PC10 = 1) and ref 332k (`PB4 = 1`)

The designers were very nice and calibrated the periods of the oscillator to microseconds. 20k reference will set the period to roughly 20us, 332k reference will set the period to roughly 332us. This makes is very easy to verify that partial results are correct.


`TIM2` and `TIM3` are used as 32bit timer. The switch from charging and discharging is used as "capture" input for these timers.

DMA (Channel 2 and 7) is setup to transfer the first measurement to RAM. Consequent measurements are not stored. The last measurement is stored directly in the `TIMx->CCRy` register.

DMA Channel 5 is setup to transfer `0xffff` units from timer2. The transfered value is not needed, it is used to count how many periods were observed.

The sampling period is about 150ms. 

Number of period seen is then `0xffff - DMA_Channel5->CNTDR - 1`. (Original dma value minus count left to transfer minus 1 because first value is used)
Last captured value (interpreted as 32bit) minus First sample gives time it took to sample these periods.

Divide this delta with number of periods and divide by timer frequency equals average period in seconds. (we divide by frequency in khz which leads to time in miliseconds)


## Temperature calculation
The 3 periods (ref 20k, ref 332k and temp) are used to calculate resistance of the temperature sensor.

There are 2 other values coming into this calculation and those are coefficients from eeprom (`coef_6` and `coef_7`). These are calibration of the 20k and 332k resistors respectively.

After the interpolation is done, logarithm and 6 point calibration from eeprom are applied

There are 2 sensors temp 1 and temp 2, and according to coefficient tables both seem very different.


## Humidity calculation


Humidity is directly calculated from period using 5point calibration table from eeprom.

Afterward temperature compensation is applied. (Temperature sensor 2 is used - the one close to humidity sensor)


# Error in documentation:
- PA1, PB0 are used as CC inputs to TIM2 and TIM2
- PA11 is `TIM_CH4` output
- PB15 is used as TIM15 output, it is also used as EXTI15.
- PB14 is connected to PB15
- PB9 is used as 1.28MHz output - connected to testpoint (to check frequency, also in original firmware)

# EEPROM

Eeprom stores configuration of the serial ports, possibly serial number of radiosonde, calibrations and measurement sequence.

Measurement sequence is stored at address `0xa0`, length of 16. The sequence looks something like `E`, `D`, `O`, `0x06`, `0x07`.

Where IDs `E` and  `O` are temperature and humidity temperature respectively, `D` is humidity, `0x06` and `0x07` are measurements of 20k and 332k resistors.

On address `0xb0` length 8 are number of coefficients stored in eeprom per sequence ID. Length is stored as 4 bits. First offset is higher 4 bits of first byte, second offset is lower 4 bits of first byte, ...

Calibration coefficients are stored from address `0xb8`. These are stored as 8 byte floating point (double) in little endian format.

Address `0x90` contains bitmask how analog switches should be set when measuring. Not used in code

Address `0x88` contains serial number. Not used in code

## Example content
0x88: 0xDB 0xE1 0x6E 0x01  = 24043995 - serial number is 24179-043995
0xa0: E D O 0x06 0x07 0x46 ...
0xb0: 0x75 0x61 0x10 0x00 ....
0xb8: 0x75 0x05 0xEE 0x1D 0xFC 0x80 0x52 0xC0 ...

Measurement sequence is external temp, humidity, humidity temp, ref 6, ref 7, some other measurement...

ID `E` has 7 coeficients, ID `D` 5 coefficients, ID `O` 6 coefficients, ID `0x06` 1 coeficient, ID `0x07` 1 coeficient.


