/*
 * bsp_exit_button.c
 */

#include "bsp_exit_button.h"
#include "dio.h"
#include "event.h"
#include "stm32f1xx.h"

#define BSP_EXIT_BUTTON_GPIO_PORT_A  (1u)  /* matches dio.h's port numbering (1=GPIOA) */

static dio_t s_exitPin = { .port = BSP_EXIT_BUTTON_GPIO_PORT_A, .pin = 11U };

void bsp_exit_button_init(void)
{
    dio_mode(&s_exitPin, INPUT_PULLUP);

    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

    /* PA11 -> AFIO_EXTICR3_EXTI11_PA is 0, i.e. GPIOA is this field's
     * reset value - the AND still clears any stale selection first, in
     * case AFIO was left configured by something else. */
    AFIO->EXTICR[2] = (AFIO->EXTICR[2] & ~AFIO_EXTICR3_EXTI11_Msk) | AFIO_EXTICR3_EXTI11_PA;

    EXTI->RTSR &= ~EXTI_RTSR_TR11_Msk;
    EXTI->FTSR |= EXTI_FTSR_TR11_Msk;
    EXTI->PR = EXTI_PR_PR11_Msk;   /* clear any stale pending bit before unmasking */
    EXTI->IMR |= EXTI_IMR_MR11_Msk;

    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/* Shared by EXTI lines 10-15 on this part - only line 11 is ever
 * unmasked here, but check the pending bit anyway (same defensive habit
 * as the rest of this codebase's EXTI/IRQ handling). ISR-safe: only
 * sets a volatile flag via event_post() - see bsp_exit_button.h. */
void EXTI15_10_IRQHandler(void)
{
    if ((EXTI->PR & EXTI_PR_PR11_Msk) != 0U) {
        EXTI->PR = EXTI_PR_PR11_Msk;
        event_post(BSP_EXIT_BUTTON_EVENT_PRESSED);
    }
}
