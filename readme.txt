======================================

Red Beating Heart every Saint Valentin. 
For the whole day. 
10 years autonomy goal with 2x2AA alcaline batteries.
On NUCLEO-L011K4

======================================

Bill of material 

USE ONLY ALCALINE 1.5V batteries, 
Get 4 batteries, new from the same pack. This is to ensure the same voltages because we are going to put them in parallel.

DO NOT USE ACCUMULATORS NiMH (1.2V), we need 1.5V during the programming phase else too large a voltage difference between 3.3V from USB and 2*Vbat

Farnell farnell.fr , farnell.com, www.newark.com references

03.530€ 1x 2146419 DLA/6SRD - LED, Basse puissance, Rouge, Traversant, 20mm, 20 mA, 1.85 V, 640 nm
03.230€ 1x 2395961 MCBB400 -  Carte prototypage, sans soudure, ABS (Acrylonitrile Butadiène Styrène), 8.3mm, 54.5mm x 83.5mm
01.200€ 2x 1702631 2463    -  Support batterie, AA x 2, à fils
11.000€ 1x 2535699 NUCLEO-L011K4 -  Carte de développement, STM32L011K4 MCU, Débogueur sur la carte, Compatible Arduino Nano
05.070€ 1x 817363  pack of four AA Alcaline Batteries 
00.148€ 1x 2453790 BC547BTF -  Transistor simple bipolaire (BJT), NPN, 45 V, 300 MHz, 500 mW, 100 mA, 200 hFE (per 5)
00.053€ 1x 2329911 TE CONNECTIVITY - Résistance traversante, 2 kohm, Série LR, 200 V, Couche métallique, Axial, 250 mW (per 10)
00.053€ 1x 2329951 TE CONNECTIVITY - Résistance traversante, 51 ohm, Série LR, 200 V, Couche métallique, Axial, 250 mW (per 10)

Some wire, for example if you have nothing (it is a rigid core, single conductor)
14.610€ 1 x 2290875 - 3050/1 WH005 -  Câble, Rigide, Hook Up, PVC (Chlorure de polyvinyle), Blanc, 24 AWG, 0.205 mm², 100 ft, 30.5 m
or much thinner and flexible, harder to strip and insert cause small:
14.500€ 1 x 1183994 - SPC00440A002 25M -  Câble, PTFE (Polytétrafluoroéthylène), Noir, 26 AWG, 0.128 mm², 82 ft, 25 m

One micro USB cable

6x 50 ohms resistors will limit current on each led to about 20mA
1x 2K ohms resistor from cpu PB3/D13 pin to base of transistor

an iron (just to open the SB14 solder bridge on the nucleo board)

======================================
Justification for the values of the resistors

I leds ? we want 0.02A current in each of the 6 leds total 0.120A, at ~1.85V.

Checking that the transistor will be able to drive the leds in good saturation
We have in datasheet NPN : hFE = 200.
Ibase = (3-0.6)/2000 = 0.0012A 
Icollector = 1.2mA*hFE = 0.240A. OK > 0.120A
OK


What resistor to put in series with the leds for that ?
BC547BTF Saturation voltage at 0.12A ~= 0.15v to 0.2v
which leads to a resistance for each led of R = U/I = (3-0.15-1.85)/0.02 = 50 ohms.

I think it would not be recommended to put all the leds in parallel with a single 8.3333 ohm resistor because of the variations in production, but it should rather work. Would still be below 1/4w
But that would make less wires.

======================================
Checking before making the cicuit

Because it happens that PB3/D13 also drives the green led of the nucleo board, you can test the functionality with the nucleo board as it is out of the factory. 
(no resistor, no transistor, no battery, with SB14 still closed, no big led, no breadboard).



USING IT WITH JUST THE NUCLEO BOARD, just to take a look and make some tests
======================================
Flashing
*connect usb to a pc
*flash using STM32CubeProg available from www.st.com 
(don't need the trusted package thing). 
*You flash program the file heart.hex
*stop STM32CubeProg
======================================
Settings and run
*start a terminal ('tera term' for instance) at 4800 bauds 
*configure the terminal at 4800 bauds 
*connect usb to a pc 
*open connection to new serial port in tera term
*follow instructions (displayed every 10 sec, with a led pulse)
Then it runs
========================================



To build the circuit with the breadboard, the components and wires:
See schematic.png
See nucleo.png
Ask if you have no idea how to do that.


USING IT ON BATTERY, the real deal
That's the only way to have it running for years.
======================================
On the nucleo, REQUIRED ! 
See open_sb14.png : open SB14 solder bridge on the bottom on the nucleo board (v conflict).
From that moment, never connect anymore usb without the 3V battery holders connected to 3.3V
======================================
Flashing
*connect battery to +3V pin and GND pin
*connect usb to a pc
*flash using STM32CubeProg available from www.st.com 
(don't need the trusted package thing).
*You flash program the file heart.hex
*stop STM32CubeProg
*disconnect usb
======================================
Settings and run
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
========================================


TRYING some tests
======================================
Example out of TESTING mode (heart.hex compile without the define TESTING)

it is 1:20pm on thursday 15 march 2018

$13h20m00s04w03m15d18y00000wkup

will pulse on next saint valentin in 11 months... and short pulse every midnight

=====================================
Example out of TESTING mode (nominal)

it is 5:59pm on thursday 14 feb 2019

$05h59m00s04w02m14d19y00000wkup

will pulse on next saint valentin in one minute, for 18 hours, then next year.
+ short pulse every midnight
+ no wake up pulses
=====================================
Example out of TESTING mode (nominal)

it is 5:59pm on thursday 14 feb 2019

$05h50m00s04w02m14d19y00120wkup

will pulse on next saint valentin in ten minute, for 18 hours, then next year.
+ short pulse every midnight
+ wake up pulse every 120 seconds
========================================

$20h35m00s06w03m11d18y00000wkup
