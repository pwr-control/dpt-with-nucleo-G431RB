
// dpt.h
#ifndef DPT_H
#define DPT_H

#include "stm32g4xx_hal.h"
#include "tim.h"

	#define CLOCK_PWM_MHZ   170  					/* PWM module clock input */
	#define P1_TICKS 		(80 * CLOCK_PWM_MHZ)  	/* pulse time 1 in us per clock frequency = pulse 1 in ticks @ 170MHz */
	#define GAP_US 			(10) 					/* gap in us */
	#define P2_TICKS 		(10 * CLOCK_PWM_MHZ)  	/* pulse time 2 in us per clock frequency = pulse 2 in ticks @ 170MHz */

// Parametri del test — definiti una volta sola qui
extern uint32_t p1_ticks;
extern uint32_t p2_ticks;
extern uint32_t gap_us;
//extern TIM_HandleTypeDef htim1;

// Prototipi
void DWT_Init(void);
void delay_us(uint32_t us);
void PWMN_ForceInactive(TIM_HandleTypeDef *htim, uint32_t channel);
void PWMN_RestorePWMMode(TIM_HandleTypeDef *htim, uint32_t channel);
void Run_DoublePulseTest(void);

#endif // DPT_H




