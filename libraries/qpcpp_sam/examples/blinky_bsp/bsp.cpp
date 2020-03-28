//.$file${.::bsp.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: blinky_bsp.qm
// File:  ${.::bsp.cpp}
//
// This code has been generated by QM 5.0.0 <www.state-machine.com/qm/>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
// for more details.
//
//.$endhead${.::bsp.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#include "qpcpp.hpp"   // QP-C++ framework
#include "blinky.hpp"  // Blinky application interface
#include "bsp.hpp"     // Board Support Package (BSP)

using namespace QP;

//----------------------------------------------------------------------------
// BSP functions

//............................................................................
void BSP::init(void) {
    // initialize the hardware used in this sketch...
    // NOTE: interrupts are configured and started later in QF::onStartup()
    pinMode(LED_BUILTIN, OUTPUT);
}
//............................................................................
void BSP::ledOff(void) {
    digitalWrite(LED_BUILTIN, LOW);
}
//............................................................................
void BSP::ledOn(void) {
    digitalWrite(LED_BUILTIN, HIGH);
}

//----------------------------------------------------------------------------
// QF callbacks...

// ATSAM3X timer used for the system clock tick
//
// NOTE: The usual source of system clock tick in ARM Cortex-M (SysTick timer)
// is aready used by the Arduino library. Therefore, this code uses a different
// hardware timer of the ATSAM MCU for providing the system clock tick.
//
// NOTE: You can re-define the macros to use a different ATSAM timer/channel.
//
#define TIMER           TC1
#define TIMER_CLCK_HZ   650000
#define TIMER_CHANNEL   0
#define TIMER_IRQn      TC3_IRQn
#define TIMER_HANDLER   TC3_Handler

// interrupts.................................................................
void TIMER_HANDLER(void) {
    TC_GetStatus(TIMER, TIMER_CHANNEL);  // clear the interrupt source
    QF::TICK_X(0, (void *)0); // process time events for tick rate 0
}
//............................................................................
void QF::onStartup(void) {
    // configure the timer-counter channel........
    pmc_set_writeprotect(false);   // disable write protection
    pmc_enable_periph_clk(TIMER_IRQn); // enable peripheral clock
    TC_Configure(TIMER, TIMER_CHANNEL,
                 TC_CMR_WAVE           // WAVE mode
                 | TC_CMR_WAVSEL_UP_RC // count-up with trigger on RC compare
                 | TC_CMR_TCCLKS_TIMER_CLOCK4);  // internal Clock4
    TC_SetRC(TIMER, TIMER_CHANNEL,
             TIMER_CLCK_HZ / BSP::TICKS_PER_SEC); // set the RC compare value
    TC_Start(TIMER, TIMER_CHANNEL);
    // enable interrrupt for RC compare
    TIMER->TC_CHANNEL[TIMER_CHANNEL].TC_IER = TC_IER_CPCS;
    TIMER->TC_CHANNEL[TIMER_CHANNEL].TC_IDR = ~TC_IER_CPCS;
    pmc_set_writeprotect(true); // enable write protection

    // explicitly set the NVIC priorities for all "kernel AWARE" interrupts
    // NOTE: This is important!
    NVIC_SetPriority(TIMER_IRQn,  QF_AWARE_ISR_CMSIS_PRI);
    // ...

    // enable the interrupts in the NVIC
    NVIC_EnableIRQ(TIMER_IRQn);
    // ...
}
//............................................................................
void QV::onIdle(void) { // called with interrupts DISABLED
#ifdef NDEBUG
    // Put the CPU and peripherals to the low-power mode. You might
    // need to customize the clock management for your application,
    // see the datasheet for your particular MCU.
    QV_CPU_SLEEP();  // atomically go to sleep and enable interrupts
#else
    QF_INT_ENABLE(); // simply re-enable interrupts
#endif
}
//............................................................................
extern "C" Q_NORETURN Q_onAssert(char const * const module, int location) {
    //
    // NOTE: add here your application-specific error handling
    //
    (void)module;
    (void)location;

    QF_INT_DISABLE(); // disable all interrupts
    BSP::ledOn();  // trun the LED on
    for (;;) { // sit in an endless loop for now
    }
}
