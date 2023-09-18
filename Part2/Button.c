// ButtonLed.c: starter file for CECS447 Project 1 Part 1
// Runs on TM4C123, 
// Dr. Min He
// September 10, 2022
// Port B bits 5-0 outputs to the 6-bit DAC
// Port D bits 3-0 inputs from piano keys: CDEF:doe ray mi fa,negative logic connections.
// Port F is onboard LaunchPad switches and LED
// SysTick ISR: PF3 is used to implement heartbeat

#include "tm4c123gh6pm.h"
#include <stdint.h>
#include "Button.h"

// Constants
#define SW1 0x10  // bit position for onboard switch 1(left switch)
#define SW2 0x01  // bit position for onboard switch 2(right switch)
#define NVIC_EN0_PORTF 0x40000000  // enable PORTF edge interrupt
#define NVIC_EN0_PORTD 0x00000008  // enable PORTD edge interrupt
#define PORTF0_UNLOCK 0x4C4F434B

// Golbals
volatile uint8_t curr_mode=PIANO;  // 0: piano mode, 1: auto-play mode

//---------------------Switch_Init---------------------
// initialize onboard switch and LED interface
// Input: none
// Output: none 
void Button_Init(void){ 
    SYSCTL_RCGC2_R     |= 0x00000020;  // (a) activate clock for port F
    GPIO_PORTF_LOCK_R   =  PORTF0_UNLOCK;
    GPIO_PORTF_CR_R    |= 0x11;
    GPIO_PORTF_DIR_R   &= ~0x11;       // (c) make PF4 and PF0 in (built-in button)
    GPIO_PORTF_AFSEL_R &= ~0x11;       //     disable alt funct on PF4 and PF0
    GPIO_PORTF_DEN_R   |=  0x11;       //     enable digital I/O on PF4 and PF0  
    GPIO_PORTF_PCTL_R  &= ~0x000F000F; // configure PF4 and PF0 as GPIO
    GPIO_PORTF_AMSEL_R  = 0;           //     disable analog functionality on PF
    GPIO_PORTF_PUR_R   |= 0x11;        //     enable weak pull-up on PF4 and PF0
    GPIO_PORTF_IS_R    &= ~0x11;       // (d) PF4 and PF0 is edge-sensitive
    GPIO_PORTF_IBE_R   &= ~0x11;       //     PF4 and PF0 is not both edges
    GPIO_PORTF_IEV_R   &= ~0x11;       //     PF4 and PF0 falling edge event
    GPIO_PORTF_ICR_R   |= 0x11;        // (e) clear flag4 and flag0
    GPIO_PORTF_IM_R    |= 0x11;        // (f) arm interrupt on PF4 and PF0
    NVIC_PRI7_R = (NVIC_PRI7_R&0xFF1FFFFF)|0x00A00000; // (g) priority 5
    NVIC_EN0_R |= 0x40000000;          // (h) enable interrupt 30 in NVIC
}

//---------------------PianoKeys_Init---------------------
// initialize onboard Piano keys interface: PORT D 0 - 3 are used to generate 
// tones: CDEF:doe ray mi fa
// No need to unlock. Only PD7 needs to be unlocked.
// Input: none
// Output: none 
void PianoKeys_Init(void){ 
	SYSCTL_RCGC2_R     |= 0x00000008;  // (a) activate clock for port D
    GPIO_PORTF_DIR_R   &= ~0x0F;       // (c) make PD0 - PD3 inputs
    GPIO_PORTF_AFSEL_R &= ~0x0F;       //     disable alt funct on PD0 - PD3
    GPIO_PORTF_DEN_R   |=  0x0F;       //     enable digital I/O on PD0 - PD3 
    GPIO_PORTF_PCTL_R  &= ~0x0000FFFF; // configure PF4 and PF0 as GPIO
    GPIO_PORTF_AMSEL_R  = 0;           //     disable analog functionality on PD
    GPIO_PORTF_PUR_R   |= 0x0F;        //     enable weak pull-up on PD0 - PD3
    GPIO_PORTF_IS_R    &= ~0x0F;       // (d) PD0 - PD3 are edge-sensitive
    GPIO_PORTF_IBE_R   |= 0x0F;       //     PD0 - PD3 are both edges
    GPIO_PORTF_IEV_R   &= ~0x0F;       //     PD0 - PD3 falling edge event
    GPIO_PORTF_ICR_R   |= 0x0F;        // (e) clear flag 0 - 3
    GPIO_PORTF_IM_R    |= 0x0F;        // (f) arm interrupt on PD0 - PD3
    NVIC_PRI7_R = (NVIC_PRI7_R&0xFF1FFFFF)|0x00A00000; // (g) priority 5
    NVIC_EN0_R |= 0x40000000;          // (h) enable interrupt 30 in NVIC
}


uint8_t get_current_mode(void)
{
	return curr_mode;
}
