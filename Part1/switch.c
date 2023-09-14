// switch.c
// Runs on TM4C123
// Provide functions that initialize the onboard push buttons
// Author: Min He
// August 26, 2022

#include <stdbool.h>

#include "tm4c123gh6pm.h"
#include "switch.h"
#include "music.h"

// Enum for switch config and handling
typedef enum 
{
    SWITCH1_MASK  = 0x10,
    SWITCH2_MASK  = 0x01, 
    PORTF0_UNLOCK = 0x4C4F434B,
} PORTF_CONFIGS;

// Global musicOn variable
extern bool musicOn;

// Initialize Switches
void Switch_Init(void)
{ 
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

// ISR for PORTF
void GPIOPortF_Handler(void)
{
    // Switch 1 Pressed, controls if music is on or off
    if (GPIO_PORTF_RIS_R & SWITCH1_MASK)
    {
        if ( !musicOn )
        {
            turn_on_music();
        }
        else
        {
            turn_off_music();
        }
        GPIO_PORTF_ICR_R |= SWITCH1_MASK;      
    }
    // Switch 2 Pressed, select next song
    else if (GPIO_PORTF_RIS_R & SWITCH2_MASK) 
    { 
        next_song();
	    GPIO_PORTF_ICR_R |= SWITCH2_MASK;      
    }
}

