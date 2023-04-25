/*****************************************************************************
* Product: "Blinky" example
* Last updated for version 7.1.1
* Last updated on  2022-09-22
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

#include "ao_blinky.h"
#include "ao_canopennodestm32.h"

Q_DEFINE_THIS_FILE

/*..........................................................................*/
static int qf_main(void) {
    QF_init();  /* initialize the framework and the underlying RT kernel */

    BSP_init(); /* initialize the Board Support Package */

    /* publish-subscribe not used, no call to QF_psInit() */
    /* dynamic event allocation not used, no call to QF_poolInit() */

    /* instantiate and start the active objects... */
    Blinky_ctor();
    CANopenNodeSTM32_ctor();
    static QEvt const *l_blinkyQSto[10]; /* Event queue storage for Blinky */
    static QEvt const *l_canopennodeSTM32QSto[64];
    QACTIVE_START(AO_Blinky,      /* AO pointer to start */
                  2U,             /* QF-priority/preemption-threshold */
                  l_blinkyQSto,   /* storage for the AO's queue */
                  Q_DIM(l_blinkyQSto), /* length of the queue [entries] */
                  (void *)0,      /* stack storage (not used in QK) */
                  0U,             /* stack size [bytes] (not used in QK) */
                  (QEvt *)0);     /* initial event (or 0) */
    QACTIVE_START(AO_CANopenNodeSTM32,
                  3U,
                  l_canopennodeSTM32QSto,
                  Q_DIM(l_canopennodeSTM32QSto),
                  (void *)0,
                  0U,
                  (QEvt *)0);
    return QF_run(); /* run the QF application */
}

void app_main (void){
    (void)qf_main();
}
