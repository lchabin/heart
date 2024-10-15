# Red Beating Heart every Saint Valentin For the whole day. 
Proven 4 years autonomy with 2x2AA alcaline batteries.
On NUCLEO-L011K4

## Battery Autonomy
This design has been running non stop from March 17th 2018 to the end of 2022 on 4 AA alcaline batteries 
Configured to generate a pulse every hour (rather acceptable for the battery).
It has successfully generated heart beats every Saint Valentin of 2019,2020,2021,2022
The batteries were changed in Dec. 2022. 


Used this configuration in 2018:

$13h20m00s04w03m17d18y03600wkup

See below for explanations.


Used this configuration:

$17h14m00s06w12m31d22y03600wkup


## USING THE DESIGN WITH JUST THE NUCLEO BOARD : just to take a look, flash the board (program it) and make some tests

You can check the firmware and functionality before making the cicuit : 

Because it happens that PB3/D13 (GPIO of the nucleo board) also drives the green led of the nucleo board, you can test the functionality with the nucleo board as it is out of the factory. 
(no resistor, no transistor, no battery, with solder bridges SB14 and SB9 still closed, no big red led, no breadboard).


### Flashing (programming)
+ ensure no battery is connected
+ ensure SB14 and SB9 are closed
+ connect usb to a pc
+ flash using STM32CubeProg available from www.st.com 
(don't need the trusted package thing). 
+ You flash program the file MDK-ARM/heart/heart.hex
+ stop STM32CubeProg
### Settings and run
+ ensure no battery is connected
+ ensure SB14 is closed
+ start a terminal ('tera term' for instance) at 4800 bauds 
+ configure the terminal at 4800 bauds 
+ connect usb to a pc 
+ open connection to the new serial port in tera term
+ follow the instructions (displayed every 10 sec, with a led pulse)
Then it runs




## USING THE DESIGN ON BATTERY : the real deal
That's the only way to have it running for years.

### Prepare the NUCLEO board : REQUIRED ! 
See doc/open_sb14_sb9.png : open SB14 and SB9 solder bridges on the bottom on the nucleo board (voltage conflict, and reset).

From that moment, never connect anymore usb without the 3V battery holders connected to 3.3V first

Once SB9 is open, programming of the nucleo _may_ be a little more difficult (might fail more often), but i am not sure of that.  

### Flashing (programming) if not already done. RESPECT THAT SEQUENCE
+ ensure SB14 is open
+ connect battery to +3V3 pin and GND pin
+ connect usb to a pc
+ flash using STM32CubeProg available from www.st.com 
(don't need the trusted package thing).
+ You flash program the file heart.hex
+ stop STM32CubeProg
+ disconnect usb (clean fast disconnection)

### Settings and run. RESPECT THAT SEQUENCE
+ ensure SB14 and SB9 are open
+ start a terminal ('tera term' for instance) at 4800 bauds 
+ configure the terminal at 4800 bauds 
+ connect battery to +3V3 pin and GND pin
+ connect usb to a pc 
+ open connection to new serial port in tera term
+ follow instructions (displayed every 10 sec, with a led pulse)
+ disconnect usb (clean fast disconnection), voilà !

Once SB14 is open, Never connect usb without battery connected.

To restart the setting phase, you need to disconnect the battery then reconnect it (if a reset with the little push button on the side of the pcb is not enough).


## TRYING some tests

Examples (not in TESTING mode, nominal, heart.hex as on github)

### it is 1:20pm on thursday 15 march 2018

$13h20m00s04w03m15d18y00000wkup

+ will pulse on next saint valentin in 11 months, for 18 hours, then next year... 
+ one heart beat pulse every midnight
+ no wake up pulses

nota : monday is 01w

### it is 5:59pm on thursday 14 feb 2019

$17h59m00s04w02m14d19y00000wkup

+ will pulse on next saint valentin in one minute, for 18 hours, then next year.
+ one heart beat pulse every midnight
+ no wake up pulses

### it is 5:50pm on thursday 14 feb 2019

$17h50m00s04w02m14d19y03600wkup

+ will pulse on next saint valentin in ten minutes, for 18 hours, then next year.
+ one heart beat pulse every midnight
+ one wake up short pulse (150ms) every 3600 seconds  (just ok for the battery, help ensure it works)


## Touble shooting

if the red led flashes every 10 sec, that means that it is still waiting for a configuration to be given to it

using '03600wkup' in the config, helps check after one hour that it is really working 

## normal behaviour of the board

after a power on start up, 

+ the board does a 3 seconds fast pulse train
+ then it waits for the configuration information

Example of the text displayed:
```
You are in final target mode

Please write (or paste) the time and wake up delay. Don't use del,backspace because it is ignored
Example :
$21h24m00s05w03m09d18y00060wkup
Format : hour, minute, second, weekday, month, day of the month, year, wakeupEvery in seconds from 1 to 65535 (0 for no wake up). 01w == monday
$19h36m00s06w12m31d22y0360wkup
Setting time to $19h36m00s06w12m31d22y03600wkup
You can now disconnect usb.
```

+ just after configuration, it produces again a 3 seconds fast pulse train
+ there is then a 3 seconds delay, 
+ then, it produces again a 3 seconds fast pulse train
+ then it goes into deep sleep (standby) until the next wake up, midnight or saint valentin.

I would recommend that you disconnect USB during these 9 seconds of post configuration 