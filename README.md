# heart
Red Beating Heart every Saint Valentin. 
For the whole day. 
10 years autonomy goal.

Work in progress march 2018

================================
USING IT
======================================
On the nucleo, REQUIRED ! 
open SB14 solder bridge (v conflict).
Never connect usb without battery connected
======================================
Flashing
*connect battery to +3V pin and GND pin
*connect usb to a pc
*flash using STM32CubeProg available from www.st.com 
(don't need the trusted package thing). program the file heart.hex
*stop STM32CubeProg
*disconnect usb
======================================
Settings and run
*start a terminal ('tera term' for instance) at 4800 bauds 
*configure the terminal at 4800 bauds 
*connect battery to +3V pin and GND pin
*connect usb to a pc (you have 1000 sec to react)
*open connection to new serial port in tera term
*follow instructions (displayed every 10 sec)
*disconnect usb, voilà !
Never connect usb without battery connected
To restart the setting phase, you need to disconnect 
the battery then reconnect it.
========================================

Bill of material 

USE ONLY ALCALINE 1.5V batteries, 
DO NOT USE ACCUMULATORS NiMH (1.2V), we need 1.5V during the programming phase else too large a voltage difference between 3.3V from USB and 2*Vbat

Farnell farnell.fr , farnell.com, www.newark.com references

03.530€ 1x 2146419 DLA/6SRD - LED, Basse puissance, Rouge, Traversant, 20mm, 20 mA, 1.85 V, 640 nm
03.230€ 1x 2395961 MCBB400 -  Carte prototypage, sans soudure, ABS (Acrylonitrile Butadiène Styrène), 8.3mm, 54.5mm x 83.5mm
01.200€ 1x 1702631 2463    -  Support batterie, AA x 2, à fils
11.000€ 1x 2535699 NUCLEO-L011K4 -  Carte de développement, STM32L011K4 MCU, Débogueur sur la carte, Compatible Arduino Nano
05.070€ 1x 817363  pack of four AA Alcaline Batteries 
00.148€ 1x 2453790 BC547BTF -  Transistor simple bipolaire (BJT), NPN, 45 V, 300 MHz, 500 mW, 100 mA, 200 hFE (per 5)
00.053€ 1x 2329911 TE CONNECTIVITY - Résistance traversante, 2 kohm, Série LR, 200 V, Couche métallique, Axial, 250 mW (per 10)
00.053€ 1x 2329951 TE CONNECTIVITY - Résistance traversante, 51 ohm, Série LR, 200 V, Couche métallique, Axial, 250 mW (per 10)

Some wire like 

One micro USB cable

6x 50 ohms resistors to limit current on each led
1x 2K ohms resistor from cpu to base of transistor


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




TRYING some tests
========================================
Example in TESTING mode

$23h59m00s06w03m09d18y00120wkup

dayly pulse à 0h00 after dt=1mn
wakeup short pulse after dt=2mn at 0h01
st valentin pulse 120 secs at 0h02 after dt=3mn for 2mn, then a wakeup pulse
then a wakeup pulse at 0h06 after dt=7mn

======================================
Example out of TESTING mode (heart.hex compile without the define)

it is 1:20pm on thursday 15 march 2018

$13h20m00s04w03m15d18y00000wkup

will pulse on next saint valentin in 11 months... and short pulse every midnight

=====================================
Example out of TESTING mode (nominal)

it is 5:59pm on thursday 14 feb 2019

$05h59m00s04w02m14d19y00000wkup

will pulse on next saint valentin in one minute, for 16 hours, then next year.
+ short pulse every midnight
=====================================

$17h21m00s06w03m10d18y00000wkup