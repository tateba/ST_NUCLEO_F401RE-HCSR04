/**
 * @file    main.c
 *
 * @brief   Distance measurement with an HCSR04 and a Nucléo F401RE board
 *          running ChibiOS.
 *
 * @author  Theodore Ateba
 *
 * @date    24 June 2016
 *
 * @version 1.0
 *
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

/*
 * @brief Input Capture Unit Configuration
 */
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
    palSetPad(TRIGGER_PORT, TRIGGER_PIN);
    chThdSleepMicroseconds(10);
    palClearPad(TRIGGER_PORT, TRIGGER_PIN);
    chThdSleepUntil(time);
  }
}

/*
 * HC-SR04 Echo thread
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
  
  palSetPadMode(TRIGGER_PORT, TRIGGER_PIN, PAL_MODE_OUTPUT_PUSHPULL);
  palClearPad(TRIGGER_PORT, TRIGGER_PIN);
  palSetPadMode(GPIOA, GPIOA_PIN8, PAL_MODE_ALTERNATE(1));

  icuStart(&ICUD1, &icucfg);
  icuStartCapture(&ICUD1);
  icuEnableNotifications(&ICUD1);

  sdStart(&SD2, NULL);
  
  chprintf(chp, "\n\n\r ChibiOS 3.0.2 project by NAFT.\n\r");
  chprintf(chp, "\n\r test message to print after boot complete");
  chprintf(chp, "\n\r my best number is: %d", 17);
  
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
  chThdCreateStatic(waThread2, sizeof(waThread2), HIGHPRIO, Thread2, NULL);
  chThdCreateStatic(waThread3, sizeof(waThread3), HIGHPRIO, thread3, NULL);
  
	while (true) {
		chThdSleepMilliseconds(1000);
	}
}
