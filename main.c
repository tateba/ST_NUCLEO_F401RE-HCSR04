/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

BaseSequentialStream* chp = (BaseSequentialStream*) &SD2;

icucnt_t echo;

/*
 * Define the Trigger PORT and PIN
 */
#define TRIGGER_PORT	GPIOC
#define TRIGGER_PIN	GPIOC_PIN2

#define ICU_TIM_FREQ	1000000

static void echocb(ICUDriver *icup){
	echo = icuGetWidthX(icup);
}

static ICUConfig icucfg = {
	ICU_INPUT_ACTIVE_HIGH,
	ICU_TIM_FREQ,
	echocb,
	NULL,
	NULL,
	ICU_CHANNEL_1,
	0
};

/*
 * LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg){
	(void)arg;
	chRegSetThreadName("blinker");
	systime_t time = chVTGetSystemTimeX();
	while(true){
		time +=MS2ST(1000); // Next deadline
		palTogglePad(GPIOA, GPIOA_LED_GREEN);
		chThdSleepUntil(time);
	}
}

/*
 * HC-SR04 Trigger thread
 */
static THD_WORKING_AREA(waThread2, 128);
static THD_FUNCTION(Thread2, arg){
	(void)arg;
  
	chRegSetThreadName("trigger-thread");
	systime_t time = chVTGetSystemTimeX();
	while(true){
		time += MS2ST(500);
		// To send the trigger:
		palSetPad(TRIGGER_PORT, TRIGGER_PIN);
		chThdSleepMicroseconds(10);
		palClearPad(TRIGGER_PORT, TRIGGER_PIN);

		chThdSleepUntil(time);
  }
}

/*
 * HC-SR04 Echo thread
 *
 */
static THD_WORKING_AREA(waThread3, 128);
static THD_FUNCTION(thread3, arg){
	(void)arg;
	chRegSetThreadName("echo-thread");
	systime_t time = chVTGetSystemTimeX();
	while(1){
		time += MS2ST(500);
		chprintf(chp, "\n\rHC-SR04: Distance = %d cm", echo/58);
		chThdSleepUntil(time);
	}
}

/*
 * Application entry point.
 */
int main(void) {

	/*
	* System initializations.
	* - HAL initialization, this also initializes the configured device drivers
	*   and performs the board-specific initializations.
	* - Kernel initialization, the main() function becomes a thread and the
	*   RTOS is active.
	*/
	halInit();
	chSysInit();

	/*
	* Set the trigger pin to output and it level to LOW
	*/
	palSetPadMode(TRIGGER_PORT, TRIGGER_PIN, PAL_MODE_OUTPUT_PUSHPULL);
	palClearPad(TRIGGER_PORT, TRIGGER_PIN);

	/*
	* Initialize ICU driver 1, and GPIOA_PIN8 is the input
	*/
	palSetPadMode(GPIOA, GPIOA_PIN8, PAL_MODE_ALTERNATE(1));
	icuStart(&ICUD1, &icucfg);
	icuStartCapture(&ICUD1);
	icuEnableNotifications(&ICUD1);

	/*
	* Activates the serial driver 2 using the driver default configuration.
	*/
	sdStart(&SD2, NULL);

	chprintf(chp, "\n\n\r ChibiOS 3.0.2 project by NAFT.\n\r");

	/*
	* Print a test mesage
	*/
	chprintf(chp, "\n\r test message to print after boot complete");
	chprintf(chp, "\n\r my best number is: %d", 17);
  
	/*
	* Creates the blinker thread.
	*/
	chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

	/*
	* Creates the trigger thread.
	*/
	chThdCreateStatic(waThread2, sizeof(waThread2), HIGHPRIO, Thread2, NULL);

	/*
	* Create the Echo thread.
	*/
	chThdCreateStatic(waThread3, sizeof(waThread3), HIGHPRIO, thread3, NULL);

	/*
	* Normal main thread activity.
	*/
	while (true) {
		chThdSleepMilliseconds(1000);
	}
}
