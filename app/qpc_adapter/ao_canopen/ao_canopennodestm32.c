#include "qpc.h"    /* QP/C framework API */
#include "bsp.h"    /* Board Support Package interface */

#include "ao_canopennodestm32.h"
#include "app_signals.h"
#include "CO_app_STM32.h"
#include "can.h"
#include "tim.h"
#include "gpio.h"
#include "OD.h"
/* ask QM to declare the Blinky class --------------------------------------*/
/*$declare${AOs::Blinky} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
extern TIM_HandleTypeDef htim14;
extern CAN_HandleTypeDef hcan;
extern OD_t *OD;
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
static void myFunc(OD_t *od, CANopenNodeSTM32 *pNode);
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
    me->canOpenNodeSTM32.baudrate = 250;
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

            canopen_app_process();
            myFunc(OD, &me->canOpenNodeSTM32);
            HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, !me->canOpenNodeSTM32.outStatusLEDRed);
            HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, !me->canOpenNodeSTM32.outStatusLEDGreen);
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

static void myFunc(OD_t *od, CANopenNodeSTM32 *pNode)
{
    extern CO_t* CO;
    ODR_t odRet;
    OD_entry_t *entry;
    OD_IO_t io1008;
    char buf[50];
    OD_size_t bytesRd;
    int error = 0;

    do{
        if(CO == NULL)
        {
            break;
        }
        /* Init IO for "Manufacturer device name" at index 0x1008, sub-index 0x00 */
        entry = OD_find(od, 0x1008);
        odRet = OD_getSub(entry, 0x00, &io1008, false);
        /* Read with io1008, subindex = 0x00 */
        if (odRet == ODR_OK) {
            /* Locking is necessary from mainline thread, but must not be used from
            * timer interval (real-time) thread. Locking is not necessary in the
            * CANoopen initialization section. Locking is also not necessary, if
            * OD variable is not mappable to PDO and not accessed from RT thread.*/
            CO_LOCK_OD(CO->CANmodule);
            odRet = io1008.read(&io1008.stream, &buf[0], sizeof(buf), &bytesRd);
            CO_UNLOCK_OD(CO->CANmodule);
        }
        if (odRet != ODR_OK) error++;

        /* Use helper and set "Producer heartbeat time" at index 0x1017, sub 0x00 */
        CO_LOCK_OD(CO->CANmodule); /* may not be necessary, see comment above */
        odRet = OD_set_u16(OD_find(od, 0x1017), 0x00, 500, false);
        CO_UNLOCK_OD(CO->CANmodule);
        if (odRet != ODR_OK) error++;
    }while(false);
}
