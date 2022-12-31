# Red Beating Heart every Saint Valentin
For the whole day. 
10 years autonomy goal with 2x2AA alcaline batteries.
On NUCLEO-L011K4

## Battery Autonomy
As of May 2022, this design has been running non stop from March 17th 2018 on 4 AA alcaline batteries to the end of 2022
Configured to generate a pulse every hour (rather acceptable for the battery).
It has successfully generated heart beats every Saint Valentin of 2019,2020,2021,2022

Used this configuration:
$13h20m00s04w03m17d18y03600wkup
See below for explanations.

Restarted in Dec. 2022 with new batteries, 
Used this configuration:
$17h14m00s06w12m31d22y03600wkup


## USING THE DESIGN WITH JUST THE NUCLEO BOARD : just to take a look and make some tests

You can check the firmware and functionality before making the cicuit : 

Because it happens that PB3/D13 (GPIO of the nucleo board) also drives the green led of the nucleo board, you can test the functionality with the nucleo board as it is out of the factory. 
(no resistor, no transistor, no battery, with SB14 still closed, no big red led, no breadboard).


### Flashing
*connect usb to a pc
*flash using STM32CubeProg available from www.st.com 
(don't need the trusted package thing). 
*You flash program the file heart.hex
*stop STM32CubeProg
### Settings and run
*start a terminal ('tera term' for instance) at 4800 bauds 
*configure the terminal at 4800 bauds 
*connect usb to a pc 
*open connection to new serial port in tera term
*follow instructions (displayed every 10 sec, with a led pulse)
Then it runs




## USING THE DESIGN ON BATTERY : the real deal
That's the only way to have it running for years.

### Prepare the NUCLEO board : REQUIRED ! 
See open_sb14.png : open SB14 solder bridge on the bottom on the nucleo board (v conflict).
From that moment, never connect anymore usb without the 3V battery holders connected to 3.3V

### Flashing
*connect battery to +3V pin and GND pin
*connect usb to a pc
*flash using STM32CubeProg available from www.st.com 
(don't need the trusted package thing).
*You flash program the file heart.hex
*stop STM32CubeProg
*disconnect usb

### Settings and run
*start a terminal ('tera term' for instance) at 4800 bauds 
*configure the terminal at 4800 bauds 
*connect battery to +3V pin and GND pin
*connect usb to a pc 
*open connection to new serial port in tera term
*follow instructions (displayed every 10 sec, with a led pulse)
*disconnect usb, voilà !
Never connect usb without battery connected
To restart the setting phase, you need to disconnect 
the battery then reconnect it.


## TRYING some tests

Examples (not in TESTING mode, nominal, heart.hex as on github)

### it is 1:20pm on thursday 15 march 2018

$13h20m00s04w03m15d18y00000wkup

will pulse on next saint valentin in 11 months... and short pulse every midnight

### it is 5:59pm on thursday 14 feb 2019

$05h59m00s04w02m14d19y00000wkup

will pulse on next saint valentin in one minute, for 18 hours, then next year.
+ short pulse every midnight
+ no wake up pulses

### it is 5:50pm on thursday 14 feb 2019

$05h50m00s04w02m14d19y00120wkup

will pulse on next saint valentin in ten minutes, for 18 hours, then next year.
+ short pulse every midnight
+ wake up pulse every 120 seconds  (very bad for battery)
