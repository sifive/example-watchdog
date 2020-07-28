/* Copyright 2019 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#include <metal/cpu.h>
#include <metal/interrupt.h>
#include <metal/watchdog.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define WDOG_DESIRED_RATE 1000000 /* 1 MHz */
#define WDOG_TIMEOUT_SECONDS 1

bool caught_wdog_int = false;

void metal_watchdog_handler(void) {
	struct metal_watchdog wdog = metal_watchdog_get_device(0);

	/* Stop the watchdog and clear the interrupt*/
	metal_watchdog_run(wdog, METAL_WATCHDOG_STOP);
	metal_watchdog_clear_interrupt(wdog);

	puts("Caught watchdog interrupt\n");

	caught_wdog_int = true;	
}

int main() {
	struct metal_watchdog wdog = metal_watchdog_get_device(0);

	/* Try to set the watchdog rate */
	const long int rate = metal_watchdog_set_rate(wdog, WDOG_DESIRED_RATE);

	/* Timeout after WDOG_TIMEOUT_SECONDS seconds */
	metal_watchdog_set_timeout(wdog, WDOG_TIMEOUT_SECONDS * rate);

	/* Configure the watchdog to trigger an interrupt */
	metal_watchdog_set_result(wdog, METAL_WATCHDOG_INTERRUPT);

	/* Enable interrupts */
	metal_watchdog_enable_interrupt(wdog);
	metal_cpu_enable_interrupts();

	puts("Starting watchdog\n");

	/* Start the watchdog */
	metal_watchdog_run(wdog, METAL_WATCHDOG_RUN_ALWAYS);

	/* If the watchdog doesn't fire after twice the requested timeout, fail */
	time_t timeout = time(NULL) + (2 * WDOG_TIMEOUT_SECONDS);
	
	while (!caught_wdog_int) {
		if (time(NULL) > timeout) {
			/* Stop the watchdog */
			metal_watchdog_run(wdog, METAL_WATCHDOG_STOP);

			puts("Watchdog interrupt never triggered\n");

			exit(7);
		}
	}

	return 0;
}

