#include "dpt.h"
#include "stm32g4xx_hal.h"

	#define P1_TICKS 		(100 * 170)  	/* pulse 1 in ticks @ 170MHz */
	#define GAP_US 			(50) 			/* gap in us */
	#define P2_TICKS 		(25 * 170)  	/* pulse 2 in ticks @ 170MHz */

// Parametri del test — definiti una volta sola qui
uint32_t p1_ticks = P1_TICKS;
uint32_t p2_ticks = P2_TICKS;
uint32_t gap_us = GAP_US;


// Inizializza il Data Watchpoint and Trace (DWT)
void DWT_Init(void) {
	// 1. Abilita il modulo Debug Exception and Monitor Control (DEMCR)
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

	// 2. Resetta il contatore di cicli
	DWT->CYCCNT = 0;

	// 3. Abilita il contatore di cicli nel registro di controllo DWT
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void PWMN_ForceInactive(TIM_HandleTypeDef *htim, uint32_t channel) {
    // Forza output a livello inattivo (basso se OCNPolarity = high)
    // Scrive OC1M = 100 (Force inactive) nel CCMR1
    switch (channel) {
        case TIM_CHANNEL_1:
            MODIFY_REG(htim->Instance->CCMR1,
                       TIM_CCMR1_OC1M,
                       TIM_OCMODE_FORCED_INACTIVE);
            break;
        case TIM_CHANNEL_2:
            MODIFY_REG(htim->Instance->CCMR1,
                       TIM_CCMR1_OC2M,
                       (TIM_OCMODE_FORCED_INACTIVE << 8U));
            break;
        case TIM_CHANNEL_3:
            MODIFY_REG(htim->Instance->CCMR2,
                       TIM_CCMR2_OC3M,
                       TIM_OCMODE_FORCED_INACTIVE);
            break;
        case TIM_CHANNEL_4:
            MODIFY_REG(htim->Instance->CCMR2,
                       TIM_CCMR2_OC4M,
                       (TIM_OCMODE_FORCED_INACTIVE << 8U));
            break;
    }
}

void PWMN_RestorePWMMode(TIM_HandleTypeDef *htim, uint32_t channel) {
    // Ripristina PWM Mode 1 prima del prossimo impulso
    switch (channel) {
        case TIM_CHANNEL_1:
            MODIFY_REG(htim->Instance->CCMR1,
                       TIM_CCMR1_OC1M,
                       TIM_OCMODE_PWM1);
            break;
        case TIM_CHANNEL_2:
            MODIFY_REG(htim->Instance->CCMR1,
                       TIM_CCMR1_OC2M,
                       (TIM_OCMODE_PWM1 << 8U));
            break;
        case TIM_CHANNEL_3:
            MODIFY_REG(htim->Instance->CCMR2,
                       TIM_CCMR2_OC3M,
                       TIM_OCMODE_PWM1);
            break;
        case TIM_CHANNEL_4:
            MODIFY_REG(htim->Instance->CCMR2,
                       TIM_CCMR2_OC4M,
                       (TIM_OCMODE_PWM1 << 8U));
            break;
    }
}

void delay_us(uint32_t us){
	uint32_t startTick = DWT->CYCCNT;
	uint32_t delayTicks = us * (SystemCoreClock / 1000000);

	while ((DWT->CYCCNT - startTick) < delayTicks);
}

void Run_DoublePulseTest(void) {

	    // --- PULSE 1 ---
	    PWMN_ForceInactive(&htim1, TIM_CHANNEL_1);
	    __HAL_TIM_SET_COUNTER(&htim1, 0);
	    __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE);

	    __HAL_TIM_SET_AUTORELOAD(&htim1, p1_ticks - 1);
	    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, p1_ticks);

	    PWMN_RestorePWMMode(&htim1, TIM_CHANNEL_1);
	    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);

	    while (__HAL_TIM_GET_FLAG(&htim1, TIM_FLAG_UPDATE) == RESET);
	    __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE);

	    PWMN_ForceInactive(&htim1, TIM_CHANNEL_1);  // Pin basso, bassa impedenza

	    // --- GAP ---
	    delay_us(gap_us);

	    // --- PULSE 2 ---
	    __HAL_TIM_SET_COUNTER(&htim1, 0);
	    __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE);

	    __HAL_TIM_SET_AUTORELOAD(&htim1, p2_ticks - 1);
	    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, p2_ticks);

	    PWMN_RestorePWMMode(&htim1, TIM_CHANNEL_1);
	    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);

	    while (__HAL_TIM_GET_FLAG(&htim1, TIM_FLAG_UPDATE) == RESET);
	    __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE);

	    // --- DONE ---
	    PWMN_ForceInactive(&htim1, TIM_CHANNEL_1);
	}
