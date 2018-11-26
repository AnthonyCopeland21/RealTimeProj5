#include "startup.h"

// Purpose: 	start of program and intialization. Expect user input
// Input: 		none
// Output: 		PASS/FAIL returned.
void start(void) {
	//intiialization and setup functions
	System_Clock_Init();
	UART2_Init();
	output_setup();
	timer_startup();
	
	int ret = run();
}

// Purpose: 	setup of TIM2
// Input: 		none
// Output: 		none
void timer_startup(void) {
	// TIM2 should be set to UP mode, 20ms to reset
	// TIM3 should be set to UP mode, 100ms to reset
	// CCR values will drive the high and low of the PWM signal
	// CCR1 for servo 1 and CCR2 for servo 2
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN | RCC_APB1ENR1_TIM5EN; 	// ensure RCC->APB1ENR1 set to right timer
	TIM5->PSC = 79;													// 1MHz signal with prescalar
	TIM2->PSC = 79; 						    				// load prescaler into TIM2->PSC register. It will be																		
																					// 80MHz divided by prescaler plus 1

	TIM2->ARR    = 20000 - 1;									
	TIM5->ARR 	 = 200000 - 1;
	
	TIM2->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2;
	
	TIM2->EGR |= TIM_EGR_UG;                // create an update event using the TIM2->EGR register
	TIM5->EGR |= TIM_EGR_UG;
	TIM2->CCER  &= ~0xFFFF;          				// Disables the input enable by clearing register
	//TIM2->CCMR1 &= ~0xFFFF;								// Clear CCMR1 register
	TIM2->CR1   &= ~0xFFFF;
	TIM5->CR1		&= ~0xFFFF;
	TIM2->CCMR1 |= TIM_CCMR1_OC1PE;					// Enables preload register
	TIM2->CR1   |= TIM_CR1_ARPE;						// Auto reload preload register
	TIM5->CR1   |= TIM_CR1_ARPE;
	TIM2->CCER  |= TIM_CCER_CC1E;						// Enable output for channel 1
	TIM2->SR    &= ~TIM_SR_UIF;							// Clears update flag
	TIM5->SR		&= ~TIM_SR_UIF;
	//TIM2->DIER  |= TIM_DIER_UIE;					// Enables interrupt on update
	TIM2->CCMR1 |= 0x00000060;							// Set PWM mode 1 (see page 907)
	TIM2->CR1   |= 0x0010;									// Set timer in upcount mode
	TIM5->CR1   |= 0x0010;
	//CCR1 works off PA0, which should be hooked up to left servo
	TIM2->CCR1   = 0;								// 480 is start position 0, with 300 to go between the positions
 	
	TIM2->CCMR1 |= TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2;
	
	
	//CCR2 works off PA1, which should be hooked up to right servo
	TIM2->CCMR1 |= TIM_CCMR1_OC2PE;					// Enables preload register
	TIM2->CCER  |= TIM_CCER_CC2E;						// Enable output for channel 2
	TIM2->CCR2   = 0;								// 750 is start position 0, with 300 to go between positions
	TIM2->CR1   |= TIM_CR1_CEN;      				// Starts Counter for TIM3
	// Set the duty cycle by loading clock cycle count into TIM2_CCR1 register
	//TIM5->CR1 	|= TIM_CR1_CEN;							// Starts Counter for TIM5
}

// Purpose: 	Setup GPIOA for output
// Input: 		None
// Output: 		None
void output_setup(void) {
	// we need to setup 2 output GPIOs for the 2 signals to the 2 servos
	
	RCC->AHB2ENR   |=   RCC_AHB2ENR_GPIOAEN; // Enable GPIOA
	
	//GPIOA->MODER 	 &=   ~3; 				         // clear out bits 0 and 1 for PA0
	GPIOA->MODER 	 &=   ~0xF; 				         // clear out bits 0 and 1 for PA0
	GPIOA->MODER   &=   ~(0xFF << (2*0));    // Set desired values to tie PA0
	GPIOA->MODER   |=   0x2A << (2*0);       // as an alternate function tied to TIM2_CH1 and TIM2_CH2	
	GPIOA->AFR[0]  &=   ~(0x0000000F);
	GPIOA->AFR[0]  |=   0x00000011;
	GPIOA->PUPDR   |=   0x22 << (2*0);
	
	// Enable the clock to GPIO Ports A, B, and E	
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOEEN;

	// Set PB2 (red led) as output
	GPIOB->MODER &= ~(0x03<<(2*2)) ;
	GPIOB->MODER |= (1<<4);
		
	// Set PE8 (green led) as output
	GPIOE->MODER &= ~(0x03<<(2*8));		 
	GPIOE->MODER |= (1<<16);
		
	// Set PE8 and PB2 output type as push-pull
	GPIOE->OTYPER &= ~(0x100);
	GPIOB->OTYPER &= ~(0x4);
	
	// PE8 and PB2 output type as No pull-up no pull-down
	GPIOE->PUPDR &= ~(0x30000);
	GPIOB->PUPDR &= ~(0x30);
}

// there will be 2 interupts here
// 1st: left servo interupt, at a CCR
// 2nd: right servo interupt, at a diff CCR
// each interupt should check to see if the recipe has started or not
// this can be done by checking the status register

