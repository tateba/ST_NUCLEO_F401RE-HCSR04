*****************************************************************************
** ChibiOS/RT port for ARM-Cortex-M4 STM32F401.                            **
*****************************************************************************

** TARGET **

The burst program runs on an ST_NUCLEO_F401RE board.

** The Program **

The demo flashes the board LED using a thread, it represent a heart beat of
the program.
The program also trigger and read the distance between HC-SR04 and some object.

** Notes **

Some files used by the demo are not part of ChibiOS/RT but are copyright of
ST Microelectronics and are licensed under a different license.
Also note that not all the files present in the ST library are distributed
with ChibiOS/RT, you can find the whole library on the ST web site:

                             http://www.st.com
