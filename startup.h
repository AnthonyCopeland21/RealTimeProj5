#ifndef STARTUP_H
#define STARTUP_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "run.h"
#include "stm32l476xx.h"
#include "SysClock.h"
#include "UART.h"

// define constants PASS and FAIL for returning from functions
#define PASS (1)
#define FAIL (0)

static int recipe_status = 0x0;

void start(void);
void timer_startup(void);
void output_setup(void);

#endif  // STARTUP_H
