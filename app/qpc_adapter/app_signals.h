#ifndef APP_SIGNALS_H
#define APP_SIGNALS_H
enum appSignals {
    DUMMY_SIG = Q_USER_SIG,
    MAX_PUB_SIG,          /* the last published signal */

    AO_BLINKY_TIMEOUT_SIG,
    AO_CANOPENNODESTM32_TIMEOUT_SIG,
    MAX_SIG               /* the last signal */
};

#endif /*APP_SIGNALS_H*/
