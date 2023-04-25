#include "qpc.h"    /* QP/C framework API */
#include "bsp.h"    /* Board Support Package interface */

#include "ao_canopennodestm32.h"
#include "app_signals.h"
#include "CO_app_STM32.h"
#include "can.h"
#include "tim.h"
#include "gpio.h"

/* ask QM to declare the Blinky class --------------------------------------*/
/*$declare${AOs::Blinky} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
extern TIM_HandleTypeDef htim14;
extern CAN_HandleTypeDef hcan;
/*${AOs::Blinky} ...........................................................*/
typedef struct {
/* protected: */
    QActive super;

/* private: */
    QTimeEvt timeEvt;
    CANopenNodeSTM32 canOpenNodeSTM32;
} CANopenNodeSTM32_ao;

/* protected: */
static QState CANopenNodeSTM32_initial(CANopenNodeSTM32_ao * const me, void const * const par);
static QState CANopenNodeSTM32_operational(CANopenNodeSTM32_ao * const me, QEvt const * const e);
/*$enddecl${AOs::Blinky} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/* instantiate the Blinky active object ------------------------------------*/
static CANopenNodeSTM32_ao l_CANopenNodeSTM32;
QActive * const AO_CANopenNodeSTM32 = &l_CANopenNodeSTM32.super;

/* ask QM to define the Blinky class ---------------------------------------*/
/*$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/* Check for the minimum required QP version */
#if (QP_VERSION < 700U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpc version 7.0.0 or higher required
#endif
/*$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/*$define${AOs::Blinky_ctor} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/

/*${AOs::Blinky_ctor} ......................................................*/
void CANopenNodeSTM32_ctor(void) {
    CANopenNodeSTM32_ao *me = (CANopenNodeSTM32_ao *)AO_CANopenNodeSTM32;
    QActive_ctor(&me->super, Q_STATE_CAST(&CANopenNodeSTM32_initial));
    QTimeEvt_ctorX(&me->timeEvt, &me->super, AO_CANOPENNODESTM32_TIMEOUT_SIG, 0U);
}
/*$enddef${AOs::Blinky_ctor} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*$define${AOs::Blinky} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/

/*${AOs::Blinky} ...........................................................*/

/*${AOs::Blinky::SM} .......................................................*/
static QState CANopenNodeSTM32_initial(CANopenNodeSTM32_ao * const me, void const * const par) {
    /*${AOs::Blinky::SM::initial} */
    (void)par; /* unused parameter */
    me->canOpenNodeSTM32.CANHandle = &hcan;
    me->canOpenNodeSTM32.HWInitFunction = MX_CAN_Init;
    me->canOpenNodeSTM32.timerHandle = &htim14;
    me->canOpenNodeSTM32.desiredNodeID = 24;
    me->canOpenNodeSTM32.baudrate = 125;
    canopen_app_init(&me->canOpenNodeSTM32);
    /* arm the private time event to expire in 1/2s
    * and periodically every 1/2 second
    */
    QTimeEvt_armX(&me->timeEvt,
        BSP_TICKS_PER_SEC/2,
        1);
    return Q_TRAN(&CANopenNodeSTM32_operational);
}

/*${AOs::Blinky::SM::off} ..................................................*/
static QState CANopenNodeSTM32_operational(CANopenNodeSTM32_ao * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*${AOs::Blinky::SM::off} */
        case Q_ENTRY_SIG: {
            status_ = Q_HANDLED();
            break;
        }
        case AO_CANOPENNODESTM32_TIMEOUT_SIG: {
            HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, !me->canOpenNodeSTM32.outStatusLEDRed);
            HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, !me->canOpenNodeSTM32.outStatusLEDGreen);

            canopen_app_process();
            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status_;
}

void Timer14UpdateCallback(void)
{
    canopen_app_interrupt();
}

