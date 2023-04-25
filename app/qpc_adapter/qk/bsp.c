/*****************************************************************************
* Product: "Blinky" example, EK-TM4C123GXL board, preemptive QK kernel
* Last updated for version 7.2.0
* Last updated on  2022-12-17
*
*                    Q u a n t u m  L e a P s
*                    ------------------------
*                    Modern Embedded Software
*
* Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
*
* This program is open source software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published
* by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Alternatively, this program may be distributed and modified under the
* terms of Quantum Leaps commercial licenses, which expressly supersede
* the GNU General Public License and are specifically designed for
* licensees interested in retaining the proprietary status of their code.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <www.gnu.org/licenses/>.
*
* Contact information:
* <www.state-machine.com/licensing>
* <info@state-machine.com>
*****************************************************************************/
#include "qpc.h"
#include "bsp.h"

#include "stm32f072xb.h"
#include "main.h"
#include "stm32f0xx_hal.h"
/* add other drivers if necessary... */

Q_DEFINE_THIS_FILE

#ifdef Q_SPY
    #error Simple Blinky Application does not provide Spy build configuration
#endif
extern TIM_HandleTypeDef htim17;
extern TIM_HandleTypeDef htim14;
extern CAN_HandleTypeDef hcan;
static volatile uint32_t tVar;
/* ISRs defined in this BSP ------------------------------------------------*/
void SysTick_Handler(void);
void TIM17_IRQHandler(void);
void TIM14_IRQHandler(void);
void CEC_CAN_IRQHandler(void);
/* Local-scope objects -----------------------------------------------------*/

/* ISRs used in this project ===============================================*/
void SysTick_Handler(void) {
    QK_ISR_ENTRY();   /* inform QK about entering an ISR */
    QTIMEEVT_TICK_X(0U, (void *)0); /* process time events for rate 0 */
    QK_ISR_EXIT();  /* inform QK about exiting an ISR */
}
void TIM17_IRQHandler(void)
{
    QK_ISR_ENTRY();   /* inform QK about entering an ISR */
    HAL_TIM_IRQHandler(&htim17);
    tVar++;
    QK_ISR_EXIT();  /* inform QK about exiting an ISR */
}
void TIM14_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim14);
}
void CEC_CAN_IRQHandler(void)
{
    HAL_CAN_IRQHandler(&hcan);
}
/*..........................................................................*/
void BSP_init(void) {

    /* NOTE: SystemInit() already called from the startup code
    *  but SystemCoreClock needs to be updated
    */
    SystemCoreClockUpdate();
    HAL_SetTickFreq(HAL_TICK_FREQ_1KHZ);
}
/*..........................................................................*/
void BSP_ledOn(void) {
    float volatile x = 3.1415926F;
    x = x + 2.7182818F;
    HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_SET);
}
/*..........................................................................*/
void BSP_ledOff(void) {
   /* exercise the FPU with some floating point computations */
    float volatile x = 3.1415926F;
    x = x + 2.7182818F;
    HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_RESET);
}

/* QF callbacks ============================================================*/
void QF_onStartup(void) {
    /* set up the SysTick timer to fire at BSP_TICKS_PER_SEC rate */
    SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC);

    /* assing all priority bits for preemption-prio. and none to sub-prio. */
    NVIC_SetPriorityGrouping(0U);

    /* set priorities of ALL ISRs used in the system, see NOTE1
    *
    * !!!!!!!!!!!!!!!!!!!!!!!!!!!! CAUTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    * Assign a priority to EVERY ISR explicitly by calling NVIC_SetPriority().
    * DO NOT LEAVE THE ISR PRIORITIES AT THE DEFAULT VALUE!
    */
    NVIC_SetPriority(SysTick_IRQn,   QF_AWARE_ISR_CMSIS_PRI);
    NVIC_SetPriority(TIM17_IRQn, QF_AWARE_ISR_CMSIS_PRI + 1);
    /* ... */

    /* enable IRQs... */

    /* Initialize the hardware intetrupts here */
    NVIC_EnableIRQ(TIM17_IRQn);
}
/*..........................................................................*/
void QF_onCleanup(void) {
}
/*..........................................................................*/
#ifdef QF_ON_CONTEXT_SW
/* NOTE: the context-switch callback is called with interrupts DISABLED */
void QF_onContextSw(QActive *prev, QActive *next) {
}
#endif /* QF_ON_CONTEXT_SW */
/*..........................................................................*/
void QK_onIdle(void) {
    /* toggle the User LED on and then off, see NOTE2 */
    QF_INT_DISABLE();
    QF_INT_ENABLE();

#ifdef NDEBUG
    /* Put the CPU and peripherals to the low-power mode.
    * you might need to customize the clock management for your application,
    * see the datasheet for your particular Cortex-M MCU.
    */
    __WFI(); /* Wait-For-Interrupt */
#endif
}

/*..........................................................................*/
Q_NORETURN Q_onAssert(char const * const module, int_t const loc) {
    /*
    * NOTE: add here your application-specific error handling
    */
    (void)module;
    (void)loc;
    QS_ASSERTION(module, loc, 10000U); /* report assertion to QS */
    NVIC_SystemReset();
}

/*============================================================================
* NOTE1:
* The QF_AWARE_ISR_CMSIS_PRI constant from the QF port specifies the highest
* ISR priority that is disabled by the QF framework. The value is suitable
* for the NVIC_SetPriority() CMSIS function.
*
* Only ISRs prioritized at or below the QF_AWARE_ISR_CMSIS_PRI level (i.e.,
* with the numerical values of priorities equal or higher than
* QF_AWARE_ISR_CMSIS_PRI) are allowed to call the QXK_ISR_ENTRY/QXK_ISR_EXIT
* macros or any other QF/QXK  services. These ISRs are "QF-aware".
*
* Conversely, any ISRs prioritized above the QF_AWARE_ISR_CMSIS_PRI priority
* level (i.e., with the numerical values of priorities less than
* QF_AWARE_ISR_CMSIS_PRI) are never disabled and are not aware of the kernel.
* Such "QF-unaware" ISRs cannot call any QF/QXK services. In particular they
* can NOT call the macros QXK_ISR_ENTRY/QXK_ISR_ENTRY. The only mechanism
* by which a "QF-unaware" ISR can communicate with the QF framework is by
* triggering a "QF-aware" ISR, which can post/publish events.
*
* NOTE2:
* The User LED is used to visualize the idle loop activity. The brightness
* of the LED is proportional to the frequency of invcations of the idle loop.
* Please note that the LED is toggled with interrupts locked, so no interrupt
* execution time contributes to the brightness of the User LED.
*/
