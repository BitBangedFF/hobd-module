//******************************************************************************
//! @file $RCSfile: rtc_drv.c,v $
//!
//! Copyright (c) 2007 Atmel.
//!
//! Use of this program is subject to Atmel's End User License Agreement.
//! Please read file license.txt for copyright notice.
//!
//! @brief This file contains the prototypes and the macros of the
//!        low level functions (drivers) of:
//!             - RTC (Real Time Counter)
//!             - for DVK90CAN1 board
//! These routines need some #define's of "dvk90can1_board.h" file.
//!
//! This file can be parsed by Doxygen for automatic documentation generation.
//! This file has been validated with AVRStudio-413528/WinAVR-20070122.
//!
//! @version $Revision: 3.20 $ $Name: jtellier $
//!
//! @todo
//! @bug
//******************************************************************************

//_____  I N C L U D E S _______________________________________________________

#include "board.h"
#include "timer8_drv.h"
#include "rtc_drv.h"

//_____ M A C R O S ____________________________________________________________

//_____ D E F I N I T I O N S __________________________________________________

//_____ D E C L A R A T I O N S ________________________________________________

volatile uint32_t rtc_tics;
volatile uint32_t rtc_milliseconds;

BOOL rtc_running = OFF;

//------------------------------------------------------------------------------
//  @fn delay_ms
//!
//! Wait_for (delay or loop) milli-seconds using "tics" from an RTC_TIMER.
//!
//! @warning - FOSC must be define in "config.h".
//!          - RTC_TIMER must run and rtc_tics must count milli-seconds.
//!          - Do not wait_for 0 ms !
//!          - Problem may occur if: rtc_tics + ms_count > 0xFFFFFFFF !
//!
//! @param  uint16_t - wait value in ms (max: 65535 ms)
//!
//! @return  none
//!
//------------------------------------------------------------------------------
void delay_ms(uint16_t ms_count)
{
uint32_t temp;
uint16_t i;
uint8_t  j, k;

    if (rtc_running == ON)
    {
        disable_interrupt(); temp = rtc_tics; enable_interrupt();    //-- Get atomic uint32_t value
        temp += ((uint32_t)(ms_count));
        while (1)
        {
            disable_interrupt();    //-- Get atomic uint32_t values
            if (rtc_tics == temp) break;                //-- Standart EXIT
            if (rtc_tics == ((uint32_t)(ms_count)) ) break;  //-- EXIT if overflow (not right)
            enable_interrupt();
        }
        enable_interrupt();
    }
    else    //-- No RTC, enabling something almost equivalent but not right.
    {
        for (i=0;i<ms_count;i++)
        {
            for (j=0;j<(uint8_t)(FOSC/1000);j++)
            {
                for (k=0; k<90;k++);
            }
        }
    }
}

//------------------------------------------------------------------------------
//  @fn rtc_int_init
//!
//! Timer2 initialization to have 1 ms tic interval managed under interrupt.
//!
//! @warning  RTC_TIMER & RTC_CLOCK must be define in "dvk90can1_board.h" and
//!           FOSC in"config.h".
//!
//! @param  none
//!
//! @return  none
//!
//------------------------------------------------------------------------------
#if (RTC_TIMER == 2)

