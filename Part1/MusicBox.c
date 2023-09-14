// MusicBox.c
// This is the startup file for CECS447 Project 1 Part 1.
// 
// Authors: Min He
// Date: August 28, 2022

#include "tm4c123gh6pm.h"
#include "music.h"
#include "switch.h"
#include "SysTick.h"

/**** Global Functions *****/
extern void EnableInterrupts(void);
extern void WaitForInterrupt(void);
extern void DisableInterrupts(void);

// MAIN: Mandatory for a C Program to be executable
// Do not change this main finction.
int main(void){	
  DisableInterrupts();
  Music_Init();
	SysTick_Init();
  Switch_Init();
  EnableInterrupts();  // SysTick uses interrupts
  
  while(1){
    if ( is_music_on() ) 
    {
		  play_a_song();
    }
    else 
    {
      WaitForInterrupt();
    }
  }
}