void rtc_int_init(void)
{
uint16_t i;

    disable_interrupt();

    Timer8_clear();                 //-- Timer 2 cleared & initialized "OFF"
    for (i=0;i<0xFFFF;i++);         //-- Waiting to let the Xtal stabilize after a power-on
    Timer8_overflow_it_disable();   //-- Disable OCIE2A interrupt
    Timer8_compare_a_it_disable();  //-- Disable TOIE2 interrupt
    //-- Config: - CTC mode (mode 2, top=OCR2A)
    //--         - No output
    //--        (- Timer "OFF")
    Timer8_set_mode_output_a(TIMER8_COMP_MODE_NORMAL);
    Timer8_set_waveform_mode(TIMER8_WGM_CTC_OCR);

#   if (RTC_CLOCK == 32)

    //--- Asynchronous external clock 32,768 KHZ
        Timer8_2_external_osc();            //-- Init RTC clock
        Timer8_set_compare_a(33-1);         //-- MAGIC_NUMBER !
        //-- No prescaler & timer "ON"
        //-- Tic interval: ((1/32768)*MAGIC_NUMBER) sec = 1.00708008 msec
        Timer8_set_clock(TIMER8_CLKIO_BY_1);

#   elif (RTC_CLOCK == 0) //-- Suppose synchronous clock = system clock

        Timer8_2_system_clk();              //-- Init System clock as clock for Timer2


#       if (FOSC == 16000)
            //--- Synchronous internal clock 16000 KHZ
            Timer8_set_compare_a(125-1);    //-- MAGIC_NUMBER !
            //-- Prescaler=128 & timer "ON"
            //-- Tic interval: ((1/16000000)*128*MAGIC_NUMBER) sec = 1.00000000 msec
            Timer8_set_clock(TIMER8_2_CLKIO_BY_128);

#       elif (FOSC == 12000)
            //--- Synchronous internal clock 12000 KHZ
            Timer8_set_compare_a(94-1);     //-- MAGIC_NUMBER !
            //-- Prescaler=128 & timer "ON"
            //-- Tic interval: ((1/12000000)*128*MAGIC_NUMBER) sec = 1.00266667 msec
            Timer8_set_clock(TIMER8_2_CLKIO_BY_128);

#       elif (FOSC == 8000)
            //--- Synchronous internal clock 8000 KHZ
            Timer8_set_compare_a(125-1);    //-- MAGIC_NUMBER !
            //-- Prescaler=64 & timer "ON"
            //-- Tic interval: ((1/8000000)*64*MAGIC_NUMBER) sec = 1.00000000 msec
            Timer8_set_clock(TIMER8_2_CLKIO_BY_64);

#       elif (FOSC == 6000)
            //--- Synchronous internal clock 6000 KHZ
            Timer8_set_compare_a(94-1);     //-- MAGIC_NUMBER !
            //-- Prescaler=64 & timer "ON"
            //-- Tic interval: ((1/6000000)*64*MAGIC_NUMBER) sec = 1.00000000 msec
            Timer8_set_clock(TIMER8_2_CLKIO_BY_64);

#       elif (FOSC == 4000)
            //--- Synchronous internal clock 4000 KHZ
            Timer8_set_compare_a(125-1);    //-- MAGIC_NUMBER !
            //-- Prescaler=64 & timer "ON"
            //-- Tic interval: ((1/4000000)*32*MAGIC_NUMBER) sec = 1.00000000 msec
            Timer8_set_clock(TIMER8_2_CLKIO_BY_32);
#       else
#       error This FOSC value is not available input for "rtc_drv.c" file
#       endif   // (FOSC ...

#   else
#       error This RTC_CLOCK value is not available input for "rtc_drv.c" file

#   endif // (RTC_CLOCK ...

    while(Timer8_2_update_busy());    //-- Wait for TCN2UB, OCR2UB and TCR2UB to be cleared

    Timer8_clear_compare_a_it();      //-- Clear Output_Compare Interrupt-flags
    Timer8_compare_a_it_enable();     //-- Enable Timer2 Output_Compare Interrupt

    //-- Time setting
    rtc_tics         = 0;
    rtc_milliseconds = 0;

    rtc_running = ON;
    enable_interrupt();
}
#endif // (RTC_TIMER ...

//------------------------------------------------------------------------------
//  @fn ISR(TIMER2_COMP_vect)
//!
//! Timer2 Output_Compare INTerrupt routine. Increment tics & the real-time
//! clock, the interrupt occurs once a milli second (or close).
//!
//! @warning RTC_TIMER & RTC_CLOCK must be define in "dvk90can1_board.h".
//!
//! @param  none
//!
//! @return  none
//!
//------------------------------------------------------------------------------
#if (RTC_TIMER == 2)

ISR(TIMER2_COMP_vect)
{
    rtc_tics++;                     //-- Increments tics
    rtc_milliseconds++;             //-- Increments milli seconds
}

#endif // RTC_TIMER



uint32_t rtc_get_ms( void )
{
    uint32_t timestamp = 0;

    // atomic get
    disable_interrupt();
    timestamp = rtc_milliseconds;
    enable_interrupt();

    return timestamp;
}